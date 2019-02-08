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

#include <HyperspaceProducerConsumer/ConsumerAbstractAdaptor>

#include <HyperspaceCore/BSONDocument>

#include <QtCore/QDate>
#include <QtCore/QHash>
#include <QtCore/QPair>

namespace Hyperspace
{

namespace ProducerConsumer
{

class ConsumerAbstractAdaptor::Private
{
    public:
        typedef QPair<int, QByteArray> StatePair;

        QHash<StatePair, int> transitions;
        QHash<int, int> acceptingStates;
};

ConsumerAbstractAdaptor::ConsumerAbstractAdaptor(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, QObject *parent)
    : AbstractWaveTarget(interface, astarteTransport, parent),
      d(new Private)
{
}

ConsumerAbstractAdaptor::~ConsumerAbstractAdaptor()
{
    delete d;
}

void ConsumerAbstractAdaptor::waveFunction(const Wave &wave)
{
    QList<QByteArray> noRootTokens = wave.target().mid(1).split('/');

    int dIndex = dispatchIndex(noRootTokens);
    DispatchResult result = dispatch(dIndex, wave.payload(), noRootTokens);

    ResponseCode response;
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

    sendRebound(Rebound(wave.id(), response));
}

void ConsumerAbstractAdaptor::insertTransition(int state, QByteArray token, int newState)
{
    d->transitions.insert(Private::StatePair(state, token), newState);
}

void ConsumerAbstractAdaptor::insertDispatchState(int state, int dispatchIndex)
{
    d->acceptingStates.insert(state, dispatchIndex);
}

int ConsumerAbstractAdaptor::dispatchIndex(const QList<QByteArray> &inputTokens)
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

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, QByteArray *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = QByteArray();
        return false;
    }

    *value = doc.byteArrayValue("v");
    return true;
}

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, int *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = 0;
        return false;
    }

    *value = doc.int32Value("v");
    return true;
}

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, qint64 *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = 0;
        return false;
    }

    *value = doc.int64Value("v");
    return true;
}

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, bool *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = false;
        return false;
    }

    *value = doc.booleanValue("v");
    return true;
}

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, double *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = 0.0;
        return false;
    }

    *value = doc.doubleValue("v");
    return true;
}

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, QString *value)
{
    Util::BSONDocument doc(payload);
    if (Q_UNLIKELY(!doc.isValid() || !doc.contains("v"))) {
        *value = QString();
        return false;
    }

    *value = doc.stringValue("v");
    return true;
}

bool ConsumerAbstractAdaptor::payloadToValue(const QByteArray &payload, QDateTime *value)
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
