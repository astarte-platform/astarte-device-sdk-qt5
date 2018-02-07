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
