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

#ifndef _BSON_SERIALIZER_H_
#define _BSON_SERIALIZER_H_

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QVariantHash>
#include <QtCore/QVariantMap>

namespace Hyperspace
{

namespace Util
{

class BSONSerializer
{
    public:
        BSONSerializer();

        QByteArray document() const;

        void appendEndOfDocument();

        void appendDoubleValue(const char *name, double value);
        void appendInt32Value(const char *name, int32_t value);
        void appendInt64Value(const char *name, int64_t value);
        void appendBinaryValue(const char *name, const QByteArray &value);
        void appendASCIIString(const char *name, const QByteArray &string);
        void appendDocument(const char *name, const QByteArray &document);
        void appendString(const char *name, const QString &string);
        void appendDateTime(const char *name, const QDateTime &dateTime);
        void appendBooleanValue(const char *name, bool value);

        void appendValue(const char *name, const QVariant &value);
        void appendDocument(const char *name, const QVariantHash &document);
        void appendDocument(const char *name, const QVariantMap &document);

    private:
        void beginSubdocument(const char *name);
        QByteArray m_doc;
};

}

}

#endif
