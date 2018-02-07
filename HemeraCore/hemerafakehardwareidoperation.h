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

#ifndef HEMERAFAKEHARWAREIDOPERATION_H
#define HEMERAFAKEHARWAREIDOPERATION_H

#include <HemeraCore/CommonOperations>

namespace Hemera {

class FakeHardwareIDOperation : public Hemera::ByteArrayOperation
{
    Q_OBJECT
    Q_DISABLE_COPY(FakeHardwareIDOperation)

public:
    explicit FakeHardwareIDOperation(QObject *parent = nullptr);
    virtual ~FakeHardwareIDOperation();

    QByteArray result() const Q_DECL_OVERRIDE Q_DECL_FINAL;

    static void setHardwareId(const QByteArray &hardwareId);

protected Q_SLOTS:
    virtual void startImpl() Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
    static QByteArray s_fakeHardwareID;
};

}

#endif
