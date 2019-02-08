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
