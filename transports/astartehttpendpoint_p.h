/*
 * Copyright (C) 2017-2018 Ispirata Srl
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

    QByteArray agentKey;
    QString brokerCa;
    bool ignoreSslErrors;

    QSslConfiguration sslConfiguration;
    CredentialsSecretProvider *credentialsSecretProvider;

    void connectToEndpoint();
};

}

#endif // ASTARTE_HTTPENDPOINT_P_H
