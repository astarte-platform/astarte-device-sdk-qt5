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

#ifndef _HYPERSPACE_BSONDOCUMENT_H_
#define _HYPERSPACE_BSONDOCUMENT_H_

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QVariant>

namespace Hyperspace
{

namespace Util
{

class BSONDocument
{
    public:
        BSONDocument(const QByteArray &document);
        int size() const;
        bool isValid() const;
        bool contains(const char *name) const;

        QVariant value(const char *name, QVariant defaultValue = QVariant()) const;

        double doubleValue(const char *name, double defaultValue = 0.0) const;
        QByteArray byteArrayValue(const char *name, const QByteArray &defaultValue = QByteArray()) const;
        QString stringValue(const char *name, const QString &defaultValue = QString()) const;
        QDateTime dateTimeValue(const char *name, const QDateTime &defaultValue = QDateTime()) const;
        int32_t int32Value(const char *name, int32_t defaultValue = 0) const;
        int64_t int64Value(const char *name, int64_t defaultValue = 0) const;
        bool booleanValue(const char *name, bool defaultValue = false) const;

        BSONDocument subdocument(const char *name) const;
        QHash<QByteArray, QByteArray> byteArrayValuesHash() const;

        QByteArray toByteArray() const;

    private:
        const QByteArray m_doc;
};

} // Util
} // Hyperspace

#endif
