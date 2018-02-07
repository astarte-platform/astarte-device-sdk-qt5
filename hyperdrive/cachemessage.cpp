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

#include "cachemessage.h"

#include <hyperdriveprotocol.h>

#include <QSharedData>

#include <QtCore/QLoggingCategory>

#include <HyperspaceCore/BSONDocument>
#include <HyperspaceCore/BSONSerializer>

Q_LOGGING_CATEGORY(hyperdriveCacheMessageDC, "hyperdrive.cachemessage", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperdrive {

class CacheMessageData : public QSharedData
{
public:
    CacheMessageData() { }
    CacheMessageData(const CacheMessageData &other)
        : QSharedData(other), target(other.target), interfaceType(other.interfaceType), payload(other.payload), attributes(other.attributes) { }
    ~CacheMessageData() { }

    QByteArray target;
    Hyperdrive::Interface::Type interfaceType;
    QByteArray payload;
    QHash<QByteArray, QByteArray> attributes;
};

CacheMessage::CacheMessage()
    : d(new CacheMessageData())
{
}

CacheMessage::CacheMessage(const CacheMessage& other)
    : d(other.d)
{
}

CacheMessage::~CacheMessage()
{
}

CacheMessage& CacheMessage::operator=(const CacheMessage& rhs)
{
    if (this==&rhs) {
        // Protect against self-assignment
        return *this;
    }

    d = rhs.d;
    return *this;
}

bool CacheMessage::operator==(const CacheMessage& other) const
{
    return d->target == other.target() && d->payload == other.payload() && d->interfaceType == other.interfaceType() && d->attributes == other.attributes();
}

QByteArray CacheMessage::payload() const
{
    return d->payload;
}

void CacheMessage::setPayload(const QByteArray& p)
{
    d->payload = p;
}

QByteArray CacheMessage::target() const
{
    return d->target;
}

void CacheMessage::setTarget(const QByteArray& t)
{
    d->target = t;
}

Hyperdrive::Interface::Type CacheMessage::interfaceType() const
{
    return d->interfaceType;
}

void CacheMessage::setInterfaceType(Hyperdrive::Interface::Type interfaceType)
{
    d->interfaceType = interfaceType;
}

QHash<QByteArray, QByteArray> CacheMessage::attributes() const
{
    return d->attributes;
}

QByteArray CacheMessage::attribute(const QByteArray &attribute) const
{
    return d->attributes.value(attribute);
}

bool CacheMessage::hasAttribute(const QByteArray &attribute) const
{
    return d->attributes.contains(attribute);
}

void CacheMessage::setAttributes(const QHash<QByteArray, QByteArray>& attributes)
{
    d->attributes = attributes;
}

void CacheMessage::addAttribute(const QByteArray& attribute, const QByteArray& value)
{
    d->attributes.insert(attribute, value);
}

bool CacheMessage::removeAttribute(const QByteArray& attribute)
{
    return d->attributes.remove(attribute);
}

QByteArray CacheMessage::takeAttribute(const QByteArray& attribute)
{
    return d->attributes.take(attribute);
}

QByteArray CacheMessage::serialize() const
{
    Hyperspace::Util::BSONSerializer s;
    s.appendInt32Value("y", static_cast<int32_t>(Hyperdrive::Protocol::MessageType::CacheMessage));
    s.appendASCIIString("t", d->target);
    s.appendBinaryValue("p", d->payload);
    s.appendInt32Value("i", static_cast<int32_t>(d->interfaceType));

    if (!d->attributes.isEmpty()) {
        Hyperspace::Util::BSONSerializer sa;
        for (QHash<QByteArray, QByteArray>::const_iterator i = d->attributes.constBegin(); i != d->attributes.constEnd(); ++i) {
            sa.appendASCIIString(i.key(), i.value());
        }
        sa.appendEndOfDocument();
        s.appendDocument("a", sa.document());
    }

    s.appendEndOfDocument();

    return s.document();
}

CacheMessage CacheMessage::fromBinary(const QByteArray &data)
{
    Hyperspace::Util::BSONDocument doc(data);
    if (Q_UNLIKELY(!doc.isValid())) {
        qCWarning(hyperdriveCacheMessageDC) << "CacheMessage BSON document is not valid!";
        return CacheMessage();
    }
    if (Q_UNLIKELY(doc.int32Value("y") != static_cast<int32_t>(Hyperdrive::Protocol::MessageType::CacheMessage))) {
        qCWarning(hyperdriveCacheMessageDC) << "Received message is not a CacheMessage";
        return CacheMessage();
    }

    CacheMessage c;
    c.setTarget(doc.byteArrayValue("t"));
    c.setPayload(doc.byteArrayValue("p"));
    c.setInterfaceType(static_cast<Interface::Type>(doc.int32Value("i")));

    if (doc.contains("a")) {
        Hyperspace::Util::BSONDocument attributesDoc = doc.subdocument("a");
        if (!attributesDoc.isValid()) {
            qDebug() << "CacheMessage attributes are not valid\n";
            return CacheMessage();
        }
        c.setAttributes(attributesDoc.byteArrayValuesHash());
    }

    return c;
}

}
