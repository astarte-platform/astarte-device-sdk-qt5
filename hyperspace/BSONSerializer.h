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
