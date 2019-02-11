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

#include "Fluctuation.h"

#include "BSONDocument.h"
#include "BSONSerializer.h"

#include <QtCore/QDebug>
#include <QSharedData>

namespace Hyperspace {

class FluctuationData : public QSharedData
{
public:
    FluctuationData() { }
    FluctuationData(const FluctuationData &other)
        : QSharedData(other), interface(other.interface), target(other.target), payload(other.payload), attributes(other.attributes) { }
    ~FluctuationData() { }

    QByteArray interface;
    QByteArray target;
    QByteArray payload;
    QHash<QByteArray, QByteArray> attributes;
};

Fluctuation::Fluctuation()
    : d(new FluctuationData())
{
}

Fluctuation::Fluctuation(const Fluctuation& other)
    : d(other.d)
{
}

Fluctuation::~Fluctuation()
{

}

Fluctuation& Fluctuation::operator=(const Fluctuation& rhs)
{
    if (this==&rhs) {
        // Protect against self-assignment
        return *this;
    }

    d = rhs.d;
    return *this;
}

bool Fluctuation::operator==(const Fluctuation& other) const
{
    return (d->target == other.target()) && (d->payload == other.payload()) && (d->interface == other.interface()) && (d->attributes == other.attributes());
}

QByteArray Fluctuation::payload() const
{
    return d->payload;
}

void Fluctuation::setPayload(const QByteArray& p)
{
    d->payload = p;
}

QByteArray Fluctuation::interface() const
{
    return d->interface;
}

void Fluctuation::setInterface(const QByteArray& i)
{
    d->interface = i;
}

QByteArray Fluctuation::target() const
{
    return d->target;
}

void Fluctuation::setTarget(const QByteArray& t)
{
    d->target = t;
}

QHash<QByteArray, QByteArray> Fluctuation::attributes() const
{
    return d->attributes;
}

void Fluctuation::setAttributes(const QHash<QByteArray, QByteArray>& attributes)
{
    d->attributes = attributes;
}

void Fluctuation::addAttribute(const QByteArray& attribute, const QByteArray& value)
{
    d->attributes.insert(attribute, value);
}

bool Fluctuation::removeAttribute(const QByteArray& attribute)
{
    return d->attributes.remove(attribute);
}

QByteArray Fluctuation::takeAttribute(const QByteArray& attribute)
{
    return d->attributes.take(attribute);
}

QByteArray Fluctuation::serialize() const
{
    Util::BSONSerializer s;
    s.appendInt32Value("y", (int32_t) Protocol::MessageType::Fluctuation);
    s.appendASCIIString("i", d->interface);
    s.appendASCIIString("t", d->target);
    s.appendBinaryValue("p", d->payload);

    if (!d->attributes.isEmpty()) {
        Util::BSONSerializer sa;
        for (ByteArrayHash::const_iterator i = d->attributes.constBegin(); i != d->attributes.constEnd(); ++i) {
            sa.appendASCIIString(i.key(), i.value());
        }
        sa.appendEndOfDocument();
        s.appendDocument("a", sa.document());
    }

    s.appendEndOfDocument();

    return s.document();
}

Fluctuation Fluctuation::fromBinary(const QByteArray &data)
{
    Util::BSONDocument doc(data);
    if (Q_UNLIKELY(!doc.isValid())) {
        qWarning() << "Fluctuation BSON document is not valid!";
        return Fluctuation();
    }
    if (Q_UNLIKELY(doc.int32Value("y") != (int32_t) Protocol::MessageType::Fluctuation)) {
        qWarning() << "Received message is not a Fluctuation";
        return Fluctuation();
    }

    Fluctuation f;
    f.setInterface(doc.byteArrayValue("i"));
    f.setTarget(doc.byteArrayValue("t"));
    f.setPayload(doc.byteArrayValue("p"));

    if (doc.contains("a")) {
        Util::BSONDocument attributesDoc = doc.subdocument("a");
        if (!attributesDoc.isValid()) {
            qDebug() << "Fluctuation attributes are not valid\n";
            return Fluctuation();
        }
        f.setAttributes(attributesDoc.byteArrayValuesHash());
    }

    return f;
}

}
