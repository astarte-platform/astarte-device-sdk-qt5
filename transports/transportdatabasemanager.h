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

#ifndef TRANSPORT_DATABASE_MANAGER_H
#define TRANSPORT_DATABASE_MANAGER_H

#include <cachemessage.h>

#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QString>

namespace Hyperdrive
{

namespace TransportDatabaseManager
{
    bool ensureDatabase(const QString &dbPath = QString(), const QString &migrationsDirPath = QString());

namespace Transactions
{
    bool insertPersistentEntry(const QByteArray &target, const QByteArray &payload);
    bool updatePersistentEntry(const QByteArray &target, const QByteArray &payload);
    bool deletePersistentEntry(const QByteArray &target);
    QHash<QByteArray, QByteArray> allPersistentEntries();

    int insertCacheMessage(const CacheMessage &cacheMessage, const QDateTime &expiry = QDateTime());
    bool deleteCacheMessage(int id);
    QList<CacheMessage> allCacheMessages();
}

}

}

#endif
