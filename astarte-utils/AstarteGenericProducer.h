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

    bool sendData(const QVariant &value, const QByteArray &target,
            const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
    bool unsetPath(const QByteArray &target);

    void setMappingToTokens(const QHash<QByteArray, QByteArrayList> &mappingToTokens);
    void setMappingToType(const QHash<QByteArray, QVariant::Type> &mappingToType);
    void setMappingToRetention(const QHash<QByteArray, Hyperspace::Retention> &m_mappingToRetention);
    void setMappingToReliability(const QHash<QByteArray, Hyperspace::Reliability> &m_mappingToReliability);
    void setMappingToExpiry(const QHash<QByteArray, int> &m_mappingToExpiry);
    void setMappingToAllowUnset(const QHash<QByteArray, bool> &mappingToAllowUnset);

protected:
    virtual void populateTokensAndStates() override final;
    virtual Hyperspace::ProducerConsumer::ProducerAbstractInterface::DispatchResult dispatch(int i, const QByteArray &payload, const QList<QByteArray> &inputTokens) override final;

private:
    QHash<QByteArray, QByteArrayList> m_mappingToTokens;
    QHash<QByteArray, QVariant::Type> m_mappingToType;
    QHash<QByteArray, Hyperspace::Retention> m_mappingToRetention;
    QHash<QByteArray, Hyperspace::Reliability> m_mappingToReliability;
    QHash<QByteArray, int> m_mappingToExpiry;
    QHash<QByteArray, bool> m_mappingToAllowUnset;

    Hyperdrive::Interface::Type m_interfaceType;
};

#endif // ASTARTE_GENERIC_PRODUCER_H
