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
