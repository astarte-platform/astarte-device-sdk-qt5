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

#ifndef HEMERA_HEMERAFINGERPRINTS_H
#define HEMERA_HEMERAFINGERPRINTS_H

#include <QtCore/QObject>
#include <HemeraCore/Global>

namespace Hemera {

class ByteArrayOperation;

class HEMERA_QT5_SDK_EXPORT Fingerprints : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Fingerprints)

public:
    static ByteArrayOperation *globalHardwareId();

private:
    Fingerprints();
    virtual ~Fingerprints();
    class Private;
    Private * const d;
};

}

#endif // HEMERA_HEMERAFINGERPRINTS_H
