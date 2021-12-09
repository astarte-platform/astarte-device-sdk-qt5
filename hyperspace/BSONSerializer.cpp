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

#include "BSONSerializer.h"

#include <QtCore/QByteArray>
#include <QtCore/QDebug>

#include <stdint.h>
#if defined(__APPLE__)
  #include "apple_endian.h"
#else
  #include <endian.h>
#endif

#define BSON_TYPE_DOUBLE    '\x01'
#define BSON_TYPE_STRING    '\x02'
#define BSON_TYPE_DOCUMENT  '\x03'
#define BSON_TYPE_ARRAY     '\x04'
#define BSON_TYPE_BINARY    '\x05'
#define BSON_TYPE_BOOLEAN   '\x08'
#define BSON_TYPE_DATETIME  '\x09'
#define BSON_TYPE_INT32     '\x10'
#define BSON_TYPE_INT64     '\x12'

#define BSON_SUBTYPE_DEFAULT_BINARY '\0'

#define INT32_TO_BYTES(value, buf) \
    union data32 { \
        int64_t sval; \
        uint64_t uval; \
        char valBuf[4]; \
    }; \
    data32 d32; \
    d32.sval = (value); \
    d32.uval = htole32(d32.uval); \
    buf = d32.valBuf;

#define INT64_TO_BYTES(value, buf) \
    union data64 { \
        int64_t sval; \
        uint64_t uval; \
        char valBuf[8]; \
    }; \
    data64 d64; \
    d64.sval = (value); \
    d64.uval = htole64(d64.uval); \
    buf = d64.valBuf;

#define DOUBLE_TO_BYTES(value, buf) \
    union data64 { \
        double dval; \
        uint64_t uval; \
        char valBuf[8]; \
    }; \
    data64 d64; \
    d64.dval = (value); \
    d64.uval = htole64(d64.uval); \
    buf = d64.valBuf;

namespace Hyperspace
{

namespace Util
{

BSONSerializer::BSONSerializer()
    : m_doc(QByteArray("\0\0\0\0", 4))
{
}

QByteArray BSONSerializer::document() const
{
    return m_doc;
}

void BSONSerializer::appendEndOfDocument()
{
    m_doc.append('\0');

    char *sizeBuf;
    INT32_TO_BYTES(m_doc.count(), sizeBuf)

    m_doc.replace(0, 4, QByteArray(sizeBuf, sizeof(int32_t)));
}

void BSONSerializer::appendDoubleValue(const char *name, double value)
{
    char *valBuf;
    DOUBLE_TO_BYTES(value, valBuf)

    m_doc.append(BSON_TYPE_DOUBLE);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(valBuf, 8);
}

void BSONSerializer::appendInt32Value(const char *name, int32_t value)
{
    char *valBuf;
    INT32_TO_BYTES(value, valBuf)

    m_doc.append(BSON_TYPE_INT32);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(valBuf, sizeof(int32_t));
}

void BSONSerializer::appendInt64Value(const char *name, int64_t value)
{
    char *valBuf;
    INT64_TO_BYTES(value, valBuf)

    m_doc.append(BSON_TYPE_INT64);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(valBuf, sizeof(int64_t));
}

void BSONSerializer::appendBinaryValue(const char *name, const QByteArray &value)
{
    char *lenBuf;
    INT32_TO_BYTES(value.count(), lenBuf)

    m_doc.append(BSON_TYPE_BINARY);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(lenBuf, sizeof(int32_t));
    m_doc.append(BSON_SUBTYPE_DEFAULT_BINARY);
    m_doc.append(value);
}

void BSONSerializer::appendASCIIString(const char *name, const QByteArray &string)
{
    char *lenBuf;
    INT32_TO_BYTES(string.count() + 1, lenBuf)

    m_doc.append(BSON_TYPE_STRING);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(lenBuf, sizeof(int32_t));
    m_doc.append(string.constData());
    m_doc.append('\0');
}

void BSONSerializer::appendString(const char *name, const QString &string)
{
    appendASCIIString(name, string.toUtf8());
}

void BSONSerializer::appendDateTime(const char *name, const QDateTime &dateTime)
{
    int64_t millis = dateTime.toUTC().toMSecsSinceEpoch();
    char *valBuf;
    INT64_TO_BYTES(millis, valBuf)

    m_doc.append(BSON_TYPE_DATETIME);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(valBuf, sizeof(int64_t));
}

void BSONSerializer::appendBooleanValue(const char *name, bool value)
{
    m_doc.append(BSON_TYPE_BOOLEAN);
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(value ? '\1' : '\0');
}

void BSONSerializer::appendArray(const char *name, const QList<QVariant> &value)
{
    m_doc.append(BSON_TYPE_ARRAY);

    BSONSerializer subDocument;
    for (int i = 0; i < value.length(); i++) {
        subDocument.appendValue(QString::number(i).toLatin1().constData(), value[i]);
    }
    subDocument.appendEndOfDocument();
    m_doc.append(name, strlen(name) + 1);
    m_doc.append(subDocument.document());
}

void BSONSerializer::appendValue(const char *name, const QVariant &value)
{
    switch (value.type()) {
        case QVariant::Bool:
            appendBooleanValue(name, value.toBool());
            break;
        case QVariant::ByteArray:
            appendBinaryValue(name, value.toByteArray());
            break;
        case QVariant::DateTime:
            appendDateTime(name, value.toDateTime());
            break;
        case QVariant::Double:
            appendDoubleValue(name, value.toDouble());
            break;
        case QVariant::Int:
            appendInt32Value(name, value.toInt());
            break;
        case QVariant::LongLong:
            appendInt64Value(name, value.toLongLong());
            break;
        case QVariant::String:
            appendString(name, value.toString());
            break;
        case QVariant::List:
            appendArray(name, value.toList());
            break;
        default:
            qWarning() << "Can't find valid type for " << value;
    }
}

void BSONSerializer::appendDocument(const char *name, const QVariantHash &document)
{
    BSONSerializer subDocument;
    for (QVariantHash::const_iterator i = document.constBegin(); i != document.constEnd(); ++i) {
        subDocument.appendValue(i.key().toLatin1().constData(), i.value());
    }
    subDocument.appendEndOfDocument();
    appendDocument(name, subDocument.document());
}

void BSONSerializer::appendDocument(const char *name, const QVariantMap &document)
{
    BSONSerializer subDocument;
    for (QVariantMap::const_iterator i = document.constBegin(); i != document.constEnd(); ++i) {
        subDocument.appendValue(i.key().toLatin1().constData(), i.value());
    }
    subDocument.appendEndOfDocument();
    appendDocument(name, subDocument.document());
}

void BSONSerializer::beginSubdocument(const char *name)
{
    m_doc.append(BSON_TYPE_DOCUMENT);
    m_doc.append(name, strlen(name) + 1);
}

void BSONSerializer::appendDocument(const char *name, const QByteArray &document)
{
    beginSubdocument(name);
    m_doc.append(document);
}

}

}
