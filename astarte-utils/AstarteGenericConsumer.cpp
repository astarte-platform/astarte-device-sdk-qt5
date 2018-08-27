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

#include "AstarteGenericConsumer.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>

AstarteGenericConsumer::AstarteGenericConsumer(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, AstarteDeviceSDK *parent)
    : Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor(interface, astarteTransport, parent)
{
}

AstarteGenericConsumer::~AstarteGenericConsumer()
{
}

void AstarteGenericConsumer::setMappingToTokens(const QHash<QByteArray, QByteArrayList> &mappingToTokens)
{
    m_mappingToTokens = mappingToTokens;
}

void AstarteGenericConsumer::setMappingToType(const QHash<QByteArray, QVariant::Type> &mappingToType)
{
    m_mappingToType = mappingToType;
}

void AstarteGenericConsumer::setMappingToAllowUnset(const QHash<QByteArray, bool> &mappingToAllowUnset)
{
    m_mappingToAllowUnset = mappingToAllowUnset;
}

void AstarteGenericConsumer::populateTokensAndStates()
{
}

Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult AstarteGenericConsumer::dispatch(int i, const QByteArray &payload,
                                                                                                       const QList<QByteArray> &inputTokens)
{
    QHash<QByteArray, QByteArrayList>::const_iterator it;

    for (it = m_mappingToTokens.constBegin(); it != m_mappingToTokens.constEnd(); it++) {
        if (inputTokens.size() != it.value().size()) {
            continue;
        }

        bool mappingMatch = true;
        for (int i = 0; i < inputTokens.size(); ++i) {
            if (it.value().at(i).startsWith("%{")) {
                continue;
            }

            if (inputTokens.at(i) != it.value().at(i)) {
                mappingMatch = false;
            }
        }

        if (!mappingMatch) {
            continue;
        }

        QByteArray matchedMapping = it.key();

        QByteArray path = QByteArray("/%1").replace("%1", inputTokens.join('/'));

        if (payload.isEmpty()) {
            if (m_mappingToAllowUnset.value(matchedMapping)) {
                parent()->unsetValue(interface(), path);
                return Success;
            } else {
                return CouldNotConvertPayload;
            }
        }

        switch (m_mappingToType.value(matchedMapping)) {
            case QMetaType::Bool: {
                bool value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            case QMetaType::Int: {
                int value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            case QMetaType::LongLong: {
                qint64 value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            case QMetaType::QByteArray: {
                QByteArray value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            case QMetaType::Double: {
                double value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            case QMetaType::QString: {
                QString value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            case QMetaType::QDateTime: {
                QDateTime value;
                if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
                parent()->receiveValue(interface(), path, value);
                return Success;
            }
            default:
                return CouldNotConvertPayload;
        }
    }

    // If we're here, mapping not found
    return IndexNotFound;
}
