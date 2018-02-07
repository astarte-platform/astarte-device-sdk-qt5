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
