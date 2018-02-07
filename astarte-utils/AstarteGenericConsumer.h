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
    void setMappingToAllowUnset(const QHash<QByteArray, bool> &mappingToAllowUnset);

protected:
    virtual void populateTokensAndStates() override final;
    virtual Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult dispatch(int i, const QByteArray &payload, const QList<QByteArray> &inputTokens) override final;

private:
    inline AstarteDeviceSDK *parent() const { return static_cast<AstarteDeviceSDK *>(QObject::parent()); }

    QHash<QByteArray, QByteArrayList> m_mappingToTokens;
    QHash<QByteArray, QVariant::Type> m_mappingToType;
    QHash<QByteArray, bool> m_mappingToAllowUnset;
};

#endif // ASTARTE_GENERIC_CONSUMER_H
