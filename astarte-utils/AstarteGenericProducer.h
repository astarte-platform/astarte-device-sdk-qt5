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

#ifndef ASTARTE_GENERIC_PRODUCER_H
#define ASTARTE_GENERIC_PRODUCER_H

#include <HyperspaceProducerConsumer/ProducerAbstractInterface>

#include <hyperdriveinterface.h>

namespace Hyperdrive {
class AstarteTransport;
}

class AstarteGenericProducer : public Hyperspace::ProducerConsumer::ProducerAbstractInterface
{
    Q_OBJECT
    Q_DISABLE_COPY(AstarteGenericProducer)

public:
    explicit AstarteGenericProducer(const QByteArray &interface, Hyperdrive::Interface::Type interfaceType,
                                    Hyperdrive::AstarteTransport *astarteTransport, QObject *parent = nullptr);
    virtual ~AstarteGenericProducer();

    bool sendData(const QVariant &value, const QByteArray &target);

    void setMappingToTokens(const QHash<QByteArray, QByteArrayList> &mappingToTokens);
    void setMappingToType(const QHash<QByteArray, QVariant::Type> &mappingToType);
    void setMappingToRetention(const QHash<QByteArray, Hyperspace::Retention> &m_mappingToRetention);
    void setMappingToReliability(const QHash<QByteArray, Hyperspace::Reliability> &m_mappingToReliability);
    void setMappingToExpiry(const QHash<QByteArray, int> &m_mappingToExpiry);

protected:
    virtual void populateTokensAndStates() override final;
    virtual Hyperspace::ProducerConsumer::ProducerAbstractInterface::DispatchResult dispatch(int i, const QByteArray &payload, const QList<QByteArray> &inputTokens) override final;

private:
    QHash<QByteArray, QByteArrayList> m_mappingToTokens;
    QHash<QByteArray, QVariant::Type> m_mappingToType;
    QHash<QByteArray, Hyperspace::Retention> m_mappingToRetention;
    QHash<QByteArray, Hyperspace::Reliability> m_mappingToReliability;
    QHash<QByteArray, int> m_mappingToExpiry;

    Hyperdrive::Interface::Type m_interfaceType;
};

#endif // ASTARTE_GENERIC_PRODUCER_H
