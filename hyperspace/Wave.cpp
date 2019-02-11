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

#include "Wave.h"

#include "BSONDocument.h"
#include "BSONSerializer.h"

#include <QtCore/QDebug>
#include <QtCore/QSharedData>

#include <time.h>

static bool random_initialized = false;
static quint64 internal_qualifier;
static quint32 internal_id_sequential = 0;

namespace Hyperspace {

class WaveData : public QSharedData
{
public:
    WaveData() { }
    WaveData(quint64 id, const QByteArray &method, const QByteArray &target, const ByteArrayHash &attributes, const QByteArray &payload)
        : id(id), method(method), target(target), attributes(attributes), payload(payload) { }
    WaveData(const WaveData &other)
        : QSharedData(other), id(other.id), method(other.method), interface(other.interface), target(other.target), attributes(other.attributes), payload(other.payload) { }
    ~WaveData() { }

    quint64 id;
    QByteArray method;
    QByteArray interface;
    QByteArray target;
    ByteArrayHash attributes;
    QByteArray payload;
};

Wave::Wave()
    : Wave((internal_qualifier << 32) | internal_id_sequential)
{
}

Wave::Wave(quint64 id)
    : d(new WaveData())
{
    d->id = id;
    if (Q_UNLIKELY(!random_initialized)) {
        qsrand(time(NULL));
        random_initialized = true;
        internal_qualifier = qrand();
        d->id = (internal_qualifier << 32) | internal_id_sequential;
    }
    ++internal_id_sequential;
}

Wave::Wave(const Wave& other)
    : d(other.d)
{
}

Wave::~Wave()
{
}

Wave& Wave::operator=(const Wave& rhs)
{
    if (this==&rhs) {
        // Protect against self-assignment
        return *this;
    }

    d = rhs.d;
    return *this;
}

bool Wave::operator==(const Wave& other) const
{
    return d->id == other.id();
}

ByteArrayHash Wave::attributes() const
{
    return d->attributes;
}

void Wave::setAttributes(const ByteArrayHash& attributes)
{
    d->attributes = attributes;
}

void Wave::addAttribute(const QByteArray& attribute, const QByteArray& value)
{
    d->attributes.insert(attribute, value);
}

bool Wave::removeAttribute(const QByteArray& attribute)
{
    return d->attributes.remove(attribute);
}

QByteArray Wave::takeAttribute(const QByteArray& attribute)
{
    return d->attributes.take(attribute);
}

quint64 Wave::id() const
{
    return d->id;
}

void Wave::setId(quint64 id)
{
    d->id = id;
}

QByteArray Wave::method() const
{
    return d->method;
}

void Wave::setMethod(const QByteArray& m)
{
    d->method = m;
}

QByteArray Wave::payload() const
{
    return d->payload;
}

void Wave::setPayload(const QByteArray& p)
{
    d->payload = p;
}

QByteArray Wave::interface() const
{
    return d->interface;
}

void Wave::setInterface(const QByteArray& i)
{
    d->interface = i;
}

QByteArray Wave::target() const
{
    return d->target;
}

void Wave::setTarget(const QByteArray& t)
{
    d->target = t;
}

QByteArray Wave::serialize() const
{
    Util::BSONSerializer s;
    s.appendInt32Value("y", (int32_t) Protocol::MessageType::Wave);
    s.appendInt64Value("u", (int64_t) d->id);
    s.appendASCIIString("m", d->method);
    s.appendASCIIString("i", d->interface);
    s.appendASCIIString("t", d->target);
    if (!d->attributes.isEmpty()) {
        Util::BSONSerializer sa;
        for (ByteArrayHash::const_iterator i = d->attributes.constBegin(); i != d->attributes.constEnd(); ++i) {
            sa.appendASCIIString(i.key(), i.value());
        }
        sa.appendEndOfDocument();
        s.appendDocument("a", sa.document());
    }
    s.appendBinaryValue("p", d->payload);
    s.appendEndOfDocument();

    return s.document();
}

Wave Wave::fromBinary(const QByteArray &data)
{
    Util::BSONDocument doc(data);
    if (Q_UNLIKELY(!doc.isValid())) {
        qWarning() << "Wave BSON document is not valid!";
        return Wave();
    }
    if (Q_UNLIKELY(doc.int32Value("y") != (int32_t) Protocol::MessageType::Wave)) {
        qWarning() << "Received message is not a Wave";
        return Wave();
   }

    Wave w;
    w.setId(doc.int64Value("u"));
    w.setMethod(doc.byteArrayValue("m"));
    w.setInterface(doc.byteArrayValue("i"));
    w.setTarget(doc.byteArrayValue("t"));
    w.setPayload(doc.byteArrayValue("p"));

    if (doc.contains("a")) {
        Util::BSONDocument attributesDoc = doc.subdocument("a");
        if (!attributesDoc.isValid()) {
            qDebug() << "Wave attributes are not valid\n";
            return Wave();
        }
        w.setAttributes(attributesDoc.byteArrayValuesHash());
    }

    return w;
}

}
