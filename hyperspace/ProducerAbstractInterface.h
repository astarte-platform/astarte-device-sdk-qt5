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

#ifndef _HYPERSPACE_PLUS_PROVIDERABSTRACTINTERFACE_H_
#define _HYPERSPACE_PLUS_PROVIDERABSTRACTINTERFACE_H_

#include <HyperspaceCore/AbstractWaveTarget>

#include <QtCore/QDateTime>

namespace Hyperdrive {
class AstarteTransport;
}

namespace Hyperspace
{

enum class Retention {
    Unknown = 0,
    Discard = 1,
    Volatile = 2,
    Stored = 3
};

enum class Reliability {
    Unknown = 0,
    Unreliable = 1,
    Guaranteed = 2,
    Unique = 3
};

namespace ProducerConsumer
{

class ProducerAbstractInterface : public AbstractWaveTarget
{
    Q_OBJECT

    public:
        enum DispatchResult {
            Success,
            CouldNotConvertPayload,
            IndexNotFound
        };

        ProducerAbstractInterface(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, QObject *parent);
        virtual ~ProducerAbstractInterface();

    protected:
        virtual void waveFunction(const Wave &wave) override;

        void insertTransition(int state, QByteArray token, int newState);
        void insertDispatchState(int state, int dispatchIndex);

        int dispatchIndex(const QList<QByteArray> &inputTokens);

        virtual void populateTokensAndStates() = 0;
        virtual Hyperspace::ProducerConsumer::ProducerAbstractInterface::DispatchResult dispatch(int i, const QByteArray &value, const QList<QByteArray> &inputTokens) = 0;

        void sendRawDataOnEndpoint(const QByteArray &value, const QByteArray &target, const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>());

        void sendDataOnEndpoint(const QByteArray &value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
        void sendDataOnEndpoint(double value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
        void sendDataOnEndpoint(int value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
        void sendDataOnEndpoint(qint64 value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
        void sendDataOnEndpoint(bool value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
        void sendDataOnEndpoint(const QString &value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());
        void sendDataOnEndpoint(const QDateTime &value, const QByteArray &target,
            const QHash<QByteArray, QByteArray> &attributes = QHash<QByteArray, QByteArray>(), const QDateTime &timestamp = QDateTime(), const QVariantHash &metadata = QVariantHash());

        bool payloadToValue(const QByteArray &payload, QByteArray *value);
        bool payloadToValue(const QByteArray &payload, int *value);
        bool payloadToValue(const QByteArray &payload, qint64 *value);
        bool payloadToValue(const QByteArray &payload, bool *value);
        bool payloadToValue(const QByteArray &payload, double *value);
        bool payloadToValue(const QByteArray &payload, QString *value);
        bool payloadToValue(const QByteArray &payload, QDateTime *value);

    private:
        class Private;
        Private *const d;
};

}

}

#endif
