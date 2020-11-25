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

#ifndef HYPERDRIVE_MQTTCLIENTWRAPPER_H
#define HYPERDRIVE_MQTTCLIENTWRAPPER_H

#include <HemeraCore/AsyncInitObject>
#include <HemeraCore/Operation>

#include <QtCore/QUrl>

namespace Hemera {
class Operation;
}

namespace Hyperdrive {

class MQTTClientWrapperPrivate;
class MQTTClientWrapper : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MQTTClientWrapper)
    Q_DECLARE_PRIVATE_D(d_h_ptr, MQTTClientWrapper)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
    enum Status {
        UnknownStatus = 0,
        DisconnectedStatus = 1,
        ConnectedStatus = 2,
        ConnectingStatus = 3,
        DisconnectingStatus = 4,
        ReconnectingStatus = 5
    };
    Q_ENUM(Status);
    enum MQTTQoS {
        AtMostOnceQoS = 0,
        AtLeastOnceQoS = 1,
        ExactlyOnceQoS = 2,
        DefaultQoS = 99
    };
    Q_ENUM(MQTTQoS);

    explicit MQTTClientWrapper(const QUrl &host, QObject *parent);
    explicit MQTTClientWrapper(const QUrl &host, const QByteArray &clientId, QObject *parent = nullptr);
    virtual ~MQTTClientWrapper();

    Status status() const;
    QByteArray hardwareId() const;
    QByteArray rootClientTopic() const;
    QDateTime clientCertificateExpiry() const;
    bool sessionPresent() const;

    void setMutualSSLAuthentication(const QString &pathToCA, const QString &pathToPKey, const QString &pathToCertificate);
    void setPublishQoS(MQTTQoS qos);
    void setSubscribeQoS(MQTTQoS qos);
    void setIgnoreSslErrors(bool ignoreSslErrors);
    /// Note: this will only work if set before initializing the Client.
    void setCleanSession(bool cleanSession = true);
    void setKeepAlive(quint64 seconds);
    void setLastWill(const QByteArray &topic, const QByteArray &message, MQTTQoS qos, bool retained = false);

    int publish(const QByteArray &topic, const QByteArray &payload, MQTTQoS qos = DefaultQoS, bool retained = false);
    void subscribe(const QByteArray &topic, MQTTQoS qos = DefaultQoS);

public Q_SLOTS:
    bool connectToBroker();
    bool disconnectFromBroker();

protected:
    virtual void initImpl() override final;

Q_SIGNALS:
    void messageReceived(const QByteArray &topic, const QByteArray &payload);
    void statusChanged(Hyperdrive::MQTTClientWrapper::Status status);
    void connectionLost(const QString &cause);
    void publishConfirmed(int mid);
    void connectionFailed();
    void connectionStarted();
    void connackReceived();
    void connackTimeout();

};
}

#endif // HYPERDRIVE_MQTTCLIENTWRAPPER_H
