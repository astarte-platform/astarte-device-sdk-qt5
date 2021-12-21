/*
 * This file is part of Astarte.
 *
 * Copyright 2017-2021 Ispirata Srl
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

#ifndef ASTARTEDEVICESDK_H
#define ASTARTEDEVICESDK_H

#include <HemeraCore/AsyncInitObject>

#include <HyperspaceProducerConsumer/ProducerAbstractInterface>
#include <astartetransport.h>
#include <QtCore/QPair>

namespace Hyperdrive {
class Interface;
}

class AstarteGenericConsumer;
class AstarteGenericProducer;
class QJsonSchemaChecker;

enum EndpointType { AstarteScalarType, AstarteArrayType };

class AstarteDeviceSDK : public Hemera::AsyncInitObject
{
    Q_OBJECT

public:
    enum ConnectionStatus {
        UnknownStatus = Hyperdrive::AstarteTransport::UnknownStatus,
        DisconnectedStatus = Hyperdrive::AstarteTransport::DisconnectedStatus,
        ConnectedStatus = Hyperdrive::AstarteTransport::ConnectedStatus,
        ConnectingStatus = Hyperdrive::AstarteTransport::ConnectingStatus,
        DisconnectingStatus = Hyperdrive::AstarteTransport::DisconnectingStatus,
        ReconnectingStatus = Hyperdrive::AstarteTransport::ReconnectingStatus
    };
    #if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    Q_ENUMS(AstarteDeviceSDK::ConnectionStatus)
    #else
    Q_ENUM(AstarteDeviceSDK::ConnectionStatus)
    #endif

    AstarteDeviceSDK(const QString &configurationPath, const QString &interfacesDir,
                     const QByteArray &hardwareId, QObject *parent = nullptr);
    ~AstarteDeviceSDK();

    bool sendData(const QByteArray &interface, const QByteArray &path, const QVariant &value,
            const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());

    bool sendData(const QByteArray &interface, const QByteArray &path, const QVariant &value,
            const QVariantHash &metadata);

    bool sendData(const QByteArray &interface, const QVariantHash &value, const QDateTime &timestamp = QDateTime(),
            const QVariantHash &metadata = QVariantHash());

    bool sendData(const QByteArray &interface, const QVariantHash &value, const QVariantHash &metadata);

    template <typename T> bool sendData(const QByteArray &interface, const QByteArray &path, const QList<T> &value,
            const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());

    bool sendUnset(const QByteArray &interface, const QByteArray &path);

    ConnectionStatus connectionStatus() const;

    bool connectToAstarte();
    bool disconnectFromAstarte();

Q_SIGNALS:
    void unsetReceived(const QByteArray &interface, const QByteArray &path);
    void dataReceived(const QByteArray &interface, const QByteArray &path, const QVariant &value);
    void connectionStatusChanged();

protected Q_SLOTS:

    void initImpl() override final;

private:
    QByteArray m_hardwareId;
    Hyperdrive::AstarteTransport *m_astarteTransport;

    QJsonSchemaChecker *m_checker;
    QString m_configurationPath;
    QString m_interfacesDir;

    QHash<QByteArray, AstarteGenericProducer *> m_producers;
    QHash<QByteArray, AstarteGenericConsumer *> m_consumers;

    void loadInterfaces();

    void createProducer(const Hyperdrive::Interface &interface, const QJsonObject &producerObject);
    void createConsumer(const Hyperdrive::Interface &interface, const QJsonObject &consumerObject);

    QPair<EndpointType, QVariant::Type> typeStringToVariantType(const QString &typeString) const;
    Hyperspace::Retention retentionStringToRetention(const QString &retentionString) const;
    Hyperspace::Reliability reliabilityStringToReliability(const QString &reliabilityString) const;

    void receiveValue(const QByteArray &interface, const QByteArray &path, const QVariant &value);
    void unsetValue(const QByteArray &interface, const QByteArray &path);

    friend class AstarteGenericConsumer;
};

#endif
