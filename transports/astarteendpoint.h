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

#ifndef ASTARTE_ENDPOINT_H
#define ASTARTE_ENDPOINT_H

#include <HemeraCore/AsyncInitObject>
#include "astartecrypto.h"

#include <QtCore/QUrl>

class QNetworkReply;

namespace Hyperdrive {
class MQTTClientWrapper;
}

namespace Astarte {

class EndpointPrivate;
class Endpoint : public Hemera::AsyncInitObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Endpoint)
    Q_DECLARE_PRIVATE_D(d_h_ptr, Endpoint)

public:
    virtual ~Endpoint();

    QUrl endpoint() const;
    virtual QString endpointVersion() const;
    virtual QUrl mqttBrokerUrl() const = 0;

    virtual Hyperdrive::MQTTClientWrapper *createMqttClientWrapper() = 0;

    virtual bool isPaired() const = 0;

    virtual Hemera::Operation *pair(bool force = true) = 0;

    virtual Hemera::Operation *verifyCertificate() = 0;

    virtual QNetworkReply *sendRequest(const QString &relativeEndpoint, const QByteArray &payload,
                                       Crypto::AuthenticationDomain authenticationDomain = Crypto::DeviceAuthenticationDomain) = 0;

protected:
    explicit Endpoint(EndpointPrivate &dd, const QUrl &endpoint, QObject *parent = nullptr);
};
}

#endif // ASTARTE_ENDPOINT_H
