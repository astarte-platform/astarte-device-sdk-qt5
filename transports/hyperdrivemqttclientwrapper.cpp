/*
 * This file is part of Astarte.
 *
 * Copyright 2017 Ispirata Srl
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hyperdrivemqttclientwrapper.h"
#include "hyperdrivemqttclientwrapper_p.h"

#include <hyperdriveutils.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>
#include <QtCore/QMetaMethod>

#include <QtNetwork/QSslCertificate>

#include <HemeraCore/CommonOperations>
#include <HemeraCore/Fingerprints>
#include <HemeraCore/Literals>

// We use the as variant
#include <mosquittopp.h>

#define CONNACK_TIMEOUT (2 * 60 * 1000)

#define MIN_RECONNECTION_DELAY 3
#define MAX_RECONNECTION_DELAY 30

Q_LOGGING_CATEGORY(mqttWrapperDC, "hyperdrive.mqttclientwrapper", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperdrive {

void MQTTClientWrapperPrivate::setStatus(MQTTClientWrapper::Status s)
{
    if (status != s) {
        status = s;
        Q_Q(MQTTClientWrapper);
        Q_EMIT q->statusChanged(status);
    }
}

void MQTTClientWrapperPrivate::handleConnack(int rc)
{
    qCInfo(mqttWrapperDC) << "Connected to broker returned!";

    Q_Q(MQTTClientWrapper);

    // We received a CONNACK, stop the timer
    Q_EMIT q->connackReceived();

    if (rc == MOSQ_ERR_SUCCESS) {
        qCInfo(mqttWrapperDC) << "Connected to broker, session present: " << sessionPresent;
        setStatus(MQTTClientWrapper::ConnectedStatus);
    } else {
        qCInfo(mqttWrapperDC) << "Could not connected to broker!" << rc;
    }
}

void MQTTClientWrapperPrivate::on_connect(int rc)
{
    if (rc == MOSQ_ERR_SUCCESS) {
        sessionPresent = false;
    }
    this->handleConnack(rc);
}

void MQTTClientWrapperPrivate::on_connect_with_flags(int rc, int flags)
{
    if (rc == MOSQ_ERR_SUCCESS) {
        sessionPresent = isSessionPresent(flags);
    }
    this->handleConnack(rc);
}

void MQTTClientWrapperPrivate::on_disconnect(int rc)
{
    setStatus(MQTTClientWrapper::DisconnectedStatus);

    if (rc == 0) {
        // Client requested disconnect.
        mosquitto->loop_stop();
    } else {
        // Unexpected disconnect, Mosquitto will reconnect
        qCInfo(mqttWrapperDC) << "Unexpected disconnection from broker!" << rc;
        setStatus(MQTTClientWrapper::ReconnectingStatus);

        Q_Q(MQTTClientWrapper);
        Q_EMIT q->connectionStarted();
    }
}

void MQTTClientWrapperPrivate::on_message(const struct mosquitto_message *message)
{
    Q_Q(MQTTClientWrapper);

    // We need to copy the bytes. It's impossible to control the ownership otherwise.
    // Do not use ::fromRawData until you find out how to do 0-copy without leaking!!
    QByteArray payload((char*)message->payload, message->payloadlen);
    QByteArray topic(message->topic);

    Q_EMIT q->messageReceived(topic, payload);

    // mosquitto does free on message data for us
}

void MQTTClientWrapperPrivate::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    Q_UNUSED(mid);
    Q_UNUSED(qos_count);
    Q_UNUSED(granted_qos);
}

void MQTTClientWrapperPrivate::on_unsubscribe(int mid)
{
    Q_UNUSED(mid);
}

void MQTTClientWrapperPrivate::on_log(int level, const char *str)
{
    switch (level) {
    case MOSQ_LOG_WARNING:
    case MOSQ_LOG_ERR:
        qWarning() << "MOSQUITTO LOG!" << str;
        break;
    default:
        qDebug() << "MOSQUITTO: " << str;
        break;
    }
}

void MQTTClientWrapperPrivate::on_error()
{
    qWarning() << "MOSQUITTO ERROR!";
}

void MQTTClientWrapperPrivate::on_publish(int mid)
{
    Q_Q(MQTTClientWrapper);

    Q_EMIT q->publishConfirmed(mid);
}

bool MQTTClientWrapperPrivate::isSessionPresent(int flags)
{
    return (flags & 0x1) != 0;
}

MQTTClientWrapper::MQTTClientWrapper(const QUrl &host, QObject *parent)
    : MQTTClientWrapper(host, QByteArray(), parent)
{
}

MQTTClientWrapper::MQTTClientWrapper(const QUrl &host, const QByteArray &clientId, QObject *parent)
    : Hemera::AsyncInitObject(*new MQTTClientWrapperPrivate(this), parent)
{
    Q_D(MQTTClientWrapper);
    d->hardwareId = clientId;
    d->serverUrl = host;
    d->connackTimer = new QTimer(this);
    d->connackTimer->setInterval(Hyperdrive::Utils::randomizedInterval(CONNACK_TIMEOUT, 1.0));
    d->connackTimer->setSingleShot(true);
}

MQTTClientWrapper::~MQTTClientWrapper()
{
    Q_D(MQTTClientWrapper);

    if (Q_LIKELY(d->mosquitto)) {
        qWarning() << "Stopping mosquitto!";
        d->mosquitto->disconnect();
        d->mosquitto->loop_stop();

        delete d->mosquitto;
        mosqpp::lib_cleanup();
    }
}

MQTTClientWrapper::Status MQTTClientWrapper::status() const
{
    Q_D(const MQTTClientWrapper);
    return d->status;
}

QDateTime MQTTClientWrapper::clientCertificateExpiry() const
{
    Q_D(const MQTTClientWrapper);
    return d->clientCertificateExpiry;
}

void MQTTClientWrapper::setMutualSSLAuthentication(const QString& pathToCA, const QString& pathToPKey, const QString& pathToCertificate)
{
    Q_D(MQTTClientWrapper);

    d->pathToCA = pathToCA;
    d->pathToPKey = pathToPKey;
    d->pathToCertificate = pathToCertificate;

    QList<QSslCertificate> certificates = QSslCertificate::fromPath(pathToCertificate, QSsl::Pem);
    d->clientCertificateExpiry = certificates.value(0).expiryDate();

    // Instead of doing strdup, we let Qt's implicit sharing manage the stack for us
    QByteArray privateKey = d->pathToPKey.toLatin1();
    QByteArray trustStore = d->pathToCA.toLatin1();
    QByteArray keyStore = d->pathToCertificate.toLatin1();
}

void MQTTClientWrapper::setPublishQoS(MQTTClientWrapper::MQTTQoS qos)
{
    Q_D(MQTTClientWrapper);
    d->publishQoS = qos;
}

void MQTTClientWrapper::setSubscribeQoS(MQTTClientWrapper::MQTTQoS qos)
{
    Q_D(MQTTClientWrapper);
    d->subscribeQoS = qos;
}

void MQTTClientWrapper::setCleanSession(bool cleanSession)
{
    Q_D(MQTTClientWrapper);
    d->cleanSession = cleanSession;
}

void MQTTClientWrapper::setKeepAlive(quint64 seconds)
{
    Q_D(MQTTClientWrapper);
    d->keepAlive = seconds;
}

void MQTTClientWrapper::setIgnoreSslErrors(bool ignoreSslErrors)
{
    Q_D(MQTTClientWrapper);
    d->ignoreSslErrors = ignoreSslErrors;
}

void MQTTClientWrapper::setLastWill(const QByteArray& topic, const QByteArray& message, MQTTClientWrapper::MQTTQoS qos, bool retained)
{
    Q_D(MQTTClientWrapper);
    d->lwtMessage = message;
    d->lwtTopic = topic;
    d->lwtQos = qos;
    d->lwtRetained = retained;
}

QByteArray MQTTClientWrapper::hardwareId() const
{
    Q_D(const MQTTClientWrapper);
    return d->hardwareId;
}

QByteArray MQTTClientWrapper::rootClientTopic() const
{
    Q_D(const MQTTClientWrapper);
    return d->hardwareId;
}

bool MQTTClientWrapper::sessionPresent() const
{
    Q_D(const MQTTClientWrapper);
    return d->sessionPresent;
}

void MQTTClientWrapper::initImpl()
{
    Q_D(MQTTClientWrapper);

    Hyperdrive::Utils::seedRNG();

    auto initMosquitto = [this, d] {
        // Initialize stuff
        d->mosquitto = new HyperdriveMosquittoClient(d, d->hardwareId.constData(), d->cleanSession);

        // Set a randomized reconnect with exponential backoff
        int randomizedMinDelaySec = Hyperdrive::Utils::randomizedInterval(MIN_RECONNECTION_DELAY, 0.7);
        int randomizedMaxDelaySec = Hyperdrive::Utils::randomizedInterval(MAX_RECONNECTION_DELAY, 0.2);
        d->mosquitto->reconnect_delay_set(randomizedMinDelaySec, randomizedMaxDelaySec, true);

        // SSL
        if (!d->pathToCA.isEmpty() && !d->pathToPKey.isEmpty() && !d->pathToCertificate.isEmpty()) {
            // Instead of doing strdup, we let Qt's implicit sharing manage the stack for us
            QByteArray privateKey = d->pathToPKey.toLatin1();
            QByteArray trustStore = d->pathToCA.toLatin1();
            QByteArray keyStore = d->pathToCertificate.toLatin1();
            // Configure mutual SSL authentication.
            qDebug() << "Setting TLS!" << trustStore << keyStore << privateKey;
            d->mosquitto->tls_set(trustStore.constData(), NULL, keyStore.constData(), privateKey.constData());
            if (d->ignoreSslErrors) {
                d->mosquitto->tls_opts_set(0);
            } else {
                d->mosquitto->tls_opts_set(1);
            }
        }

        // Always successful
        mosqpp::lib_init();

        qCWarning(mqttWrapperDC) << "Mosquitto is up!";

        setReady();
    };

    // Check the connack timeout and propagate it
    connect(d->connackTimer, &QTimer::timeout, this, &MQTTClientWrapper::connackTimeout);
    // Connect the signals received from the callback thread
    connect(this, &MQTTClientWrapper::connectionStarted, d->connackTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(this, &MQTTClientWrapper::connackReceived, d->connackTimer, &QTimer::stop);

    if (!d->hardwareId.isEmpty()) {
        initMosquitto();
        return;
    }

    Hemera::ByteArrayOperation *op = Hemera::Fingerprints::globalHardwareId();
    connect(op, &Hemera::Operation::finished, this, [this, op, d, initMosquitto] {
        if (op->isError()) {
            setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()), tr("Could not retrieve global hardware ID!"));
            return;
        }

        d->hardwareId = op->result();

        initMosquitto();
    });
}

bool MQTTClientWrapper::connectToBroker()
{
    Q_D(MQTTClientWrapper);

    switch (d->status) {
        // We are already connecting
        case ConnectingStatus:
        case ConnectedStatus:
        case ReconnectingStatus: {
            return true;
        }

        case DisconnectedStatus:
        case DisconnectingStatus: {
            int rc;

            qCInfo(mqttWrapperDC) << "Starting mosquitto connection" << d->serverUrl.host() << d->serverUrl.port();

            if ((rc = d->mosquitto->connect_async(qstrdup(d->serverUrl.host().toLatin1().constData()), d->serverUrl.port(), d->keepAlive)) != MOSQ_ERR_SUCCESS) {
                qCWarning(mqttWrapperDC) << "Could not initiate async mosquitto connection! Return code " << rc;
                Q_EMIT connectionFailed();
                return false;
            }
            if (d->mosquitto->loop_start() != MOSQ_ERR_SUCCESS) {
                qCWarning(mqttWrapperDC) << "Could not initiate async mosquitto loop! Something is beyond broken!";
                return false;
            }

            d->setStatus(MQTTClientWrapper::ConnectingStatus);
            qCInfo(mqttWrapperDC) << "Started mosquitto connection";

            d->connackTimer->start();

            return true;
        }

        default: {
            qCWarning(mqttWrapperDC) << "Unknown status in connect, something is really wrong";
            return false;
        }
    }
}

bool MQTTClientWrapper::disconnectFromBroker()
{
    Q_D(MQTTClientWrapper);

    switch (d->status) {
        // We are already disconnecting
        case DisconnectingStatus:
        case DisconnectedStatus: {
            return true;
        }

        case ConnectingStatus:
        case ConnectedStatus:
        case ReconnectingStatus: {
            int rc;

            d->connackTimer->stop();

            if ((rc = d->mosquitto->disconnect()) != MOSQ_ERR_SUCCESS) {
                if (rc == MOSQ_ERR_NO_CONN) {
                    qCWarning(mqttWrapperDC) << "Trying to disconnect, but not connected to a broker";
                    d->setStatus(MQTTClientWrapper::DisconnectedStatus);
                    return true;
                } else {
                    qCWarning(mqttWrapperDC) << "Failed to start disconnect, return code" << rc;
                    return false;
                }
            }
            d->setStatus(MQTTClientWrapper::DisconnectingStatus);
            return true;
        }
        default: {
            qCWarning(mqttWrapperDC) << "Unknown status in disconnect, something is really wrong";
            return false;
        }
    }
}

int MQTTClientWrapper::publish(const QByteArray& topic, const QByteArray& payload, MQTTQoS lqos, bool retained)
{
    Q_D(MQTTClientWrapper);

    if (Q_UNLIKELY(!d->mosquitto)) {
        qCWarning(mqttWrapperDC) << "Attempted to call publish before initializing the client!";
        return -1;
    }

    int rc;
    int qos = lqos == MQTTQoS::DefaultQoS ? d->publishQoS : (int)lqos;
    int mid;

    if ((rc = d->mosquitto->publish(&mid, topic.constData(), payload.length(), const_cast<char*>(payload.data()), qos, retained)) != MOSQ_ERR_SUCCESS) {
        qCWarning(mqttWrapperDC) << "Failed to start sendMessage, return code " << rc;
        return -rc;
    }

    return mid;
}

void MQTTClientWrapper::subscribe(const QByteArray& topic, MQTTQoS subQoS)
{
    Q_D(MQTTClientWrapper);

    if (Q_UNLIKELY(!d->mosquitto)) {
        qCWarning(mqttWrapperDC) << "Attempted to call subscribe before initializing the client!";
        return;
    }

    int rc;
    int qos;
    if (subQoS == MQTTQoS::DefaultQoS) {
        qos = d->publishQoS;
    } else {
        qos = (int)subQoS;
    }

    if ((rc = d->mosquitto->subscribe(NULL, topic.constData(), qos)) != MOSQ_ERR_SUCCESS) {
        qCWarning(mqttWrapperDC) << "Failed to start subscribe, return code " << rc;
    }
}

}

#include "moc_hyperdrivemqttclientwrapper.cpp"
