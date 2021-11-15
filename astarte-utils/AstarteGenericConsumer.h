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

#ifndef ASTARTE_GENERIC_CONSUMER_H
#define ASTARTE_GENERIC_CONSUMER_H

#include <HyperspaceProducerConsumer/ConsumerAbstractAdaptor>

#include <AstarteDeviceSDK.h>

namespace Hyperdrive {
class AstarteTransport;
}

class AstarteGenericConsumer : public Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor
{
    Q_OBJECT
    Q_DISABLE_COPY(AstarteGenericConsumer)

public:
    explicit AstarteGenericConsumer(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, AstarteDeviceSDK *parent = nullptr);
    virtual ~AstarteGenericConsumer();

    void setMappingToTokens(const QHash<QByteArray, QByteArrayList> &mappingToTokens);
    void setMappingToType(const QHash<QByteArray, QVariant::Type> &mappingToType);
    void setMappingToArrayType(const QHash<QByteArray, QVariant::Type> &mappingToType);
    void setMappingToAllowUnset(const QHash<QByteArray, bool> &mappingToAllowUnset);

protected:
    virtual void populateTokensAndStates() override final;
    virtual Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult dispatch(int i, const QByteArray &payload, const QList<QByteArray> &inputTokens) override final;

private:
    inline AstarteDeviceSDK *parent() const { return static_cast<AstarteDeviceSDK *>(QObject::parent()); }

    QHash<QByteArray, QByteArrayList> m_mappingToTokens;
    QHash<QByteArray, QVariant::Type> m_mappingToType;
    QHash<QByteArray, QVariant::Type> m_mappingToArrayType;
    QHash<QByteArray, bool> m_mappingToAllowUnset;
};

#endif // ASTARTE_GENERIC_CONSUMER_H
