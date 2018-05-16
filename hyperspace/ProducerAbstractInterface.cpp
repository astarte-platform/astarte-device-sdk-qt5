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

#include <HyperspaceProducerConsumer/ProducerAbstractInterface>

#include <HyperspaceCore/BSONDocument>

#include <HyperspaceCore/BSONSerializer>
#include <HyperspaceCore/Fluctuation>

#define METHOD_ERROR "ERROR"

namespace Hyperspace
{

namespace ProducerConsumer
{

class ProducerAbstractInterface::Private
{
    public:
        typedef QPair<int, QByteArray> StatePair;

        QHash<StatePair, int> transitions;
        QHash<int, int> acceptingStates;
};

ProducerAbstractInterface::ProducerAbstractInterface(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, QObject *parent)
   : AbstractWaveTarget(interface, astarteTransport, parent),
     d(new Private)
{
}

ProducerAbstractInterface::~ProducerAbstractInterface()
{
}

void ProducerAbstractInterface::waveFunction(const Wave &wave)
{
    ResponseCode response;
    // We only support ERROR waves in Producers
    if (wave.method() != METHOD_ERROR) {
        response = ResponseCode::NotImplemented;
    } else {
        QList<QByteArray> noRootTokens = wave.target().mid(1).split('/');

        int dIndex = dispatchIndex(noRootTokens);
        DispatchResult result = dispatch(dIndex, wave.payload(), noRootTokens);

        switch (result) {
            case Success:
                response = ResponseCode::OK;
                break;

            case IndexNotFound:
                response = ResponseCode::NotFound;
                break;

            case CouldNotConvertPayload:
                response = ResponseCode::BadRequest;
                break;
        }
    }

    sendRebound(Rebound(wave.id(), response));
}

void ProducerAbstractInterface::insertTransition(int state, QByteArray token, int newState)
{
    d->transitions.insert(Private::StatePair(state, token), newState);
}

void ProducerAbstractInterface::insertDispatchState(int state, int dispatchIndex)
{
    d->acceptingStates.insert(state, dispatchIndex);
}

int ProducerAbstractInterface::dispatchIndex(const QList<QByteArray> &inputTokens)
{
   if (d->acceptingStates.isEmpty()) {
        populateTokensAndStates();
        d->transitions.squeeze();
        d->acceptingStates.squeeze();
    }

    QList<int> currentStates;
    currentStates.append(0);

    QList<int> nextStates;

    for (const QByteArray token : inputTokens) {
        nextStates = QList<int>();

        for (int i = 0; i < currentStates.count(); i++) {
            if (d->transitions.contains(Private::StatePair(currentStates.at(i), token))) {
                nextStates.append(d->transitions.value(Private::StatePair(currentStates.at(i), token)));
                // qDebug() << "tok: " << token;
            }
            if (d->transitions.contains(Private::StatePair(currentStates.at(i), QByteArray()))) {
                nextStates.append(d->transitions.value(Private::StatePair(currentStates.at(i), QByteArray())));
                // qDebug() << "epsi";
            }
        }

        // qDebug() << "next states: " << nextStates;

        if (Q_UNLIKELY(nextStates.isEmpty())) {
            return -1;
        }

        currentStates = nextStates;
    }

    for (int i = 0; i < currentStates.count(); i++) {
        if (d->acceptingStates.contains(currentStates.at(i))) {
            return d->acceptingStates.value(currentStates.at(i));
        }
    }

    return -1;
}

void ProducerAbstractInterface::sendRawDataOnEndpoint(const QByteArray &value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes)
{
    if (!target.startsWith('/') || target.endsWith('/') || target.contains("//")
            || target.contains(";") || target.contains('\n') || target.contains('\r')
            || target.contains('\b') || target.contains('\t') || target.contains('\v')
            || target.contains('\f') || target.contains('+') || target.contains('#') ) {
        qWarning() << "ProducerAbstractInterface: invalid target: " << target << ". discarding value: " << value;
        return;
    }

    Fluctuation fluctuation;
    fluctuation.setPayload(value);
    fluctuation.setAttributes(attributes);
    sendFluctuation(target, fluctuation);
}

void ProducerAbstractInterface::sendDataOnEndpoint(const QByteArray &value, const QByteArray &target,
        const QHash<QByteArray, QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendBinaryValue("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

void ProducerAbstractInterface::sendDataOnEndpoint(double value, const QByteArray &target,
        const QHash<QByteArray, QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendDoubleValue("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

void ProducerAbstractInterface::sendDataOnEndpoint(int value, const QByteArray &target,
        const QHash<QByteArray, QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendInt32Value("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

void ProducerAbstractInterface::sendDataOnEndpoint(qint64 value, const QByteArray &target, const QHash<QByteArray,
        QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendInt64Value("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

void ProducerAbstractInterface::sendDataOnEndpoint(bool value, const QByteArray &target,
        const QHash<QByteArray, QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendBooleanValue("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

void ProducerAbstractInterface::sendDataOnEndpoint(const QString &value, const QByteArray &target,
        const QHash<QByteArray, QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendString("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

void ProducerAbstractInterface::sendDataOnEndpoint(const QDateTime &value, const QByteArray &target,
        const QHash<QByteArray, QByteArray> &attributes, const QDateTime &timestamp, const QVariantHash &metadata)
{
    Util::BSONSerializer serializer;
    serializer.appendDateTime("v", value);
    if (!timestamp.isNull() && timestamp.isValid()) {
        serializer.appendDateTime("t", timestamp);
    }
    if (!metadata.isEmpty()) {
        serializer.appendDocument("m", metadata);
    }
    serializer.appendEndOfDocument();
    sendRawDataOnEndpoint(serializer.document(), target, attributes);
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, QByteArray *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = QByteArray();
        return false;
    }

    *value = doc.byteArrayValue("v");
    return true;
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, int *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = 0;
        return false;
    }

    *value = doc.int32Value("v");
    return true;
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, qint64 *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = 0;
        return false;
    }

    *value = doc.int64Value("v");
    return true;
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, bool *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = false;
        return false;
    }

    *value = doc.booleanValue("v");
    return true;
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, double *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = 0.0;
        return false;
    }

    *value = doc.doubleValue("v");
    return true;
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, QString *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = QString();
        return false;
    }

    *value = doc.stringValue("v");
    return true;
}

bool ProducerAbstractInterface::payloadToValue(const QByteArray &payload, QDateTime *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = QDateTime();
        return false;
    }

    *value = doc.dateTimeValue("v");
    return true;
}

}
}
