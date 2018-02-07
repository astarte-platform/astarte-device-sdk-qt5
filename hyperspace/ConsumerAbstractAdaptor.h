#ifndef _HYPERSPACEBASEADAPTOR_H_
#define _HYPERSPACEBASEADAPTOR_H_

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include <HyperspaceCore/AbstractWaveTarget>

class QDateTime;

namespace Hyperdrive {
class AstarteTransport;
}

namespace Hyperspace
{

namespace ProducerConsumer
{

class ConsumerAbstractAdaptor : public AbstractWaveTarget
{
    Q_OBJECT

    public:
        enum DispatchResult {
            Success,
            IndexNotFound,
            CouldNotConvertPayload
        };

        explicit ConsumerAbstractAdaptor(const QByteArray &interface, Hyperdrive::AstarteTransport *astarteTransport, QObject *parent);
        virtual ~ConsumerAbstractAdaptor();

    protected:
        virtual void waveFunction(const Wave &wave) override;

        void insertTransition(int state, QByteArray token, int newState);
        void insertDispatchState(int state, int dispatchIndex);

        int dispatchIndex(const QList<QByteArray> &inputTokens);

        virtual void populateTokensAndStates() = 0;
        virtual Hyperspace::ProducerConsumer::ConsumerAbstractAdaptor::DispatchResult dispatch(int i, const QByteArray &value, const QList<QByteArray> &inputTokens) = 0;

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
