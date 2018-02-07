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

#ifndef HEMERA_HEMERAASYNCINITOBJECT_H
#define HEMERA_HEMERAASYNCINITOBJECT_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <HemeraCore/Global>

namespace Hemera {

class Operation;

class AsyncInitObjectPrivate;
/**
 * @class AsyncInitObject
 * @ingroup HemeraCore
 * @headerfile Hemera/hemeraasyncinitobject.h <HemeraCore/AsyncInitObject>
 *
 * @brief An asynchronous object performing a late initialization
 *
 * @b AsyncInitObject is the backbone of Hemera Qt5 SDK. Its initialization paradigm recurs in
 * almost any class exported by the SDK, and allows for a truly asynchronous and transparent
 * workflow in your SDK usage.
 *
 * An @b AsyncInitObject has in most cases an empty constructor, and the whole creation logic is
 * handled during its initialization phase, represented as an @b Operation. The initialization procedure
 * can as well fail, and in that case the object will be unusable.
 *
 * Even though @b AsyncInitObject is heavily used inside Hemera, it is also meant to be exploited
 * in client applications needing specific asynchronous logic.
 *
 * @sa Operation
 */
class HEMERA_QT5_SDK_EXPORT AsyncInitObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncInitObject)
    Q_DECLARE_PRIVATE_D(d_h_ptr, AsyncInitObject)
    Q_PRIVATE_SLOT(d_func(), void initHook())
    Q_PRIVATE_SLOT(d_func(), void finalizeInit())

    Q_PROPERTY(bool ready READ isReady NOTIFY ready)
    Q_PROPERTY(QString initError READ initError)
    Q_PROPERTY(QString initErrorMessage READ initErrorMessage)

public:
    /**
     * Default destructor
     */
    virtual ~AsyncInitObject();

    /**
     * @returns Whether the object has been initialized or not
     *
     * @note If the object has been initialized and failed its init phase, this method will return false.
     */
    bool isReady() const;
    /**
     * @returns Whether the object has failed its initialization or not
     */
    bool hasInitError() const;

    /**
     * @returns In case the object failed to initialize, the error name. Otherwise, an empty string.
     */
    QString initError() const;
    /**
     * @returns In case the object failed to initialize, the error message. Otherwise, an empty string.
     */
    QString initErrorMessage() const;

public Q_SLOTS:
    /**
     * Initializes the object.
     *
     * This function returns a meaningful value only upon its first call. The returned @b Operation tracks
     * the progress of the init procedure, and notifies when the initialization has completed, and of
     * any possible errors occurred.
     *
     * @returns An operation representing the initialization procedure, or a null pointer in case the request was unsatisfiable.
     */
    Hemera::Operation *init();

protected:
    /**
     * Sets the number of parts of this object.
     *
     * When implementing an AsyncInitObject, one can choose whether to have a sequential procedure, or maybe a parallel
     * one. In case your object initializes throughout a sum of parallel procedures, AsyncInitObject can be used
     * with a partial completion mechanism.
     *
     * When setParts is called, setOnePartIsReady should be used instead of setReady. The object will be set ready as soon
     * as setOnePartIsReady is called as many times as the object's parts. If setFinishedWithError is called at any time before
     * completion, the initialization procedure will be considered failed, no matter how many parts have been completed.
     *
     * @p parts The number of parts this object is composed of
     *
     * @note Upon calling setParts, setReady won't be working anymore.
     */
    void setParts(uint parts);

protected Q_SLOTS:
    /**
     * Implements the object intialization
     *
     * When implementing an AsyncInitObject, this method should hold your initialization logic.
     * It is called by AsyncInitObject when needed, and the developer should just reimplement it without invoking it.
     *
     * Once the initialization procedure is completed, either setReady, setOnePartIsReady or setInitError must
     * be called.
     */
    virtual void initImpl() = 0;

    /**
     * Completes the initialization of the object
     *
     * Once setReady is called, the object's initialization will be declared successfully completed.
     *
     * @note If your object has parts, this method will do nothing
     */
    void setReady();
    /**
     * Triggers a fatal failure in the object's initialization.
     *
     * @note Once this function is called, the object is to be considered garbage.
     */
    void setInitError(const QString &errorName, const QString &message = QString());

    /**
     * Completes the initialization of a part of the object.
     *
     * This method tracks the reference counting of the object's parts. When called as many times as the
     * object's parts, its behavior is equivalent to setReady.
     *
     * @note If your object has no parts, this method will do nothing
     */
    void setOnePartIsReady();

Q_SIGNALS:
    /**
     * Emitted when the object has initialized successfully.
     *
     * @note Object state can be monitored either from this signal or from the init operation.
     */
    void ready();

protected:
    explicit AsyncInitObject(QObject *parent);
    explicit AsyncInitObject(AsyncInitObjectPrivate &dd, QObject *parent);

    AsyncInitObjectPrivate * const d_h_ptr;

    friend class InitObjectOperation;
};

}

#endif // HEMERA_HEMERAASYNCINITOBJECT_H
