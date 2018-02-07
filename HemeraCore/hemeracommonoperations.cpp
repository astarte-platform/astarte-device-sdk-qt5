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

#include "hemeracommonoperations.h"
#include <HemeraCore/Literals>

#include <QtCore/QJsonDocument>
#include <QtCore/QUrl>
#include <QtCore/QLoggingCategory>

#include <functional>

Q_LOGGING_CATEGORY(LOG_SEQUENTIALOPERATION, "Hemera::SequentialOperation")

namespace Hemera {

class SequentialOperation::Private
{
public:
    QList<Hemera::Operation*> sequence;
    QList<Hemera::Operation*> revertSequence;
    QList<Hemera::Operation*>::const_iterator opIterator;
    QString failReasonErrorName;
    QString failReasonErrorMessage;
    SequenceOptions options;
    SequentialOperation *q;
    bool runningRevertSequence : 1;
    bool hasCleanupSucceeded : 1;

    Private(SequentialOperation *parent)
        : q(parent), runningRevertSequence(false), hasCleanupSucceeded(false)
    {
    }

    void finishedOp(Operation *op)
    {
        if (op->isError()) {
            Q_EMIT q->stepFailed(op);
            if (!runningRevertSequence && !(options & ContinueOnFailure) && !revertSequence.isEmpty()) {
                opIterator = revertSequence.constBegin();
                runningRevertSequence = true;
                failReasonErrorName = op->errorName();
                failReasonErrorMessage = op->errorMessage();

                Q_EMIT q->cleanupStarted();
                return;
            } else if (runningRevertSequence || !(options & ContinueOnFailure) ||  revertSequence.isEmpty()) {
                if (runningRevertSequence) {
                    Q_EMIT q->cleanupFailed(op);
                }

                failReasonErrorName = op->errorName();
                failReasonErrorMessage = op->errorMessage();
                q->setFinishedWithError(failReasonErrorName, failReasonErrorMessage);
                return;
            } else {
                Q_EMIT q->stepFinished(op);
                opIterator++;
            }
        } else {
            opIterator++;
        }

        if (opIterator == sequence.constEnd()) {
            q->setFinished();
            return;
        }
        if (opIterator == revertSequence.constEnd()) {
            hasCleanupSucceeded = true;
            Q_EMIT q->cleanupSucceeded();
            q->setFinishedWithError(failReasonErrorName, failReasonErrorMessage);
            return;
        }

        QObject::connect(*opIterator, SIGNAL(finished(Hemera::Operation *)), q, SLOT(finishedOp(Hemera::Operation *)));
        (*opIterator)->start();
    }
};

SequentialOperation::SequentialOperation(const QList<Hemera::Operation*> &sequence, QObject* parent)
    : SequentialOperation(sequence, QList<Hemera::Operation *>(), NoSequenceOptions, NoOptions, parent)
{
}

SequentialOperation::SequentialOperation(const QList<Hemera::Operation*> &sequence, const QList<Hemera::Operation*> &revertSequence, QObject *parent)
    : SequentialOperation(sequence, revertSequence, NoSequenceOptions, NoOptions, parent)
{
}

SequentialOperation::SequentialOperation(const QList<Hemera::Operation*> &sequence, const QList<Hemera::Operation*> &revertSequence,
                                         SequenceOptions options, ExecutionOptions execOptions, QObject *parent)
    : Operation(execOptions, parent)
    , d(new Private(this))
{
    d->sequence = sequence;
    d->revertSequence = revertSequence;
    d->options = options;
    for (const Hemera::Operation *operation : sequence) {
        if (!(operation->executionOptions() & Operation::ExplicitStartOption)) {
            qCWarning(LOG_SEQUENTIALOPERATION) << tr("Executing an Operation without ExplicitStartOption with SequentialOperation can lead to non-deterministic behavior");
        }
    }
    d->opIterator = d->sequence.constBegin();
}

SequentialOperation::~SequentialOperation()
{
    delete d;
}

void SequentialOperation::startImpl()
{
    connect((*d->opIterator), SIGNAL(finished(Hemera::Operation *)), this, SLOT(finishedOp(Hemera::Operation *)));
    (*d->opIterator)->start();
}

bool SequentialOperation::isRunningCleanup() const
{
    return d->runningRevertSequence;
}

bool SequentialOperation::hasCleanupSucceeded() const
{
    return d->hasCleanupSucceeded;
}

/////////////////////


class CompositeOperation::Private
{
public:
    QList<Hemera::Operation*> operations;
    CompositeOptions options;
    uint left;
};

CompositeOperation::CompositeOperation(const QList<Hemera::Operation*> &operations, QObject* parent)
    : CompositeOperation(operations, NoCompositeOptions, NoOptions, parent)
{
}

CompositeOperation::CompositeOperation(const QList<Hemera::Operation*> &operations, CompositeOptions options,
                                       ExecutionOptions execOptions, QObject *parent)
    : Operation(execOptions, parent)
    , d(new Private)
{
    d->options = options;
    d->operations = operations;
    d->left = operations.count();
}

CompositeOperation::~CompositeOperation()
{
    delete d;
}

void CompositeOperation::startImpl()
{
    if (d->left == 0) {
        // Nothing to do
        setFinished();
        return;
    }

    auto process = [this] (Hemera::Operation *operation) {
        if (isError() || isFinished()) {
            return;
        }

        if (operation->isError() && !(d->options & DontAbortOnFailure)) {
            setFinishedWithError(operation->errorName(), operation->errorMessage());
        } else {
            --d->left;
            if (d->left == 0) {
                // Done!
                setFinished();
            }
        }
    };

    for (Hemera::Operation *operation : d->operations) {
        if (operation->isFinished()) {
            process(operation);
        } else {
            connect(operation, &Hemera::Operation::finished, process);
        }
    }
}

/////////////////////

class FailureOperation::Private
{
public:
    QString name;
    QString message;
};

FailureOperation::FailureOperation(const QString& errorName, const QString& errorMessage, QObject* parent)
    : Operation(parent)
    , d(new Private)
{
    d->name = errorName;
    d->message = errorMessage;
}

FailureOperation::~FailureOperation()
{
}

void FailureOperation::startImpl()
{
    setFinishedWithError(d->name, d->message);
}

/////////////////////

SuccessOperation::SuccessOperation(QObject* parent)
    : Operation(parent)
    , d(nullptr)
{
}

SuccessOperation::~SuccessOperation()
{
}

void SuccessOperation::startImpl()
{
    setFinished();
}

/////////////////////

ObjectOperation::ObjectOperation(QObject *parent)
    : Operation(parent)
    , d(nullptr)
{
}

ObjectOperation::~ObjectOperation()
{
}

/////////////////////

VariantOperation::VariantOperation(QObject *parent)
    : Operation(parent)
    , d(nullptr)
{
}

VariantOperation::~VariantOperation()
{
}


/////////////////////

VariantMapOperation::VariantMapOperation(QObject *parent)
    : Operation(parent)
    , d(nullptr)
{
}

VariantMapOperation::~VariantMapOperation()
{
}

/////////////////////

UIntOperation::UIntOperation(QObject *parent)
    : Operation(parent)
    , d(nullptr)
{
}

UIntOperation::~UIntOperation()
{
}

/////////////////////

BoolOperation::BoolOperation(QObject *parent)
    : Operation(parent)
    , d(nullptr)
{
}

BoolOperation::~BoolOperation()
{
}

/////////////////////


ByteArrayOperation::ByteArrayOperation(QObject *parent)
    : Operation(parent),
      d(nullptr)
{
}

ByteArrayOperation::~ByteArrayOperation()
{
}

/////////////////////


SSLCertificateOperation::SSLCertificateOperation(QObject *parent)
    : Operation(parent)
    , d(nullptr)
{
}

SSLCertificateOperation::~SSLCertificateOperation()
{
}

/////////////////////

StringOperation::StringOperation(QObject* parent)
    : Operation(parent)
    , d(nullptr)
{
}

StringOperation::~StringOperation()
{
}

/////////////////////

StringListOperation::StringListOperation(QObject* parent)
    : Operation(parent)
    , d(nullptr)
{
}

StringListOperation::~StringListOperation()
{
}

/////////////////////

JsonOperation::JsonOperation(QObject* parent)
    : Operation(parent)
    , d(nullptr)
{
}

JsonOperation::~JsonOperation()
{
}

/////////////////////

class UrlOperation::Private
{
public:
    QUrl result;
};

UrlOperation::UrlOperation(QObject* parent)
    : Operation(parent)
    , d(nullptr)
{
}

UrlOperation::~UrlOperation()
{
    delete d;
}

}

#include "moc_hemeracommonoperations.cpp"
