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

#include "hemeraoperation_p.h"

#include <QtCore/QEventLoop>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

#include <HemeraCore/Literals>

Q_LOGGING_CATEGORY(LOG_HEMERA_OPERATION, "Hemera::Operation")

namespace Hemera {

void OperationPrivate::emitFinished()
{
    Q_ASSERT(finished);
    Q_Q(Operation);
    Q_EMIT q->finished(q);
    // Delay deletion of 2 seconds to be safe
    QTimer::singleShot(2000, q, SLOT(deleteLater()));
}

void OperationPrivate::setStarted()
{
    Q_Q(Operation);
    started = true;
    Q_EMIT q->started(q);
}

Operation::Operation(QObject* parent)
   : Operation(NoOptions, parent)
{
}

Operation::Operation(ExecutionOptions options, QObject *parent)
    : Operation(*new OperationPrivate(this), options, parent)
{
}

Operation::Operation(OperationPrivate &dd, ExecutionOptions options, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(Operation);
    d->options = options;
    if (!(options & ExplicitStartOption)) {
        if (!(d->privateOptions & OperationPrivate::PrivateExecutionOption::SetStartedExplicitly)) {
            QTimer::singleShot(0, this, SLOT(setStarted()));
        }
        QTimer::singleShot(0, this, SLOT(startImpl()));
    }
}

Operation::~Operation()
{
    if (!isFinished()) {
        qCWarning(LOG_HEMERA_OPERATION) << tr("The operation is being deleted, but is not finished yet.");
    }
}

bool Operation::start()
{
    Q_D(Operation);

    if (!(d->started) && (d->options & ExplicitStartOption)) {
        if (!(d->privateOptions & OperationPrivate::PrivateExecutionOption::SetStartedExplicitly)) {
            d->setStarted();
        }
        QTimer::singleShot(0, this, SLOT(startImpl()));
        return true;
    } else {
       return false;
    }
}

QString Operation::errorMessage() const
{
    Q_D(const Operation);
    return d->errorMessage;
}

QString Operation::errorName() const
{
    Q_D(const Operation);
    return d->errorName;
}

bool Operation::isError() const
{
    Q_D(const Operation);
    return (d->finished && !d->errorName.isEmpty());
}

bool Operation::isFinished() const
{
    Q_D(const Operation);
    return d->finished;
}

bool Operation::isValid() const
{
    Q_D(const Operation);
    return (d->finished && d->errorName.isEmpty());
}


bool Operation::isStarted() const
{
    Q_D(const Operation);
    return d->started;
}

void Operation::setFinished()
{
    Q_D(Operation);

    if (d->finished) {
        if (!d->errorName.isEmpty()) {
            qCWarning(LOG_HEMERA_OPERATION) << this << tr("Trying to finish with success, but already failed with %2 : %3")
                                                       .arg(d->errorName, d->errorMessage);
        } else {
            qCWarning(LOG_HEMERA_OPERATION) << this << tr("Trying to finish with success, but already succeeded");
        }
        return;
    }

    d->finished = true;
    Q_ASSERT(isValid());
    QTimer::singleShot(0, this, SLOT(emitFinished()));
}

void Operation::setFinishedWithError(const QString& name, const QString& message)
{
    Q_D(Operation);

    if (d->finished) {
        if (!d->errorName.isEmpty()) {
            qCWarning(LOG_HEMERA_OPERATION) << this << tr("Trying to fail with %1, but already failed with %2 : %3")
                                                        .arg(name, d->errorName, d->errorMessage);
        } else {
            qCWarning(LOG_HEMERA_OPERATION) << this << tr("Trying to fail with %1, but already succeeded").arg(name);
        }
        return;
    }

    if (name.isEmpty()) {
        qCWarning(LOG_HEMERA_OPERATION) << this << tr("should be given a non-empty error name");
        d->errorName = Hemera::Literals::literal(Hemera::Literals::Errors::errorHandlingError());
    } else {
        d->errorName = name;
    }

    d->errorMessage = message;
    d->finished = true;
    Q_ASSERT(isError());
    QTimer::singleShot(0, this, SLOT(emitFinished()));
}

bool Operation::synchronize(int timeout)
{
    if (isFinished()) {
        return !isError();
    }

    QEventLoop e;
    connect(this, &Hemera::Operation::finished, &e, &QEventLoop::quit);

    if (timeout > 0) {
        QTimer::singleShot(timeout * 1000, &e, SLOT(quit()));
    }

    start();
    e.exec();

    if (!isFinished()) {
        qCWarning(LOG_HEMERA_OPERATION) << this << tr("timed out while trying to synchronize.");
        return false;
    }

    return !isError();
}

Operation::ExecutionOptions Operation::executionOptions() const
{
    Q_D(const Operation);
    return d->options;
}


}

#include "moc_hemeraoperation.cpp"
