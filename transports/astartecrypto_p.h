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
