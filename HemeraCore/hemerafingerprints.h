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
