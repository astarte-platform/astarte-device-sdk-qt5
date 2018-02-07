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

#ifndef HEMERA_HEMERAASYNCINITOBJECT_P_H
#define HEMERA_HEMERAASYNCINITOBJECT_P_H

#include <HemeraCore/AsyncInitObject>
#include <HemeraCore/Operation>

#include <QtCore/QPointer>

namespace Hemera
{

class InitObjectOperation : public Operation
{
    Q_OBJECT

public:
    explicit InitObjectOperation(AsyncInitObject *parent);
    virtual ~InitObjectOperation();

protected:
    virtual void startImpl();

private:
    AsyncInitObject *m_object;

    friend class AsyncInitObject;
    friend class AsyncInitObjectPrivate;
};

class AsyncInitObjectPrivate
{
public:
    AsyncInitObjectPrivate(AsyncInitObject *q) : q_ptr(q), ready(false), parts(0), initOperation(nullptr) {}
    virtual ~AsyncInitObjectPrivate() {}

    Q_DECLARE_PUBLIC(AsyncInitObject)
    AsyncInitObject *q_ptr;

    QString errorName;
    QString errorMessage;
    bool ready;
    uint parts;

    QPointer<InitObjectOperation> initOperation;

    virtual void initHook();
    virtual void postInitHook();

private:
    // Q_PRIVATE_SLOT
    void finalizeInit();
};

}

#endif
