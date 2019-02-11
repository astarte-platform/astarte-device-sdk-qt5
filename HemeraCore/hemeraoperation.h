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

#ifndef HEMERA_OPERATION_H
#define HEMERA_OPERATION_H

#include <HemeraCore/Global>

#include <QtCore/QObject>

namespace Hemera {

class OperationPrivate;
/**
 * @class Operation
 * @ingroup HemeraCore
 * @headerfile Hemera/hemeraoperation.h <HemeraCore/Operation>
 *
 * @brief A pending asynchronous operation
 *
 * This class tracks a pending asynchronous operation. Hemera is designed
 * to work on fully asynchronous models or over IPC calls; Operation allows
 * to track asynchronous operations (such as a pending IPC call or a delayed
 * initialization) through a nice job-based model.
 *
 * Operations, once created, are started immediately. Though, it is safe
 * to perform connections and setups right after an Operation's creationm as
 * the starting procedure will be triggered as soon as the event loop returns.
 *
 * Various subclasses specialize in specific return types. When @ref finished
 * is emitted, the Operation can be evaluated and the result can be extracted.
 * When the operation finishes, it is automatically garbage collected, and after
 * the result recollection the object should be considered dead. Likewise, Operation
 * must not be deleted explicitly by the user.
 */
class HEMERA_QT5_SDK_EXPORT Operation : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Operation)
    Q_DECLARE_PRIVATE(Operation)

    Q_PRIVATE_SLOT(d_func(), void emitFinished())
    Q_PRIVATE_SLOT(d_func(), void setStarted())

    Q_PROPERTY(bool started READ isStarted NOTIFY started)
    Q_PROPERTY(bool finished READ isFinished NOTIFY finished)
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(QString errorName READ errorName)
    Q_PROPERTY(QString errorMessage READ errorMessage)

public:
    /**
     * @brief Defines the execution options for the operation.
     */
    enum ExecutionOption {
        /** Operation will have no additional options when executed */
        NoOptions = 1 << 0,
        /** Operation will need to be started explicitly. */
        ExplicitStartOption = 1 << 1
    };
    Q_DECLARE_FLAGS(ExecutionOptions, ExecutionOption)
    Q_FLAGS(ExecutionOptions)
    Q_ENUM(ExecutionOption)

    /**
     * Default destructor
     */
    virtual ~Operation();

    /**
     * Returns whether this operation is running.
     *
     * @returns @p true if the Operation is running, @p false if not.
     */
    bool isStarted() const;

    /**
     * Returns whether this operation has finished.
     *
     * @returns @p true if the Operation has finished, @p false if not.
     */
    bool isFinished() const;

    /**
     * @returns Whether this operation is valid or not.
     */
    bool isValid() const;

    /**
     * @returns Whether this operation has finished with an error or not.
     *
     * @note This function returns a meaningful result only when @ref isFinished is true.
     */
    bool isError() const;
    /**
     * @returns The error name of the occurred error, if any.
     *
     * @note If @ref isError returns false, this function will return an empty string.
     *
     * @note This function returns a meaningful result only when @ref isFinished is true.
     */
    QString errorName() const;
    /**
     * @returns The error message of the occurred error, if any.
     *
     * @note If @ref isError returns false, this function will return an empty string. Given that
     *       it is not compulsory to provide an error message for Operations, the returned string
     *       might as well be empty even in case the Operation failed.
     *
     * @note This function returns a meaningful result only when @ref isFinished is true.
     */
    QString errorMessage() const;

    /**
     * @brief Synchronously awaits for the Operation's completion.
     *
     * This method allows to process an Operation synchronously, awaiting for its
     * completion and returning when the Operation is finished. It returns whether the
     * operation finished successfully or not, whereas further information can be
     * retrieved from the Operation object once this method returns.
     *
     * It is possible to specify a timeout after which synchronize will return regardless
     * of the Operation's completion.
     *
     * @p timeout Timeout, in seconds, after which the synchronization will fail. A negative
     *            value implies the synchronization will never time out.
     *
     * @returns Whether the operation succeeded or not.
     *
     * @note If the operation has already finished, this method returns immediately. It
     *       returns the result of the operation nonetheless.
     *
     * @note If synchronize times out, the Operation won't fail or will be stopped. If you set a
     *       timeout and synchronize returns false, you should be checking @ref isFinished to
     *       make sure the Operation has failed or timed out synchronizing.
     *
     * @note As much as synchronize is used extensively in bindings and is safe to use,
     *       it is not part of the advised paradigm of development in Hemera Qt5, which
     *       strives to be as asynchronous as possible. If you are using Qt5 SDK directly,
     *       connecting to @ref finished is usually the advised choice.
     */
    bool synchronize(int timeout = -1);

    /**
     * @returns The Operation's options.
     */
    ExecutionOptions executionOptions() const;


public Q_SLOTS:
    /**
     * Starts an asynchronous operation which has not been started yet.
     */
    bool start();

Q_SIGNALS:
    /**
     * @brief Notifies the start of the asynchronous operation.
     *
     * Emitted when the operation is started.
     *
     * @param operation A pointer to the started operation.
     */
    void started(Hemera::Operation *operation);

    /**
     * @brief Notifies the completion of the asynchronous operation.
     *
     * Emitted when the operation has finished. Connect to this signal to inspect the Operation result upon
     * its completion.
     *
     * @param operation A pointer to the finished operation.
     */
    void finished(Hemera::Operation *operation);

protected:
    explicit Operation(QObject *parent = nullptr);
    explicit Operation(ExecutionOptions options, QObject *parent = nullptr);
    explicit Operation(OperationPrivate &dd, ExecutionOptions options, QObject *parent = nullptr);

protected Q_SLOTS:
    /**
     * Implements the operation main logic
     *
     * When implementing an Operation, this method should hold your main logic.
     * It is called by Operation when needed, and the developer should just reimplement it without invoking it.
     *
     * Once the Operation is completed, either setFinished or setFinishedWithError must be called.
     */
    virtual void startImpl() = 0;

    /**
     * Completes the processing of the Operation
     *
     * Once setFinished is called, the Operation will be declared successfully completed.
     *
     * @note After calling this function, the object will be garbage collected, and no further processing
     *       must be done.
     */
    void setFinished();
    /**
     * Completes the processing of the Operation with a failure
     *
     * Once setFinishedWithError is called, the Operation will be declared failed.
     *
     * @note After calling this function, the object will be garbage collected, and no further processing
     *       must be done.
     */
    void setFinishedWithError(const QString &name, const QString &message);

protected:
    const QScopedPointer< OperationPrivate > d_ptr;
};

}

#endif // HEMERA_OPERATION_H
