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

#ifndef ASTARTE_TRANSPORT_CACHE_H
#define ASTARTE_TRANSPORT_CACHE_H

#include <HemeraCore/AsyncInitObject>

#include <cachemessage.h>

class AstarteTransportCache : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AstarteTransportCache)

public:
    static AstarteTransportCache *instance();

    static void setPersistencyDir(const QString &persistencyDir);

    virtual ~AstarteTransportCache();

public Q_SLOTS:
    void insertOrUpdatePersistentEntry(const QByteArray &target, const QByteArray &payload);
    void removePersistentEntry(const QByteArray &target);

    QByteArray persistentEntry(const QByteArray &target) const;
    QHash< QByteArray, QByteArray > allPersistentEntries() const;

    bool isCached(const QByteArray &target) const;

    void addInFlightEntry(int messageId, Hyperdrive::CacheMessage message);
    Hyperdrive::CacheMessage takeInFlightEntry(int messageId);
    void resetInFlightEntries();

    int addRetryEntry(Hyperdrive::CacheMessage message);
    void removeRetryEntry(int messageId);

    Hyperdrive::CacheMessage takeRetryEntry(int id);
    QList<int> allRetryIds() const;

    void removeFromDatabase(const Hyperdrive::CacheMessage &message);


protected:
    virtual void initImpl() override final;
    virtual void timerEvent(QTimerEvent *event) override final;

private:
    explicit AstarteTransportCache(QObject *parent = nullptr);

    void insertIntoDatabaseIfNotPresent(Hyperdrive::CacheMessage &message);

    bool ensureDatabase();

    bool m_dbOk;

    class Private;
    Private * const d;
};

#endif // ASTARTE_TRANSPORT_CACHE_H


