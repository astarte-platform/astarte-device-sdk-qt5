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

#include "hemerafakehardwareidoperation.h"

namespace Hemera {

QByteArray FakeHardwareIDOperation::s_fakeHardwareID = QByteArray();

FakeHardwareIDOperation::FakeHardwareIDOperation(QObject *parent)
    : ByteArrayOperation(parent)
{
}

FakeHardwareIDOperation::~FakeHardwareIDOperation()
{
}

QByteArray FakeHardwareIDOperation::result() const
{
    return s_fakeHardwareID;
}

void FakeHardwareIDOperation::setHardwareId(const QByteArray &hardwareId)
{
    s_fakeHardwareID = hardwareId;
}

void FakeHardwareIDOperation::startImpl()
{
    setFinished();
}

}
