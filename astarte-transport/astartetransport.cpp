/*
 * Copyright (C) 2017 Ispirata Srl
 *
 * This file is part of Astarte.
 * Astarte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Astarte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Astarte.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "astartetransport.h"

#include "astartetransportcache.h"

#include <HemeraCore/Literals>
#include <HemeraCore/Operation>

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtCore/QSocketNotifier>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>
#include <QtCore/QVariantMap>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>
#include <QtCore/QFile>
#include <QtConcurrent/QtConcurrentRun>

#include <cachemessage.h>
#include <hyperdriveconfig.h>
#include <hyperdriveinterface.h>
#include <hyperdriveutils.h>

#include <astartehttpendpoint.h>

#include <HyperspaceCore/Fluctuation>
#include <HyperspaceCore/Rebound>
#include <HyperspaceCore/Wave>

#include <HyperspaceProducerConsumer/ProducerAbstractInterface>

#define CONNECTION_RETRY_INTERVAL 15000
#define PAIRING_RETRY_INTERVAL (5 * 60 * 1000)

#define METHOD_WRITE "WRITE"
#define METHOD_ERROR "ERROR"

#define CERTIFICATE_RENEWAL_DAYS 8

Q_LOGGING_CATEGORY(astarteTransportDC, "hyperdrive.transport.astarte", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperdrive
{

AstarteTransport::AstarteTransport(const QString &configurationPath, QObject* parent)
    : AsyncInitObject(parent)
    , m_configurationPath(configurationPath)
    , m_rebootTimer(new QTimer(this))
    , m_rebootWhenConnectionFails(false)
    , m_rebootDelayMinutes(600)
{
    qRegisterMetaType<MQTTClientWrapper::Status>();
    connect(this, &AstarteTransport::introspectionChanged, this, [this] {
            publishIntrospection();
            setupClientSubscriptions();
    });
}

AstarteTransport::~AstarteTransport()
{
}

void AstarteTransport::initImpl()
{
    setParts(2);

    Hyperdrive::Utils::seedRNG();

    // When we are ready to go, we setup MQTT
    connect(this, &Hemera::AsyncInitObject::ready, this, &AstarteTransport::setupMqtt);

    // Load from configuration
    if (!QFile::exists(m_configurationPath)) {
        setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::notFound()),
                     QStringLiteral("Configuration file %1 does not exist").arg(m_configurationPath));
        return;
    }

    QSettings settings(m_configurationPath, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        if (settings.status() != QSettings::AccessError) {
            setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::notAllowed()),
                        QStringLiteral("Configuration file %1 cannot be read").arg(m_configurationPath));

        } else if (settings.status() != QSettings::FormatError) {
            setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::parseError()),
                        QStringLiteral("Configuration file %1 is invalid").arg(m_configurationPath));
        }
        return;
    }
    settings.beginGroup(QStringLiteral("AstarteTransport")); {
        m_persistencyDir = settings.value(QStringLiteral("persistencyDir"), QDir::currentPath()).toString();

        QSettings syncSettings(QStringLiteral("%1/transportStatus.conf").arg(m_persistencyDir), QSettings::IniFormat);
        m_synced = syncSettings.value(QStringLiteral("isSynced"), false).toBool();

        AstarteTransportCache::setPersistencyDir(m_persistencyDir);
        connect(AstarteTransportCache::instance()->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op) {
            if (op->isError()) {
                setInitError(op->errorName(), op->errorMessage());
            } else {
                setOnePartIsReady();
            }
        });

        m_astarteEndpoint = new Astarte::HTTPEndpoint(m_configurationPath, m_persistencyDir, settings.value(QStringLiteral("endpoint")).toUrl(), QSslConfiguration::defaultConfiguration(), this);

        m_rebootWhenConnectionFails = settings.value(QStringLiteral("rebootWhenConnectionFails"), false).toBool();
        m_rebootDelayMinutes = settings.value(QStringLiteral("rebootDelayMinutes"), 600).toInt();
        m_rebootTimer->setTimerType(Qt::VeryCoarseTimer);
        int randomizedRebootDelayms = Hyperdrive::Utils::randomizedInterval(m_rebootDelayMinutes * 60 * 1000, 0.1);
        m_rebootTimer->setInterval(randomizedRebootDelayms);

        if (m_rebootWhenConnectionFails) {
            qCDebug(astarteTransportDC) << "Activating the reboot timer with delay " << (randomizedRebootDelayms / (60 * 1000)) << " minutes";
            m_rebootTimer->start();
        }
        connect(m_rebootTimer, &QTimer::timeout, this, &AstarteTransport::handleRebootTimerTimeout);

        connect(m_astarteEndpoint->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op) {
            if (op->isError()) {
                //TODO: further handling?
                qDebug(astarteTransportDC) << "An error happened during Astarte Endpoint init: " << op->errorMessage();
                return;
            }

            if (!m_mqttBroker.isNull()) {
                m_mqttBroker->disconnectFromBroker();
                m_mqttBroker->deleteLater();
            }

            // Are we paired?
            if (!m_astarteEndpoint->isPaired()) {
                startPairing(false);
            } else {
                setOnePartIsReady();
            }
        });
    } settings.endGroup();
}

void AstarteTransport::startPairing(bool forcedPairing) {
    connect(m_astarteEndpoint->pair(forcedPairing), &Hemera::Operation::finished, this, [this, forcedPairing] (Hemera::Operation *pOp) {
        if (pOp->isError()) {
            int retryInterval = Hyperdrive::Utils::randomizedInterval(PAIRING_RETRY_INTERVAL, 1.0);
            qCWarning(astarteTransportDC) << "Pairing failed!!" << pOp->errorMessage() << ", retrying in " << (retryInterval / 1000) << " seconds";
            QTimer::singleShot(retryInterval, this, [this, forcedPairing] {
                startPairing(forcedPairing);
            });
            return;
        } else {
            if (forcedPairing) {
                setupMqtt();
            } else {
                setOnePartIsReady();
            }
        }
    });
}

void AstarteTransport::setupMqtt()
{
    // Good. Let's set up our MQTT broker.
    m_mqttBroker = m_astarteEndpoint->createMqttClientWrapper();

    if (m_mqttBroker.isNull()) {
        qCWarning(astarteTransportDC) << "Could not create the MQTT client!!";
        return;
    }

    if (m_mqttBroker->clientCertificateExpiry().isValid() &&
        QDateTime::currentDateTime().daysTo(m_mqttBroker->clientCertificateExpiry()) <= CERTIFICATE_RENEWAL_DAYS) {
        forceNewPairing();
        return;
    }

    connect(m_mqttBroker->init(), &Hemera::Operation::finished, this, [this] {
        m_mqttBroker->setKeepAlive(60);
        m_mqttBroker->connectToBroker();
    });
    connect(m_mqttBroker, &MQTTClientWrapper::statusChanged, this, &AstarteTransport::onStatusChanged);
    connect(m_mqttBroker, &MQTTClientWrapper::messageReceived, this, &AstarteTransport::onMQTTMessageReceived);
    connect(m_mqttBroker, &MQTTClientWrapper::publishConfirmed, this, &AstarteTransport::onPublishConfirmed);
    connect(m_mqttBroker, &MQTTClientWrapper::connackTimeout, this, &AstarteTransport::handleConnackTimeout);
    connect(m_mqttBroker, &MQTTClientWrapper::connectionFailed, this, &AstarteTransport::handleConnectionFailed);
}

void AstarteTransport::onMQTTMessageReceived(const QByteArray& topic, const QByteArray& payload)
{
    // Normalize the message
    if (!topic.startsWith(m_mqttBroker->rootClientTopic())) {
        qCWarning(astarteTransportDC) << "Received MQTT message on topic" << topic << ", which does not match the device base hierarchy!";
        return;
    }

    QByteArray relativeTopic = topic;
    relativeTopic.remove(0, m_mqttBroker->rootClientTopic().length());

    // Prepare a write wave
    Hyperspace::Wave w;
    w.setMethod(METHOD_WRITE);
    w.setTarget(relativeTopic);
    w.setPayload(payload);
    m_waveStorage.insert(w.id(), w);
    qCDebug(astarteTransportDC) << "Sending wave" << w.method() << w.target();
    routeWave(w, -1);
}

void AstarteTransport::setupClientSubscriptions()
{
    if (m_mqttBroker.isNull()) {
        qCWarning(astarteTransportDC) << "Can't publish subscriptions, broker is null";
        return;
    }
    // Setup subscriptions to control interface
    m_mqttBroker->subscribe(m_mqttBroker->rootClientTopic() + "/control/#", MQTTClientWrapper::ExactlyOnceQoS);
    // Setup subscriptions to interfaces where we can receive data
    for (QHash< QByteArray, Hyperdrive::Interface >::const_iterator i = introspection().constBegin(); i != introspection().constEnd(); ++i) {
        if (i.value().interfaceQuality() == Interface::Quality::Consumer) {
            // Subscribe to the interface itself
            m_mqttBroker->subscribe(m_mqttBroker->rootClientTopic() + "/" + i.value().interface(), MQTTClientWrapper::ExactlyOnceQoS);
            // Subscribe to the interface properties
            m_mqttBroker->subscribe(m_mqttBroker->rootClientTopic() + "/" + i.value().interface() + "/#", MQTTClientWrapper::ExactlyOnceQoS);
        }
    }
}

void AstarteTransport::sendProperties()
{
    for (QHash< QByteArray, QByteArray >::const_iterator i = AstarteTransportCache::instance()->allPersistentEntries().constBegin();
         i != AstarteTransportCache::instance()->allPersistentEntries().constEnd();
         ++i) {

        // Recreate the cacheMessage
        CacheMessage c;
        c.setTarget(i.key());
        c.setPayload(i.value());
        c.setInterfaceType(Hyperdrive::Interface::Type::Properties);

        if (m_mqttBroker.isNull()) {
            handleFailedPublish(c);
            continue;
        }

        int rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + i.key(), i.value(), MQTTClientWrapper::ExactlyOnceQoS);
        if (rc < 0) {
            // If it's < 0, it's an error
            handleFailedPublish(c);
        } else {
            // Otherwise, it's the messageId
            qCInfo(astarteTransportDC) << "Inserting in-flight message id " << rc;
            AstarteTransportCache::instance()->addInFlightEntry(rc, c);
        }
    }
}

void AstarteTransport::resendFailedMessages()
{
    QList<int> ids = AstarteTransportCache::instance()->allRetryIds();
    for (int id: ids) {
        CacheMessage failedMessage = AstarteTransportCache::instance()->takeRetryEntry(id);
        // Call cache message function with the failed message
        cacheMessage(failedMessage);
    }
}

void AstarteTransport::rebound(const Hyperspace::Rebound& r, int fd)
{
    Q_UNUSED(fd);

    Hyperspace::Rebound rebound = r;

    // FIXME: We should just trigger errors here.
    return;

    if (!m_waveStorage.contains(r.id())) {
        qCWarning(astarteTransportDC) << "Got a rebound with id" << r.id() << "which does not match any routing rules!";
        return;
    }

    Hyperspace::Wave w = m_waveStorage.take(r.id());
    QByteArray waveTarget = w.target();

    if (m_commandTree.contains(r.id())) {
        QByteArray commandId = m_commandTree.take(r.id());
        m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + "/response" + waveTarget,
                              commandId + "," + QByteArray::number(static_cast<quint16>(r.response())));
    }

    if (r.response() != Hyperspace::ResponseCode::OK) {
        // TODO: How do we handle failed requests?
        qCWarning(astarteTransportDC) << "Wave failed!" << static_cast<quint16>(r.response());
        return;
    }

    if (!r.payload().isEmpty()) {
        m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + waveTarget, r.payload());
    }
}

void AstarteTransport::fluctuation(const Hyperspace::Fluctuation &fluctuation)
{
    qCDebug(astarteTransportDC) << "Received fluctuation from: " << fluctuation.target() << fluctuation.payload();
}

void AstarteTransport::cacheMessage(const CacheMessage &cacheMessage)
{
    qCDebug(astarteTransportDC) << "Received cacheMessage from: " << cacheMessage.target() << cacheMessage.payload();
    if (m_mqttBroker.isNull()) {
        handleFailedPublish(cacheMessage);
        return;
    }

    int rc;

    switch (cacheMessage.interfaceType()) {
        case Hyperdrive::Interface::Type::Properties: {
            if (AstarteTransportCache::instance()->isCached(cacheMessage.target())
                    && AstarteTransportCache::instance()->persistentEntry(cacheMessage.target()) == cacheMessage.payload()) {

                qCDebug(astarteTransportDC) << cacheMessage.target() << "is not changed, not publishing it again";
                // We consider it delivered, so remove it from the DB
                AstarteTransportCache::instance()->removeFromDatabase(cacheMessage);
                return;
            }

            rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + cacheMessage.target(), cacheMessage.payload(), MQTTClientWrapper::ExactlyOnceQoS);
            break;
        }

        case Hyperdrive::Interface::Type::DataStream: {
            Hyperspace::Reliability reliability = static_cast<Hyperspace::Reliability>(cacheMessage.attributes().value("reliability").toInt());
            switch (reliability) {
                case (Hyperspace::Reliability::Guaranteed):
                    rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + cacheMessage.target(), cacheMessage.payload(), MQTTClientWrapper::AtLeastOnceQoS);
                    break;
                case (Hyperspace::Reliability::Unique):
                    rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + cacheMessage.target(), cacheMessage.payload(), MQTTClientWrapper::ExactlyOnceQoS);
                    break;
                default:
                    // Default Unreliable
                    rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + cacheMessage.target(), cacheMessage.payload(), MQTTClientWrapper::AtMostOnceQoS);
                    break;
            }
            break;
        }

        default: {
            qCDebug(astarteTransportDC) << "Unsupported interfaceType";
            break;
        }
    }

    if (rc < 0) {
        // If it's < 0, it's an error
        handleFailedPublish(cacheMessage);
    } else {
        // Otherwise, it's the messageId
        qCInfo(astarteTransportDC) << "Inserting in-flight message id " << rc;
        AstarteTransportCache::instance()->addInFlightEntry(rc, cacheMessage);
    }
}

void AstarteTransport::forceNewPairing()
{
    // Operation is error, certificate is invalid
    qCWarning(astarteTransportDC) << "Forcing new pairing";

    if (!m_mqttBroker.isNull()) {
        m_mqttBroker->disconnectFromBroker();
        m_mqttBroker->deleteLater();
    }
    // Reset the cache
    AstarteTransportCache::instance()->resetInFlightEntries();

    startPairing(true);
}

void AstarteTransport::onStatusChanged(MQTTClientWrapper::Status status)
{
    if (status == MQTTClientWrapper::ConnectedStatus) {
        // We're connected, stop the reboot timer
        qCDebug(astarteTransportDC) << "Connected, stopping the reboot timer";
        m_rebootTimer->stop();
        if (!m_mqttBroker->sessionPresent() || !m_synced) {
            // We're desynced
            bigBang();
        }

        // Resend the messages that failed to be published
        resendFailedMessages();
    } else {
        // If we are in every other state, we start the reboot timer (if needed)
        if (m_rebootWhenConnectionFails && !m_rebootTimer->isActive()) {
            qCDebug(astarteTransportDC) << "Not connected state, restarting the reboot timer";
            m_rebootTimer->start();
        }
    }
}

void AstarteTransport::handleConnectionFailed()
{
    int retryInterval = Hyperdrive::Utils::randomizedInterval(CONNECTION_RETRY_INTERVAL, 1.0);
    qCInfo(astarteTransportDC) << "Connection failed, trying to reconnect to the broker in " << (retryInterval / 1000) << " seconds";
    QTimer::singleShot(retryInterval, m_mqttBroker, &MQTTClientWrapper::connectToBroker);
}

void AstarteTransport::handleConnackTimeout()
{
    qCWarning(astarteTransportDC) << "CONNACK timeout, verifying certificate";

    connect(m_astarteEndpoint->verifyCertificate(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *vOp) {
        if (vOp->isError()) {
            qCWarning(astarteTransportDC) << vOp->errorMessage();
            forceNewPairing();
        }
    });
}

void AstarteTransport::handleRebootTimerTimeout()
{
    qCWarning(astarteTransportDC) << "Connection with Astarte was lost for too long!";
}

void AstarteTransport::handleFailedPublish(const CacheMessage &cacheMessage)
{
    qCWarning(astarteTransportDC) << "Can't publish for target" << cacheMessage.target();
    if (cacheMessage.attributes().value("retention").toInt() == static_cast<int>(Hyperspace::Retention::Discard)) {
        // Prepare an error wave
        Hyperspace::Wave w;
        w.setMethod(METHOD_ERROR);
        w.setTarget(cacheMessage.target());
        w.setPayload(cacheMessage.payload());
        qCDebug(astarteTransportDC) << "Sending error wave for Discard target " << w.target();
        routeWave(w, -1);
    } else {
        int id = AstarteTransportCache::instance()->addRetryEntry(cacheMessage);
        Q_UNUSED(id);
    }
}

void AstarteTransport::bigBang()
{
    qCWarning(astarteTransportDC) << "Received bigBang";

    QSettings syncSettings(QStringLiteral("%1/transportStatus.conf").arg(m_persistencyDir), QSettings::IniFormat);
    if (m_synced) {
        m_synced = false;
        syncSettings.setValue(QStringLiteral("isSynced"), false);
    }

    if (m_mqttBroker.isNull()) {
        qCDebug(astarteTransportDC) << "Can't send emptyCache request, broker is null";
        return;
    }

    // We need to setup again the subscriptions, unless we have a persistent session on the other end.
    setupClientSubscriptions();
    // And publish the introspection.
    publishIntrospection();

    int rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + "/control/emptyCache", "1", MQTTClientWrapper::ExactlyOnceQoS);
    if (rc < 0) {
        // We leave m_synced to false and we retry when we're back online
        qCWarning(astarteTransportDC) << "Can't send emptyCache request, error " << rc;
        return;
    }

    QByteArray payload;
    for (const QByteArray &path : AstarteTransportCache::instance()->allPersistentEntries().keys()) {
        // Remove leading slash
        payload.append(path.mid(1));
        payload.append(';');
    }
    // Remove trailing semicolon
    payload.chop(1);

    qCDebug(astarteTransportDC) << "Producer property paths are: " << payload;

    rc = m_mqttBroker->publish(m_mqttBroker->rootClientTopic() + "/control/producer/properties", qCompress(payload), MQTTClientWrapper::ExactlyOnceQoS);
    if (rc < 0) {
        // We leave m_synced to false and we retry when we're back online
        qCWarning(astarteTransportDC) << "Can't send producer properties list, error " << rc;
        return;
    }

    // Send the cached properties
    sendProperties();

    // If we are here, the messages are in the hands of mosquitto
    m_synced = true;
    syncSettings.setValue(QStringLiteral("isSynced"), true);
}

void AstarteTransport::onPublishConfirmed(int messageId)
{
    qCInfo(astarteTransportDC) << "Message with id" << messageId << ": publish confirmed";
    CacheMessage cacheMessage = AstarteTransportCache::instance()->takeInFlightEntry(messageId);

    if (cacheMessage.interfaceType() == Hyperdrive::Interface::Type::Properties) {
        if (cacheMessage.payload().isEmpty()) {
            AstarteTransportCache::instance()->removePersistentEntry(cacheMessage.target());
        } else {
            AstarteTransportCache::instance()->insertOrUpdatePersistentEntry(cacheMessage.target(), cacheMessage.payload());
        }
    }
}

QByteArray AstarteTransport::introspectionString() const
{
    QByteArray ret;
    QMap<QByteArray, Hyperdrive::Interface> sortedIntrospection;

    // Put the introspection in a temporary map to guarantee ordering
    for (QHash< QByteArray, Hyperdrive::Interface >::const_iterator i = introspection().constBegin(); i != introspection().constEnd(); ++i) {
        sortedIntrospection.insert(i.key(), i.value());
    }

    for (QMap< QByteArray, Hyperdrive::Interface >::const_iterator i = sortedIntrospection.constBegin(); i != sortedIntrospection.constEnd(); ++i) {
        ret.append(i.key());
        ret.append(':');
        ret.append(QByteArray::number(i.value().versionMajor()));
        ret.append(':');
        ret.append(QByteArray::number(i.value().versionMinor()));
        ret.append(';');
    }

    // Remove last ;
    ret.chop(1);

    return ret;
}

void AstarteTransport::publishIntrospection()
{
    if (m_mqttBroker.isNull()) {
        return;
    }

    qCInfo(astarteTransportDC) << "Publishing introspection!";

    // Create a string representation
    QByteArray payload;
    for (QHash< QByteArray, Hyperdrive::Interface >::const_iterator i = introspection().constBegin(); i != introspection().constEnd(); ++i) {
        payload.append(i.key());
        payload.append(':');
        payload.append(QByteArray::number(i.value().versionMajor()));
        payload.append(':');
        payload.append(QByteArray::number(i.value().versionMinor()));
        payload.append(';');
    }

    // Remove last ;
    payload.chop(1);

    qCDebug(astarteTransportDC) << "Introspection is " << payload;

    m_mqttBroker->publish(m_mqttBroker->rootClientTopic(), payload, Hyperdrive::MQTTClientWrapper::ExactlyOnceQoS);
}

QHash< QByteArray, Hyperdrive::Interface> AstarteTransport::introspection() const
{
    // TODO
    return m_introspection;
}

void AstarteTransport::setIntrospection(const QHash< QByteArray, Hyperdrive::Interface> &introspection)
{
    if (m_introspection != introspection) {
        m_introspection = introspection;
        emit introspectionChanged();
    }
}

void AstarteTransport::routeWave(const Hyperspace::Wave &wave, int fd)
{
    Q_UNUSED(fd)

    QByteArray waveTarget = wave.target();
    int targetIndex = waveTarget.indexOf('/', 1);
    QByteArray interface = waveTarget.mid(1, targetIndex - 1);
    QByteArray relativeTarget = waveTarget.right(waveTarget.length() - targetIndex);

    if (interface == "control") {
        qCDebug(astarteTransportDC) << "Received control wave, not implemented in SDK";
        return;
    }

    Hyperspace::Wave w;
    w.setTarget(relativeTarget);
    w.setPayload(wave.payload());

    Q_EMIT waveReceived(interface, w);
}

}
