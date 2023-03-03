/*
 * This file is part of Astarte.
 *
 * Copyright 2017-2018 Ispirata Srl
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

#ifndef ASTARTE_HTTPENDPOINT_P_H
#define ASTARTE_HTTPENDPOINT_P_H

#include "astartehttpendpoint.h"

#include "astarteendpoint_p.h"

class QNetworkAccessManager;

namespace Astarte {

class CredentialsSecretProvider;

class HTTPEndpointPrivate : public EndpointPrivate {

public:
    HTTPEndpointPrivate(HTTPEndpoint *q)
        : EndpointPrivate(q)
        , credentialsSecretProvider(nullptr)
    {}

    Q_DECLARE_PUBLIC(HTTPEndpoint)

    QString endpointName;
    QString endpointVersion;
    QString configurationFile;
    QString persistencyDir;
    QUrl mqttBroker;
    QNetworkAccessManager *nam;
    QByteArray hardwareId;

    QByteArray pairingJwt;
    QByteArray credentialsSecret;
    QString brokerCa;
    bool ignoreSslErrors;

    QSslConfiguration sslConfiguration;
    CredentialsSecretProvider *credentialsSecretProvider;

    void connectToEndpoint();
    void ensureCredentialsSecret();
    void retryConnectToEndpointLater();
};

}

#endif // ASTARTE_HTTPENDPOINT_P_H
