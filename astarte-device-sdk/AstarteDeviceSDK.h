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

#ifndef ASTARTEDEVICESDK_H
#define ASTARTEDEVICESDK_H

#include <HemeraCore/AsyncInitObject>

#include <HyperspaceProducerConsumer/ProducerAbstractInterface>

namespace Hyperdrive {
class AstarteTransport;
class Interface;
}

class AstarteGenericConsumer;
class AstarteGenericProducer;
class QJsonSchemaChecker;

class AstarteDeviceSDK : public Hemera::AsyncInitObject
{
    Q_OBJECT

public:
    AstarteDeviceSDK(const QString &configurationPath, const QString &interfacesDir,
                     const QByteArray &hardwareId, QObject *parent = nullptr);
    ~AstarteDeviceSDK();

    bool sendData(const QByteArray &interface, const QByteArray &path, const QVariant &value,
            const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());

Q_SIGNALS:
    void unsetReceived(const QByteArray &interface, const QByteArray &path);
    void dataReceived(const QByteArray &interface, const QByteArray &path, const QVariant &value);

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

    QVariant::Type typeStringToVariantType(const QString &typeString) const;
    Hyperspace::Retention retentionStringToRetention(const QString &retentionString) const;
    Hyperspace::Reliability reliabilityStringToReliability(const QString &reliabilityString) const;

    void receiveValue(const QByteArray &interface, const QByteArray &path, const QVariant &value);
    void unsetValue(const QByteArray &interface, const QByteArray &path);

    friend class AstarteGenericConsumer;
};

#endif
