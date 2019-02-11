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

#include "astartehttpendpoint.h"
#include "astartehttpendpoint_p.h"

#include "astartecrypto.h"
#include "astartepairoperation.h"
#include "astarteverifycertificateoperation.h"
#include "credentialssecretprovider.h"
#include "defaultcredentialssecretprovider.h"
#include "hyperdrivemqttclientwrapper.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <HemeraCore/CommonOperations>
#include <HemeraCore/Fingerprints>
#include <HemeraCore/Literals>

#include <hemeraasyncinitobject_p.h>

#include <hyperdriveconfig.h>
#include <hyperdriveutils.h>

#define RETRY_INTERVAL 15000

Q_LOGGING_CATEGORY(astarteHttpEndpointDC, "astarte.httpendpoint", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Astarte {

void HTTPEndpointPrivate::connectToEndpoint()
{
    QUrl infoEndpoint = endpoint;
    infoEndpoint.setPath(QStringLiteral("%1/devices/%2").arg(endpoint.path()).arg(QString::fromLatin1(hardwareId)));
    QNetworkRequest req(infoEndpoint);
    req.setSslConfiguration(sslConfiguration);
    req.setRawHeader("Authorization", "Bearer " + credentialsSecretProvider->credentialsSecret());
    req.setRawHeader("X-Astarte-Transport-Provider", "Astarte Device SDK Qt5");
    req.setRawHeader("X-Astarte-Transport-Version", QStringLiteral("%1.%2.%3")
                                                     .arg(Hyperdrive::StaticConfig::hyperdriveMajorVersion())
                                                     .arg(Hyperdrive::StaticConfig::hyperdriveMinorVersion())
                                                     .arg(Hyperdrive::StaticConfig::hyperdriveReleaseVersion())
                                                     .toLatin1());
    QNetworkReply *reply = nam->get(req);
    qCDebug(astarteHttpEndpointDC) << "Connecting to our endpoint! " << infoEndpoint;

    Q_Q(HTTPEndpoint);
    QObject::connect(reply, &QNetworkReply::finished, q, [this, q, reply] {
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(astarteHttpEndpointDC) << "Error: " << reply->error();
            retryConnectToEndpointLater();
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        reply->deleteLater();
        if (!doc.isObject()) {
            qCWarning(astarteHttpEndpointDC) << "Invalid JSON in connectToEndpoint: " << response.constData();
            retryConnectToEndpointLater();
            return;
        }

        qCDebug(astarteHttpEndpointDC) << "Connected! " << doc.toJson(QJsonDocument::Indented);

        QJsonObject rootReplyObj = doc.object().value(QStringLiteral("data")).toObject();
        endpointVersion = rootReplyObj.value(QStringLiteral("version")).toString();

        // Get configuration
        QJsonObject astarteMqttV1Config = rootReplyObj.value(QStringLiteral("protocols")).toObject()
                                                      .value(QStringLiteral("astarte_mqtt_v1")).toObject();
        if (!astarteMqttV1Config.contains(QStringLiteral("broker_url"))) {
            qCWarning(astarteHttpEndpointDC) << "No broker_url in response: " << response.constData();
            retryConnectToEndpointLater();
            return;
        }

        QString status = rootReplyObj.value(QStringLiteral("status")).toString();

        qCDebug(astarteHttpEndpointDC) << "Device status is " << status;

        QSettings settings(QStringLiteral("%1/mqtt_broker.conf").arg(q->pathToAstarteEndpointConfiguration(endpointName)),
                           QSettings::IniFormat);
        mqttBroker = QUrl::fromUserInput(astarteMqttV1Config.value(QStringLiteral("broker_url")).toString());
        qCDebug(astarteHttpEndpointDC) << "Broker url is " << mqttBroker;

        // Initialize cryptography
        auto processCryptoStatus = [this, q] (bool ready) {
            if (!ready) {
                qCWarning(astarteHttpEndpointDC) << "Could not initialize signature system!!";
                if (!q->isReady()) {
                    q->setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                                    HTTPEndpoint::tr("Could not initialize signature system!"));
                }
                return;
            }

            qCInfo(astarteHttpEndpointDC) << "Signature system initialized correctly!";

            if (!q->isReady()) {
                q->setReady();
            }
        };

        if (Crypto::instance()->isReady()) {
            processCryptoStatus(true);
        } else if (Crypto::instance()->hasInitError()) {
            processCryptoStatus(false);
        } else {
            QObject::connect(Crypto::instance()->init(), &Hemera::Operation::finished, q, [this, q, processCryptoStatus] (Hemera::Operation *op) {
                processCryptoStatus(!op->isError());
            });
        }
    });
}

void HTTPEndpointPrivate::retryConnectToEndpointLater()
{
    Q_Q(HTTPEndpoint);
    int retryInterval = Hyperdrive::Utils::randomizedInterval(RETRY_INTERVAL, 1.0);
    qCWarning(astarteHttpEndpointDC) << "Error while connecting to info endpoint, retrying in " << (retryInterval / 1000) << " seconds";
    QTimer::singleShot(retryInterval, q, SLOT(connectToEndpoint()));
}

void HTTPEndpointPrivate::ensureCredentialsSecret()
{
    Q_Q(HTTPEndpoint);
    DefaultCredentialsSecretProvider *provider = new DefaultCredentialsSecretProvider(q);
    provider->setAgentKey(agentKey);
    provider->setEndpointConfigurationPath(q->pathToAstarteEndpointConfiguration(endpointName));
    provider->setEndpointUrl(endpoint);
    provider->setHardwareId(hardwareId);
    provider->setNAM(nam);
    provider->setSslConfiguration(sslConfiguration);
    credentialsSecretProvider = provider;

    QObject::connect(credentialsSecretProvider, &CredentialsSecretProvider::credentialsSecretReady, q, [this, q] (const QByteArray &credentialsSecret) {
        qCDebug(astarteHttpEndpointDC) << "Credentials secret is: " << credentialsSecret;
        connectToEndpoint();
    });

    credentialsSecretProvider->ensureCredentialsSecret();
}

HTTPEndpoint::HTTPEndpoint(const QString &configurationFile, const QString &persistencyDir, const QUrl& endpoint, const QSslConfiguration &sslConfiguration, QObject* parent)
    : Endpoint(*new HTTPEndpointPrivate(this), endpoint, parent)
{
    Q_D(HTTPEndpoint);
    d->nam = new QNetworkAccessManager(this);
    d->sslConfiguration = sslConfiguration;
    d->configurationFile = configurationFile;
    d->persistencyDir = persistencyDir;
    Crypto::setCryptoBasePath(QStringLiteral("%1/crypto").arg(persistencyDir));

    d->endpointName = endpoint.host();
}

HTTPEndpoint::~HTTPEndpoint()
{
}

QUrl HTTPEndpoint::endpoint() const
{
    Q_D(const HTTPEndpoint);
    return d->endpoint;
}

void HTTPEndpoint::initImpl()
{
    Q_D(HTTPEndpoint);

    QSettings settings(d->configurationFile, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("AstarteTransport")); {
        d->agentKey = settings.value(QStringLiteral("agentKey")).toString().toLatin1();
        d->brokerCa = settings.value(QStringLiteral("brokerCa"), QStringLiteral("/etc/ssl/certs/ca-certificates.crt")).toString();
        d->ignoreSslErrors = settings.value(QStringLiteral("ignoreSslErrors"), false).toBool();
        if (settings.contains(QStringLiteral("pairingCa"))) {
            d->sslConfiguration.setCaCertificates(QSslCertificate::fromPath(settings.value(QStringLiteral("pairingCa")).toString()));
        }
    } settings.endGroup();

    // Grab the hw id
    Hemera::ByteArrayOperation *hwIdOperation = Hemera::Fingerprints::globalHardwareId();
    connect(hwIdOperation, &Hemera::Operation::finished, this, [this, hwIdOperation] {
        if (hwIdOperation->isError()) {
            setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::unhandledRequest()), QStringLiteral("Could not retrieve hardware id!"));
        } else {
            Q_D(HTTPEndpoint);
            d->hardwareId = hwIdOperation->result();

            // Verify the configuration directory is up.
            if (!QFileInfo::exists(pathToAstarteEndpointConfiguration(d->endpointName))) {
                // Create
                qCDebug(astarteHttpEndpointDC) << "Creating Astarte endpoint configuration directory for " << d->endpointName;
                QDir dir;
                if (!dir.mkpath(pathToAstarteEndpointConfiguration(d->endpointName))) {
                    setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()), QStringLiteral("Could not create configuration directory!"));
                }
            }

            d->ensureCredentialsSecret();
        }
    });

    connect(d->nam, &QNetworkAccessManager::sslErrors, this, [this] (QNetworkReply *reply, const QList<QSslError> &errors) {
        Q_D(const HTTPEndpoint);
        qCWarning(astarteHttpEndpointDC) << "SslErrors: " << errors;
        if (d->ignoreSslErrors) {
            reply->ignoreSslErrors(errors);
        }
    });
}

QString HTTPEndpoint::endpointConfigurationBasePath() const
{
    Q_D(const HTTPEndpoint);
    return QStringLiteral("%1/endpoint/").arg(d->persistencyDir);
}

QString HTTPEndpoint::pathToAstarteEndpointConfiguration(const QString &endpointName) const
{
    return QStringLiteral("%1%2").arg(endpointConfigurationBasePath(), endpointName);
}

QNetworkReply *HTTPEndpoint::sendRequest(const QString& relativeEndpoint, const QByteArray& payload, Crypto::AuthenticationDomain authenticationDomain)
{
    Q_D(const HTTPEndpoint);
    QNetworkRequest req;
    if (authenticationDomain == Crypto::DeviceAuthenticationDomain) {
        // Build the endpoint
        QUrl target = d->endpoint;
        target.setPath(QStringLiteral("%1/devices/%2%3").arg(d->endpoint.path()).arg(QString::fromLatin1(d->hardwareId)).arg(relativeEndpoint));
        req.setUrl(target);

        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setSslConfiguration(d->sslConfiguration);

        qCDebug(astarteHttpEndpointDC) << "Request is: " << relativeEndpoint << payload << authenticationDomain;

        req.setRawHeader("Authorization", "Bearer " + d->credentialsSecretProvider->credentialsSecret());
        req.setRawHeader("X-Astarte-Transport-Provider", "Astarte Device SDK Qt5");
        req.setRawHeader("X-Astarte-Transport-Version", QStringLiteral("%1.%2.%3")
                                                     .arg(Hyperdrive::StaticConfig::hyperdriveMajorVersion())
                                                     .arg(Hyperdrive::StaticConfig::hyperdriveMinorVersion())
                                                     .arg(Hyperdrive::StaticConfig::hyperdriveReleaseVersion())
                                                     .toLatin1());
    } else {
        qCWarning(astarteHttpEndpointDC) << "Only DeviceAuthenticationDomain can be used in astartehttpendpoint";
    }

    return d->nam->post(req, payload);
}

Hemera::Operation* HTTPEndpoint::pair(bool force)
{
    if (!force && isPaired()) {
        return new Hemera::FailureOperation(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()),
                                            tr("This device is already paired to this Astarte endpoint"));
    }

    return new PairOperation(this);
}

Hemera::Operation* HTTPEndpoint::verifyCertificate()
{
    Q_D(const HTTPEndpoint);
    QFile cert(QStringLiteral("%1/mqtt_broker.crt").arg(pathToAstarteEndpointConfiguration(d->endpointName)));
    return new VerifyCertificateOperation(cert, this);
}

Hyperdrive::MQTTClientWrapper *HTTPEndpoint::createMqttClientWrapper()
{
    if (!isReady() || mqttBrokerUrl().isEmpty()) {
        return nullptr;
    }

    Q_D(const HTTPEndpoint);

    // Create the client ID
    QList < QSslCertificate > certificates = QSslCertificate::fromPath(QStringLiteral("%1/mqtt_broker.crt")
                                                    .arg(pathToAstarteEndpointConfiguration(d->endpointName)), QSsl::Pem);
    if (certificates.size() != 1) {
        qCWarning(astarteHttpEndpointDC) << "Could not retrieve device certificate!";
        return nullptr;
    }

    QByteArray customerId = certificates.first().subjectInfo(QSslCertificate::CommonName).first().toLatin1();

    Hyperdrive::MQTTClientWrapper *c = new Hyperdrive::MQTTClientWrapper(mqttBrokerUrl(), customerId, this);

    c->setCleanSession(false);
    c->setPublishQoS(Hyperdrive::MQTTClientWrapper::AtMostOnceQoS);
    c->setSubscribeQoS(Hyperdrive::MQTTClientWrapper::AtMostOnceQoS);
    c->setKeepAlive(300);
    c->setIgnoreSslErrors(d->ignoreSslErrors);

    // SSL
    c->setMutualSSLAuthentication(d->brokerCa, Crypto::pathToPrivateKey(),
                                  QStringLiteral("%1/mqtt_broker.crt").arg(pathToAstarteEndpointConfiguration(d->endpointName)));

    QSettings settings(QStringLiteral("%1/mqtt_broker.conf").arg(pathToAstarteEndpointConfiguration(d->endpointName)),
                       QSettings::IniFormat);

    return c;
}

QString HTTPEndpoint::endpointVersion() const
{
    Q_D(const HTTPEndpoint);
    return d->endpointVersion;
}

QUrl HTTPEndpoint::mqttBrokerUrl() const
{
    Q_D(const HTTPEndpoint);
    return d->mqttBroker;
}

bool HTTPEndpoint::isPaired() const
{
    Q_D(const HTTPEndpoint);
    return QFile::exists(QStringLiteral("%1/mqtt_broker.crt").arg(pathToAstarteEndpointConfiguration(d->endpointName)));
}

}

#include "moc_astartehttpendpoint.cpp"
