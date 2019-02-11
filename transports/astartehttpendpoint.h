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
