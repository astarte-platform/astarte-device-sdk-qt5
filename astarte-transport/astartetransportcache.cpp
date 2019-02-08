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


#include "astartetransportcache.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTimerEvent>

#include <hyperdriveconfig.h>
#include <hyperdriveinterface.h>
#include <transportdatabasemanager.h>

#include <HemeraCore/Literals>

#include <HyperspaceProducerConsumer/ProducerAbstractInterface>

class AstarteTransportCache::Private
{
public:
    QHash< QByteArray, QByteArray > persistentEntries;
    QHash< int, Hyperdrive::CacheMessage> inFlightEntries;
    QHash< int, Hyperdrive::CacheMessage > retryEntries;
    QHash< int, int > retryTimerToId;
    int retryIdCounter;

    Private()
    {
        retryIdCounter = 0;
    }
};

static AstarteTransportCache* s_instance;

static QString s_persistencyDir;

AstarteTransportCache::AstarteTransportCache(QObject *parent)
    : Hemera::AsyncInitObject(parent)
    , m_dbOk(false)
    , d(new Private)
{
}

AstarteTransportCache *AstarteTransportCache::instance()
{
    if (Q_UNLIKELY(!s_instance)) {
        s_instance = new AstarteTransportCache();
    }

    return s_instance;
}

AstarteTransportCache::~AstarteTransportCache()
{
    delete d;
}

void AstarteTransportCache::initImpl()
{
    if (ensureDatabase()) {

        d->persistentEntries = Hyperdrive::TransportDatabaseManager::Transactions::allPersistentEntries();
        QList<Hyperdrive::CacheMessage> dbMessages = Hyperdrive::TransportDatabaseManager::Transactions::allCacheMessages();
        for (const Hyperdrive::CacheMessage &message : dbMessages) {
           d->retryEntries.insert(d->retryIdCounter++, message);
        }
        setReady();
    } else {
        setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()), QStringLiteral("Could not open the persistence database"));
    }
}

void AstarteTransportCache::setPersistencyDir(const QString &persistencyDir)
{
    s_persistencyDir = persistencyDir;
}

bool AstarteTransportCache::ensureDatabase()
{
    if (!m_dbOk) {
        m_dbOk = Hyperdrive::TransportDatabaseManager::ensureDatabase(QStringLiteral("%1/persistence.db").arg(s_persistencyDir),
                                                                      QStringLiteral("%1/db/migrations").arg(QLatin1String(Hyperdrive::StaticConfig::transportAstarteDataDir())));
    }
    return m_dbOk;
}

void AstarteTransportCache::insertOrUpdatePersistentEntry(const QByteArray &target, const QByteArray &payload)
{
    ensureDatabase();
    if (d->persistentEntries.contains(target)) {
        Hyperdrive::TransportDatabaseManager::Transactions::updatePersistentEntry(target, payload);
    } else {
        Hyperdrive::TransportDatabaseManager::Transactions::insertPersistentEntry(target, payload);
    }
    d->persistentEntries.insert(target, payload);
}

void AstarteTransportCache::removePersistentEntry(const QByteArray &target)
{
    ensureDatabase();
    Hyperdrive::TransportDatabaseManager::Transactions::deletePersistentEntry(target);
    d->persistentEntries.remove(target);
}

bool AstarteTransportCache::isCached(const QByteArray &target) const
{
    return d->persistentEntries.contains(target);
}

QByteArray AstarteTransportCache::persistentEntry(const QByteArray &target) const
{
    return d->persistentEntries.value(target);
}

QHash< QByteArray, QByteArray > AstarteTransportCache::allPersistentEntries() const
{
    return d->persistentEntries;
}

void AstarteTransportCache::addInFlightEntry(int messageId, Hyperdrive::CacheMessage message)
{
    if (message.attributes().value("retention").toInt() == static_cast<int>(Hyperspace::Retention::Discard)) {
        // QoS 0, discard it
        return;
    }

    if (message.interfaceType() == Hyperdrive::Interface::Type::Properties ||
        message.attributes().value("retention").toInt() == static_cast<int>(Hyperspace::Retention::Stored)) {

        insertIntoDatabaseIfNotPresent(message);
    }
    d->inFlightEntries.insert(messageId, message);
}

Hyperdrive::CacheMessage AstarteTransportCache::takeInFlightEntry(int messageId)
{
    removeFromDatabase(d->inFlightEntries.value(messageId));
    return d->inFlightEntries.take(messageId);
}

void AstarteTransportCache::resetInFlightEntries()
{
    for (Hyperdrive::CacheMessage c : d->inFlightEntries.values()) {
        addRetryEntry(c);
    }
    d->inFlightEntries.clear();
}

void AstarteTransportCache::insertIntoDatabaseIfNotPresent(Hyperdrive::CacheMessage &message)
{
    if (!message.hasAttribute("dbId")) {
        // We have to insert it in the db

        ensureDatabase();
        QDateTime absoluteExpiry;
        // Check if we don't have an absolute expiry
        if (!message.hasAttribute("absoluteExpiry")) {
            // If we actually have an expiry, convert it to an absolute one
            if (message.hasAttribute("expiry")) {
                int relativeExpiry = message.attribute("expiry").toInt();
                if (relativeExpiry > 0) {
                    absoluteExpiry = QDateTime::currentDateTime().addSecs(relativeExpiry);
                    message.addAttribute("absoluteExpiry", QByteArray::number(absoluteExpiry.toMSecsSinceEpoch()));
                    message.removeAttribute("expiry");
                }
            }
        } else {
            absoluteExpiry = QDateTime::fromMSecsSinceEpoch(message.attribute("absoluteExpiry").toLongLong());
        }

        int dbId = Hyperdrive::TransportDatabaseManager::Transactions::insertCacheMessage(message, absoluteExpiry);
        message.addAttribute("dbId", QByteArray::number(dbId));
    }
}

int AstarteTransportCache::addRetryEntry(Hyperdrive::CacheMessage message)
{
    if (message.attributes().value("retention").toInt() == static_cast<int>(Hyperspace::Retention::Discard)) {
        // QoS 0, discard it
        return -1;
    }
    if (message.interfaceType() == Hyperdrive::Interface::Type::Properties ||
        message.attributes().value("retention").toInt() == static_cast<int>(Hyperspace::Retention::Stored)) {

        insertIntoDatabaseIfNotPresent(message);
    }
    int id = d->retryIdCounter++;
    d->retryEntries.insert(id, message);

    int relativeExpiryms = 0;
    if (message.hasAttribute("absoluteExpiry")) {
        QDateTime absoluteExpiry = QDateTime::fromMSecsSinceEpoch(message.attribute("absoluteExpiry").toLongLong());
        relativeExpiryms = QDateTime::currentDateTime().msecsTo(absoluteExpiry);
    } else if (message.hasAttribute("expiry")) {
        relativeExpiryms = message.attribute("expiry").toInt() * 1000;
    }
    if (relativeExpiryms > 0) {
        int timerId = startTimer(relativeExpiryms);
        d->retryTimerToId.insert(timerId, id);
    }

    return id;
}

void AstarteTransportCache::removeRetryEntry(int id)
{
    removeFromDatabase(d->retryEntries.value(id));
    d->retryEntries.remove(id);
}

Hyperdrive::CacheMessage AstarteTransportCache::takeRetryEntry(int id)
{
    return d->retryEntries.take(id);
}

QList< int > AstarteTransportCache::allRetryIds() const
{
    return d->retryEntries.keys();
}

void AstarteTransportCache::removeFromDatabase(const Hyperdrive::CacheMessage &message)
{
    if (message.hasAttribute("dbId")) {
        ensureDatabase();
        Hyperdrive::TransportDatabaseManager::Transactions::deleteCacheMessage(message.attribute("dbId").toInt());
    }
}

void AstarteTransportCache::timerEvent(QTimerEvent *event)
{
    if (d->retryTimerToId.contains(event->timerId())) {
        int messageId = d->retryTimerToId.take(event->timerId());
        removeRetryEntry(messageId);
    }
}
