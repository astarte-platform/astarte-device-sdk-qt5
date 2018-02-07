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

#include "transportdatabasemanager.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QLoggingCategory>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#define VERSION_VALUE 0

#define TARGET_VALUE 0
#define PAYLOAD_VALUE 1

#define EXPIRY_VALUE 0

#define ID_VALUE 0
#define CACHEMESSAGE_VALUE 1

Q_LOGGING_CATEGORY(transportDatabaseManagerDC, "hyperdrive.transportdatabasemanager", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperdrive {

namespace TransportDatabaseManager {

bool ensureDatabase(const QString &dbPath, const QString &migrationsDirPath)
{
    if (QSqlDatabase::database().isValid()) {
        return true;
    }

    if (dbPath.isEmpty() || migrationsDirPath.isEmpty()) {
        qCWarning(transportDatabaseManagerDC) << "You have to provide dbPath and migrationsDirPath in the first call to ensureDatabase";
        return false;
    }

    // Let's create our connection.
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));

    // Does the directory exist? We have to create, or SQLITE will complain.
    QDir dbDir(QFileInfo(dbPath).dir());
    if (!dbDir.exists()) {
        qDebug() << "Creating path... " << dbDir.mkpath(dbDir.absolutePath());
    }

    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCWarning(transportDatabaseManagerDC) << "Could not open database!" << dbPath << db.lastError().text();
        return false;
    }

    QSqlQuery checkQuery(QStringLiteral("pragma quick_check"));
    if (!checkQuery.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Database" << dbPath << " is corrupted, deleting it and starting from a new one " << checkQuery.lastError();
        db.close();
        if (!QFile::remove(dbPath)) {
            qCWarning(transportDatabaseManagerDC) << "Can't remove database " << dbPath << ", giving up ";
            return false;
        }
        if (!db.open()) {
            qCWarning(transportDatabaseManagerDC) << "Can't reopen database " << dbPath << " after deleting it, giving up " << db.lastError().text();
            return false;
        }
    }

    QSqlQuery migrationQuery;

    // Ok. Let's query our migrations.
    QDir migrationsDir(migrationsDirPath);
    migrationsDir.setFilter(QDir::Files | QDir::NoSymLinks);
    migrationsDir.setSorting(QDir::Name);
    QHash< int, QString > migrations;
    int latestSchemaVersion = 0;
    for (const QFileInfo &migration : migrationsDir.entryInfoList()) {
        latestSchemaVersion = migration.baseName().split(QLatin1Char('_')).first().toInt();
        migrations.insert(latestSchemaVersion, migration.absoluteFilePath());
    }

    // Query our schema table
    QSqlQuery schemaQuery(QStringLiteral("SELECT version from schema_version"));
    int currentSchemaVersion = -1;
    while (schemaQuery.next()) {
        if (schemaQuery.value(VERSION_VALUE).toInt() == latestSchemaVersion) {
            // Yo.
            return true;
        } else if (schemaQuery.value(VERSION_VALUE).toInt() > latestSchemaVersion) {
            // Yo.
            qCWarning(transportDatabaseManagerDC) << "Something is weird!! Our schema version is higher than available migrations? Ignoring and hoping for the best...";
            return true;
        }

        // Let's point out which one we need.
        currentSchemaVersion = schemaQuery.value(VERSION_VALUE).toInt();
    }

    // We need to migrate.
    qCWarning(transportDatabaseManagerDC) << "Found these migrations:" << migrations;
    qCWarning(transportDatabaseManagerDC) << "Latest schema version is " << latestSchemaVersion;

    // If we got here, we might need to create the schema table first.
    if (currentSchemaVersion < 0) {
        qCDebug(transportDatabaseManagerDC) << "Creating schema_version table...";
        if (!migrationQuery.exec(QStringLiteral("CREATE TABLE schema_version(version integer)"))) {
            qCWarning(transportDatabaseManagerDC) << "Could not create schema_version!!" << migrationQuery.lastError();
            return false;
        }
        if (!migrationQuery.exec(QStringLiteral("INSERT INTO schema_version (version) VALUES (0)"))) {
            qCWarning(transportDatabaseManagerDC) << "Could not create schema_version!!" << migrationQuery.lastError();
            return false;
        }
    }

    qCDebug(transportDatabaseManagerDC) << "Now performing migrations" << currentSchemaVersion << latestSchemaVersion;
    // Perform migrations.
    for (++currentSchemaVersion; currentSchemaVersion <= latestSchemaVersion; ++currentSchemaVersion) {
        if (!migrations.contains(currentSchemaVersion)) {
            continue;
        }

        // Apply migration
        QFile migrationFile(migrations.value(currentSchemaVersion));
        if (migrationFile.open(QIODevice::ReadOnly)) {
            QString queryString = QTextStream(&migrationFile).readAll();
            if (!migrationQuery.exec(queryString)) {
                qCWarning(transportDatabaseManagerDC) << "Could not execute migration" << currentSchemaVersion << migrationQuery.lastError();
                return false;
            }
        }

        // Update schema version
        if (!migrationQuery.exec(QStringLiteral("UPDATE schema_version SET version=%1").arg(currentSchemaVersion))) {
            qCWarning(transportDatabaseManagerDC) << "Could not update schema_version!! This error is critical, your database is compromised!!" << migrationQuery.lastError();
            return false;
        }

        qCDebug(transportDatabaseManagerDC) << "Migration performed" << currentSchemaVersion;
    }

    // We're set!
    return true;
}

bool Transactions::insertPersistentEntry(const QByteArray &target, const QByteArray &payload)
{
    if (!ensureDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO persistent_entries (target, payload) "
                                 "VALUES (:target, :payload)"));
    query.bindValue(QStringLiteral(":target"), QLatin1String(target));
    query.bindValue(QStringLiteral(":payload"), payload);

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Insert persistent entry query failed!" << query.lastError();
        return false;
    }

    return true;
}

bool Transactions::updatePersistentEntry(const QByteArray &target, const QByteArray &payload)
{
    if (!ensureDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE persistent_entries SET payload=:payload "
                                 "WHERE target=:target"));
    query.bindValue(QStringLiteral(":target"), QLatin1String(target));
    query.bindValue(QStringLiteral(":payload"), payload);

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Update persistent entry query failed!" << query.lastError();
        return false;
    }

    return true;
}

bool Transactions::deletePersistentEntry(const QByteArray &target)
{
    if (!ensureDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM persistent_entries WHERE target=:target"));
    query.bindValue(QStringLiteral(":target"), QLatin1String(target));

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Delete persistent entry " << target << " query failed!" << query.lastError();
        return false;
    }

    return true;
}

QHash<QByteArray, QByteArray> Transactions::allPersistentEntries()
{
    QHash<QByteArray, QByteArray> ret;

    if (!ensureDatabase()) {
        return ret;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT target, payload FROM persistent_entries"));

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "All persistent entries query failed!" << query.lastError();
        return ret;
    }

    while (query.next()) {
        ret.insert(query.value(TARGET_VALUE).toByteArray(), query.value(PAYLOAD_VALUE).toByteArray());
    }

    return ret;
}

int Transactions::insertCacheMessage(const CacheMessage &cacheMessage, const QDateTime &expiry)
{
    if (!ensureDatabase()) {
        return -1;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO cachemessages (cachemessage, expiry) "
                                 "VALUES (:cachemessage, :expiry)"));
    query.bindValue(QStringLiteral(":cachemessage"), cacheMessage.serialize());
    query.bindValue(QStringLiteral(":expiry"), expiry);

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Insert CacheMessage query failed!" << query.lastError();
        return -1;
    }

    return query.lastInsertId().toInt();
}

bool Transactions::deleteCacheMessage(int id)
{
    if (!ensureDatabase()) {
        return false;
    }

    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM cachemessages WHERE id=:id"));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Delete CacheMessage query failed!" << query.lastError();
        return false;
    }

    return true;
}

QList<CacheMessage> Transactions::allCacheMessages()
{
    QList<CacheMessage> ret;

    if (!ensureDatabase()) {
        return ret;
    }

    // Housekeeping: delete expired CacheMessages
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM cachemessages WHERE expiry < :now"));
    query.bindValue(QStringLiteral(":now"), QDateTime::currentDateTime());

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "Delete expired CacheMessages query failed!" << query.lastError();
        return ret;
    }

    query.prepare(QStringLiteral("SELECT id, cachemessage FROM cachemessages"));

    if (!query.exec()) {
        qCWarning(transportDatabaseManagerDC) << "All CacheMessages query failed!" << query.lastError();
        return ret;
    }

    while (query.next()) {
        CacheMessage c = CacheMessage::fromBinary(query.value(CACHEMESSAGE_VALUE).toByteArray());
        c.addAttribute("dbId", QByteArray::number(query.value(ID_VALUE).toInt()));
        ret.append(c);
    }

    return ret;
}

}

}
