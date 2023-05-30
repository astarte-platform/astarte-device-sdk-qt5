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

#include "AstarteGenericConsumer.h"
#include "BSONDocument.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>

AstarteGenericConsumer::AstarteGenericConsumer(const QByteArray &interface,
    Hyperdrive::Interface::Type interfaceType,
    Hyperdrive::Interface::Aggregation interfaceAggregation,
    Hyperdrive::AstarteTransport *astarteTransport, AstarteDeviceSDK *parent)
    : Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor(interface, astarteTransport, parent)
    , m_interfaceType(interfaceType)
    , m_interfaceAggregation(interfaceAggregation)
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

void AstarteGenericConsumer::setMappingToArrayType(const QHash<QByteArray, QVariant::Type> &mappingToArrayType)
{
    m_mappingToArrayType = mappingToArrayType;
}

void AstarteGenericConsumer::setMappingToAllowUnset(const QHash<QByteArray, bool> &mappingToAllowUnset)
{
    m_mappingToAllowUnset = mappingToAllowUnset;
}

void AstarteGenericConsumer::populateTokensAndStates()
{
}

Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult
AstarteGenericConsumer::dispatch(
    int i, const QByteArray &payload, const QList<QByteArray> &inputTokens)
{
    Q_UNUSED(i);
    if (m_interfaceType == Hyperdrive::Interface::Type::DataStream
        && m_interfaceAggregation == Hyperdrive::Interface::Aggregation::Object) {
        return dispatchObject(i, payload, inputTokens);
    }
    return dispatchIndividual(i, payload, inputTokens);
}

Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult
AstarteGenericConsumer::dispatchIndividual(
    int i, const QByteArray &payload, const QList<QByteArray> &inputTokens)
{
    Q_UNUSED(i);
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

        if (m_mappingToArrayType.contains(matchedMapping)){
            QList<QVariant> value;
            if (!payloadToValue(payload, &value)) return CouldNotConvertPayload;
            parent()->receiveValue(interface(), path, value);
            return Success;
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

Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult
AstarteGenericConsumer::dispatchObject(
    int i, const QByteArray &payload, const QList<QByteArray> &inputTokens)
{
    Q_UNUSED(i);
    QHash<QByteArray, QByteArrayList>::const_iterator mappingIt;

    QHash<QByteArray, QVariant> aggregateObjPayload;
    QHash<QByteArray, QVariant> astarteAggregateValue;

    if (!payloadToValue(payload, &aggregateObjPayload)) {
        return CouldNotConvertPayload;
    }

    QByteArray interfacePath = QByteArray("/%1").replace("%1", inputTokens.join('/'));

    for (auto aggregateIt = aggregateObjPayload.cbegin(), end = aggregateObjPayload.cend();
         aggregateIt != end; ++aggregateIt) {

        QByteArray path;
        path.append(interfacePath).append("/").append(aggregateIt.key());
        QList<QByteArray> paths = path.mid(1).split('/');

        for (mappingIt = m_mappingToTokens.constBegin(); mappingIt != m_mappingToTokens.constEnd();
             mappingIt++) {

            if (paths.size() != mappingIt.value().size()) {
                continue;
            }

            bool mappingMatch = true;
            for (int pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
                if (mappingIt.value().at(pathIndex).startsWith("%{")) {
                    continue;
                }

                if (paths.at(pathIndex) != mappingIt.value().at(pathIndex)) {
                    mappingMatch = false;
                }
            }

            if (!mappingMatch) {
                continue;
            }

            QByteArray matchedMapping = mappingIt.key();

            if (m_mappingToArrayType.contains(matchedMapping)) {
                if (aggregateIt.value().type() != QVariant::Type::List) {
                    return CouldNotConvertPayload;
                }
            } else {
                switch (m_mappingToType.value(matchedMapping)) {
                    case QMetaType::Bool: {
                        if (aggregateIt.value().type() != QVariant::Type::Bool)
                            return CouldNotConvertPayload;
                        break;
                    }
                    case QMetaType::Int: {
                        if (aggregateIt.value().type() != QVariant::Type::Int)
                            return CouldNotConvertPayload;
                        break;
                    }
                    case QMetaType::LongLong: {
                        if (aggregateIt.value().type() != QVariant::Type::LongLong)
                            return CouldNotConvertPayload;
                        break;
                    }
                    case QMetaType::QByteArray: {
                        if (aggregateIt.value().type() != QVariant::Type::ByteArray)
                            return CouldNotConvertPayload;
                        break;
                    }
                    case QMetaType::Double: {
                        if (aggregateIt.value().type() != QVariant::Type::Double)
                            return CouldNotConvertPayload;
                        break;
                    }
                    case QMetaType::QString: {
                        if (aggregateIt.value().type() != QVariant::Type::String)
                            return CouldNotConvertPayload;
                        break;
                    }
                    case QMetaType::QDateTime: {
                        if (aggregateIt.value().type() != QVariant::Type::DateTime)
                            return CouldNotConvertPayload;
                        break;
                    }
                    default:
                        return CouldNotConvertPayload;
                }
            }
            astarteAggregateValue.insert(aggregateIt.key(), aggregateIt.value());
        }
    }

    parent()->receiveValue(interface(), interfacePath, QVariant::fromValue(astarteAggregateValue));

    return Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::Success;
}
