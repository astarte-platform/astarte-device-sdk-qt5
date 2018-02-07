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
