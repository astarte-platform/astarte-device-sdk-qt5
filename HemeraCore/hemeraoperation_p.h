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

#ifndef HEMERA_OPERATION_P_H
#define HEMERA_OPERATION_P_H

#include <HemeraCore/Operation>
#include <QtCore/QDebug>

namespace Hemera {

class OperationPrivate
{
public:
    enum class PrivateExecutionOption {
        NoOptions = 0,
        SetStartedExplicitly = 1
    };
    Q_DECLARE_FLAGS(PrivateExecutionOptions, PrivateExecutionOption)
    Q_FLAGS(PrivateExecutionOptions)

    OperationPrivate(Operation *q, PrivateExecutionOptions pO = PrivateExecutionOption::NoOptions) : q_ptr(q), privateOptions(pO), started(false), finished(false) {}

    Q_DECLARE_PUBLIC(Operation)
    Operation * const q_ptr;

    QString errorName;
    QString errorMessage;
    Operation::ExecutionOptions options;
    PrivateExecutionOptions privateOptions;
    bool started : 1;
    bool finished : 1;

    // Private slots
    void emitFinished();
    void setStarted();
};

}

#endif
