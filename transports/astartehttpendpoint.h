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

#ifndef ASTARTE_HTTPENDPOINT_H
#define ASTARTE_HTTPENDPOINT_H

#include "astarteendpoint.h"
#include "astartecrypto.h"

#include <QtCore/QUrl>

#include <QtNetwork/QSslConfiguration>

class QNetworkReply;

namespace Hyperdrive {
class MQTTClientWrapper;
}

namespace Astarte {

class HTTPEndpointPrivate;
class HTTPEndpoint : public Endpoint
{
    Q_OBJECT
    Q_DISABLE_COPY(HTTPEndpoint)
    Q_DECLARE_PRIVATE_D(d_h_ptr, HTTPEndpoint)

    Q_PRIVATE_SLOT(d_func(), void connectToEndpoint())

    friend class PairOperation;

public:
    explicit HTTPEndpoint(const QString &configurationFile, const QString &persistencyDir, const QUrl &endpoint, const QSslConfiguration &sslConfiguration = QSslConfiguration(), QObject *parent = nullptr);
    virtual ~HTTPEndpoint();

    QUrl endpoint() const;
    virtual QString endpointVersion() const override final;
    virtual QUrl mqttBrokerUrl() const override final;

    virtual Hyperdrive::MQTTClientWrapper *createMqttClientWrapper() override final;

    virtual bool isPaired() const override final;

    virtual Hemera::Operation *pair(bool force = true) override final;

    virtual Hemera::Operation *verifyCertificate() override final;

    virtual QNetworkReply *sendRequest(const QString &relativeEndpoint, const QByteArray &payload,
                                       Crypto::AuthenticationDomain authenticationDomain = Crypto::DeviceAuthenticationDomain) override final;

protected:
    virtual void initImpl() override final;

private:
    QString endpointConfigurationBasePath() const;
    QString pathToAstarteEndpointConfiguration(const QString &endpointName) const;
};
}

#endif // ASTARTE_HTTPENDPOINT_H
