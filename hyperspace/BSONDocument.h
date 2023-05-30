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
        QList<QVariant> listVariantValue(const char *name, const QList<QVariant> &defaultValue = QList<QVariant>()) const;
        QHash<QByteArray, QVariant> mapVariantValue(const char *name, const QHash<QByteArray, QVariant> &defaultValue = QHash<QByteArray, QVariant>()) const;

        BSONDocument subdocument(const char *name) const;
        QHash<QByteArray, QByteArray> byteArrayValuesHash() const;

        QByteArray toByteArray() const;

    private:
        const QByteArray m_doc;
};

} // Util
} // Hyperspace

#endif
