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

#ifndef HYPERDRIVE_ASTARTETRANSPORT_H
#define HYPERDRIVE_ASTARTETRANSPORT_H

#include <hyperdrivemqttclientwrapper.h>

#include <HemeraCore/AsyncInitObject>

#include <QtCore/QPointer>
#include <QtCore/QSet>

class QTimer;

namespace Astarte {
class Endpoint;
}

namespace Hyperspace {
class Fluctuation;
class Rebound;
class Wave;
}

namespace Hyperdrive {
class MQTTClientWrapper;
class CacheMessage;
class Interface;

class AstarteTransport : public Hemera::AsyncInitObject
{
    Q_OBJECT

public:
    enum ConnectionStatus {
        UnknownStatus = MQTTClientWrapper::UnknownStatus,
        DisconnectedStatus = MQTTClientWrapper::DisconnectedStatus,
        ConnectedStatus = MQTTClientWrapper::ConnectedStatus,
        ConnectingStatus = MQTTClientWrapper::ConnectingStatus,
        DisconnectingStatus = MQTTClientWrapper::DisconnectingStatus,
        ReconnectingStatus = MQTTClientWrapper::ReconnectingStatus
    };

    #if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    Q_ENUMS(Hyperdrive::AstarteTransport::ConnectionStatus)
    #else
    Q_ENUM(Hyperdrive::AstarteTransport::ConnectionStatus)
    #endif

    AstarteTransport(const QString &configurationPath, QObject *parent = Q_NULLPTR);
    virtual ~AstarteTransport();

    virtual void rebound(const Hyperspace::Rebound& rebound, int fd = -1);
    virtual void fluctuation(const Hyperspace::Fluctuation& fluctuation);
    virtual void cacheMessage(const CacheMessage& cacheMessage);
    virtual void bigBang();

    QHash< QByteArray, Hyperdrive::Interface > introspection() const;
    void setIntrospection(const QHash< QByteArray, Hyperdrive::Interface > &introspection);

    ConnectionStatus connectionStatus() const;

    bool connectToBroker();
    bool disconnectFromBroker();

Q_SIGNALS:
    void introspectionChanged();
    void connectionStatusChanged();
    void waveReceived(const QByteArray &interface, const Hyperspace::Wave &wave);

protected:
    virtual void initImpl();

    void routeWave(const Hyperspace::Wave &wave, int fd);

private Q_SLOTS:
    void startPairing(bool forcedPairing);
    void setupMqtt();
    void setupClientSubscriptions();
    void sendProperties();
    void resendFailedMessages();
    void publishIntrospection();
    void onStatusChanged(MQTTClientWrapper::Status status);
    void onMQTTMessageReceived(const QByteArray &topic, const QByteArray &payload);
    void onPublishConfirmed(int messageId);
    void handleFailedPublish(const CacheMessage &cacheMessage);
    void handleConnectionFailed();
    void handleConnackTimeout();
    void handleRebootTimerTimeout();
    void forceNewPairing();

private:
    QByteArray introspectionString() const;

    Astarte::Endpoint *m_astarteEndpoint;
    QPointer<MQTTClientWrapper> m_mqttBroker;
    QHash< quint64, Hyperspace::Wave > m_waveStorage;
    QHash< quint64, QByteArray > m_commandTree;
    QHash< QByteArray, Hyperdrive::Interface > m_introspection;
    QString m_configurationPath;
    QString m_persistencyDir;
    QTimer *m_rebootTimer;
    QByteArray m_lastSentIntrospection;
    QByteArray m_inFlightIntrospection;
    bool m_synced;
    bool m_rebootWhenConnectionFails;
    int m_rebootDelayMinutes;
    int m_keepAliveSeconds;
    int m_inFlightIntrospectionMessageId;
};
}

#endif // HYPERDRIVE_ASTARTETRANSPORT_H
