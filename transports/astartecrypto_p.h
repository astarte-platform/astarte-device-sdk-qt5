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

#ifndef ASTARTE_CRYPTO_P_H
#define ASTARTE_CRYPTO_P_H

#include "astartecrypto.h"

#include <HemeraCore/Operation>

namespace Astarte {
class Connector;

class ThreadedKeyOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(ThreadedKeyOperation)

public:
    explicit ThreadedKeyOperation(const QString &privateKeyFile, const QString &publicKeyFile, QObject* parent = nullptr);
    explicit ThreadedKeyOperation(const QString &cn, const QString &privateKeyFile, const QString &publicKeyFile, const QString &csrFile, QObject* parent = nullptr);
    virtual ~ThreadedKeyOperation();

protected:
    virtual void startImpl() override final;

private:
    QString m_cn;
    QString m_privateKeyFile;
    QString m_publicKeyFile;
    QString m_csrFile;
};
}

#endif // ASTARTE_CRYPTO_P_H
