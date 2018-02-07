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
    AstarteTransport(const QString &configurationPath, QObject *parent = Q_NULLPTR);
    virtual ~AstarteTransport();

    virtual void rebound(const Hyperspace::Rebound& rebound, int fd = -1);
    virtual void fluctuation(const Hyperspace::Fluctuation& fluctuation);
    virtual void cacheMessage(const CacheMessage& cacheMessage);
    virtual void bigBang();

    QHash< QByteArray, Hyperdrive::Interface > introspection() const;
    void setIntrospection(const QHash< QByteArray, Hyperdrive::Interface > &introspection);

Q_SIGNALS:
    void introspectionChanged();
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
    Astarte::Endpoint *m_astarteEndpoint;
    QPointer<MQTTClientWrapper> m_mqttBroker;
    QHash< quint64, Hyperspace::Wave > m_waveStorage;
    QHash< quint64, QByteArray > m_commandTree;
    QHash< QByteArray, Hyperdrive::Interface > m_introspection;
    QString m_configurationPath;
    QString m_persistencyDir;
    QTimer *m_rebootTimer;
    bool m_synced;
    bool m_rebootWhenConnectionFails;
    int m_rebootDelayMinutes;
};
}

#endif // HYPERDRIVE_ASTARTETRANSPORT_H
