/*
 * This file is part of Astarte.
 *
 * Copyright 2018 Ispirata Srl
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

#ifndef ASTARTE_DEFAULTCREDENTIALSSECRETPROVIDER_H
#define ASTARTE_DEFAULTCREDENTIALSSECRETPROVIDER_H

#include "credentialssecretprovider.h"

#include <QtCore/QUrl>

class QNetworkAccessManager;

namespace Astarte {

class DefaultCredentialsSecretProvider : public CredentialsSecretProvider {
    Q_OBJECT

public:
    DefaultCredentialsSecretProvider(HTTPEndpoint *parent);
    ~DefaultCredentialsSecretProvider();

    void ensureCredentialsSecret() override;

    QByteArray credentialsSecret() const override;

    void setPairingJwt(const QByteArray &pairingJwt);
    void setEndpointConfigurationPath(const QString &endpointConfigurationPath);
    void setEndpointUrl(const QUrl &endpointUrl);
    void setHardwareId(const QByteArray &hardwareId);
    void setNAM(QNetworkAccessManager *nam);
    void setSslConfiguration(const QSslConfiguration &configuration);

private slots:
    void sendRegistrationRequest();
    void retryRegistrationLater();

private:
    QByteArray m_pairingJwt;
    QByteArray m_hardwareId;
    QNetworkAccessManager *m_nam;
    QSslConfiguration m_sslConfiguration;
    QString m_endpointConfigurationPath;
    QUrl m_endpointUrl;

    QByteArray m_credentialsSecret;
};

}

#endif
