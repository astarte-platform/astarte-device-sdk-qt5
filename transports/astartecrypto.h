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

#ifndef ASTARTE_CRYPTO_H
#define ASTARTE_CRYPTO_H

#include <HemeraCore/AsyncInitObject>
#include <QtCore/QFlags>

namespace Astarte {

// Define some convenience paths here.

class CryptoPrivate;
class Crypto : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Crypto)
    Q_DECLARE_PRIVATE_D(d_h_ptr, Crypto)

public:
    enum AuthenticationDomain {
        NoAuthenticationDomain = 0,
        DeviceAuthenticationDomain = 1 << 0,
        CustomerAuthenticationDomain = 1 << 1,
        AnyAuthenticationDomain = 255
    };
    Q_ENUMS(AuthenticationDomain)
    Q_DECLARE_FLAGS(AuthenticationDomains, AuthenticationDomain)

    static Crypto * instance();

    virtual ~Crypto();

    bool isKeyStoreAvailable() const;
    Hemera::Operation *generateAstarteKeyStore(bool forceGeneration = false);

    QByteArray sign(const QByteArray &payload, AuthenticationDomains = AnyAuthenticationDomain);

    static QString cryptoBasePath();
    static QString pathToCertificateRequest();
    static QString pathToPrivateKey();
    static QString pathToPublicKey();

    static void setCryptoBasePath(const QString &basePath);

protected:
    virtual void initImpl() override final;

Q_SIGNALS:
    void keyStoreAvailabilityChanged();

private:
    explicit Crypto(QObject *parent = 0);

    static QString s_cryptoBasePath;

    friend class ThreadedKeyOperation;
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Astarte::Crypto::AuthenticationDomains)

#endif // ASTARTE_CRYPTO_H
