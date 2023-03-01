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

#include "Rebound.h"

#include "BSONDocument.h"
#include "BSONSerializer.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QSharedData>

Q_LOGGING_CATEGORY(reboundDataDC, "hyperspace.rebounddata", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperspace {

class ReboundData : public QSharedData
{
public:
    ReboundData() { }
    ReboundData(quint64 id, ResponseCode response, const ByteArrayHash &attributes, const QByteArray &payload)
        : id(id), responseCode(response), attributes(attributes), payload(payload) { }
    ReboundData(const ReboundData &other)
        : QSharedData(other), id(other.id), responseCode(other.responseCode), attributes(other.attributes), payload(other.payload) { }
    ~ReboundData() { }

    quint64 id;
    ResponseCode responseCode;
    ByteArrayHash attributes;
    QByteArray payload;
};

Rebound::Rebound(const Wave& wave, ResponseCode code)
    : Rebound(wave.id(), code)
{
}

Rebound::Rebound(quint64 waveId, ResponseCode code)
    : d(new ReboundData)
{
    d->id = waveId;
    d->responseCode = code;
}

Rebound::Rebound(const Rebound &other)
    : d(other.d)
{

}

Rebound::~Rebound()
{

}

Rebound& Rebound::operator=(const Rebound& rhs)
{
    if (this==&rhs) {
        // Protect against self-assignment
        return *this;
    }

    d = rhs.d;
    return *this;
}

bool Rebound::operator==(const Rebound& other) const
{
    return d->id == other.id();
}

ByteArrayHash Rebound::attributes() const
{
    return d->attributes;
}

void Rebound::setAttributes(const ByteArrayHash& attributes)
{
    d->attributes = attributes;
}

void Rebound::addAttribute(const QByteArray& attribute, const QByteArray& value)
{
    d->attributes.insert(attribute, value);
}

bool Rebound::removeAttribute(const QByteArray& attribute)
{
    return d->attributes.remove(attribute);
}

quint64 Rebound::id() const
{
    return d->id;
}

void Rebound::setId(quint64 id)
{
    d->id = id;
}

QByteArray Rebound::payload() const
{
    return d->payload;
}

void Rebound::setPayload(const QByteArray& p)
{
    d->payload = p;
}

ResponseCode Rebound::response() const
{
    return d->responseCode;
}

void Rebound::setResponse(ResponseCode r)
{
    d->responseCode = r;
}

QByteArray Rebound::serialize() const
{
    Util::BSONSerializer s;
    s.appendInt32Value("y", (int32_t) Protocol::MessageType::Rebound);
    s.appendInt64Value("u", (int64_t) d->id);
    s.appendInt32Value("r", (int32_t) d->responseCode);
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

Rebound Rebound::fromBinary(const QByteArray &data)
{
    Util::BSONDocument doc(data);
    if (Q_UNLIKELY(!doc.isValid())) {
        qCWarning(reboundDataDC) << "Rebound BSON document is not valid!";
        return Rebound(0);
    }
    if (Q_UNLIKELY(doc.int32Value("y") != (int32_t) Protocol::MessageType::Rebound)) {
        qCWarning(reboundDataDC) << "Received message is not a Rebound";
        return Rebound(0);
    }

    Rebound r((quint64) doc.int64Value("u"));
    r.setResponse((Hyperspace::ResponseCode) doc.int32Value("r"));
    if (doc.contains("a")) {
        Util::BSONDocument attributesDoc = doc.subdocument("a");
        if (!attributesDoc.isValid()) {
            qCDebug(reboundDataDC) << "Rebound attributes are not valid\n";
            return Rebound(0);
        }
        r.setAttributes(attributesDoc.byteArrayValuesHash());
    }
    r.setPayload(doc.byteArrayValue("p"));

    return r;
}

}
