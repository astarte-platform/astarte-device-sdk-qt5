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

#ifndef HYPERDRIVE_CACHE_MESSAGE_H
#define HYPERDRIVE_CACHE_MESSAGE_H

#include "hyperdriveinterface.h"

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QDataStream>

namespace Hyperdrive {

class CacheMessageData;

class CacheMessage {
public:
    CacheMessage();
    CacheMessage(const CacheMessage &other);
    ~CacheMessage();

    CacheMessage &operator=(const CacheMessage &rhs);
    bool operator==(const CacheMessage &other) const;
    inline bool operator!=(const CacheMessage &other) const { return !operator==(other); }

    QByteArray target() const;
    void setTarget(const QByteArray &t);

    Hyperdrive::Interface::Type interfaceType() const;
    void setInterfaceType(Hyperdrive::Interface::Type interfaceType);

    QByteArray payload() const;
    void setPayload(const QByteArray &p);

    QHash<QByteArray, QByteArray> attributes() const;
    QByteArray attribute(const QByteArray &attribute) const;
    bool hasAttribute(const QByteArray &attribute) const;
    void setAttributes(const QHash<QByteArray, QByteArray> &a);
    void addAttribute(const QByteArray &attribute, const QByteArray &value);
    bool removeAttribute(const QByteArray &attribute);
    QByteArray takeAttribute(const QByteArray &attribute);

    QByteArray serialize() const;
    static CacheMessage fromBinary(const QByteArray &data);

private:
    QSharedDataPointer<CacheMessageData> d;
};

}

#endif // HYPERDRIVE_CACHE_MESSAGE_H
