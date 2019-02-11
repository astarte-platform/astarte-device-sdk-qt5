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

#include "hemeraasyncinitobject_p.h"

#include <HemeraCore/Literals>

#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

Q_LOGGING_CATEGORY(LOG_HEMERA_ASYNCINITOBJECT, "Hemera::AsyncInitObject")

namespace Hemera
{

InitObjectOperation::InitObjectOperation(AsyncInitObject *parent)
    : Operation(parent)
    , m_object(parent)
{
    m_object->d_h_ptr->initOperation = this;
}

InitObjectOperation::~InitObjectOperation()
{
}

void InitObjectOperation::startImpl()
{
    QTimer::singleShot(0, m_object, SLOT(initHook()));
}

void AsyncInitObjectPrivate::initHook()
{
    Q_Q(AsyncInitObject);

    QTimer::singleShot(0, q, SLOT(initImpl()));
}

void AsyncInitObjectPrivate::postInitHook()
{
    Q_Q(AsyncInitObject);

    QTimer::singleShot(0, q, SLOT(finalizeInit()));
}

void AsyncInitObjectPrivate::finalizeInit()
{
    Q_Q(AsyncInitObject);

    initOperation.data()->setFinished();
    Q_EMIT q->ready();
}

AsyncInitObject::AsyncInitObject(QObject* parent)
    : QObject(parent)
    , d_h_ptr(new AsyncInitObjectPrivate(this))
{
}

AsyncInitObject::AsyncInitObject(Hemera::AsyncInitObjectPrivate& dd, QObject* parent)
    : QObject(parent)
    , d_h_ptr(&dd)
{
}

AsyncInitObject::~AsyncInitObject()
{
    delete d_h_ptr;
}

Hemera::Operation *AsyncInitObject::init()
{
    if (isReady()) {
        qCWarning(LOG_HEMERA_ASYNCINITOBJECT) << tr("Trying to initialize, but the object is already ready! Doing nothing.");
        return nullptr;
    }

    Q_D(AsyncInitObject);

    if (d->initOperation.isNull()) {
        return new InitObjectOperation(this);
    } else {
        return d->initOperation.data();
    }
}

bool AsyncInitObject::hasInitError() const
{
    Q_D(const AsyncInitObject);

    return !d->errorName.isEmpty();
}

QString AsyncInitObject::initError() const
{
    Q_D(const AsyncInitObject);

    return d->errorName;
}

QString AsyncInitObject::initErrorMessage() const
{
    Q_D(const AsyncInitObject);

    return d->errorMessage;
}

bool AsyncInitObject::isReady() const
{
    Q_D(const AsyncInitObject);

    return d->ready;
}

void AsyncInitObject::setReady()
{
    Q_D(AsyncInitObject);

    d->ready = true;
    d->postInitHook();
}

void AsyncInitObject::setParts(uint parts)
{
    Q_D(AsyncInitObject);

    d->parts = parts;
}

void AsyncInitObject::setOnePartIsReady()
{
    Q_D(AsyncInitObject);

    // We might as well have failed already.
    if (hasInitError()) {
        return;
    }

    if (d->parts == 0) {
        setInitError(Literals::literal(Literals::Errors::badRequest()),
                     tr("Called setOnePartIsReady on an object without parts, or called when all parts are already ready."));
        return;
    }

    --d->parts;

    if (d->parts == 0) {
        // Yay!
        setReady();
    }
}

void AsyncInitObject::setInitError(const QString &errorName, const QString &message)
{
    Q_D(AsyncInitObject);

    d->errorName = errorName;
    d->errorMessage = message;
    d->initOperation.data()->setFinishedWithError(errorName, message);
}

}

#include "moc_hemeraasyncinitobject.cpp"
