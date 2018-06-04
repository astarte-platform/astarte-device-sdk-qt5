/*
 * Copyright (C) 2018 Ispirata Srl
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

#include "defaultcredentialssecretprovider.h"

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(defaultCredSecretProviderDC, "astarte.defaultcredentialssecretprovider", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Astarte {

DefaultCredentialsSecretProvider::DefaultCredentialsSecretProvider(HTTPEndpoint *parent)
    : CredentialsSecretProvider(parent)
{
}

DefaultCredentialsSecretProvider::~DefaultCredentialsSecretProvider()
{
}

void DefaultCredentialsSecretProvider::ensureCredentialsSecret()
{
}

QByteArray DefaultCredentialsSecretProvider::credentialsSecret() const
{
    return m_credentialsSecret;
}

void DefaultCredentialsSecretProvider::setAgentKey(const QByteArray &agentKey)
{
    m_agentKey = agentKey;
}

void DefaultCredentialsSecretProvider::setEndpointConfigurationPath(const QString &endpointConfigurationPath)
{
    m_endpointConfigurationPath = endpointConfigurationPath;
}

void DefaultCredentialsSecretProvider::setEndpointUrl(const QUrl &endpointUrl)
{
    m_endpointUrl = endpointUrl;
}

void DefaultCredentialsSecretProvider::setHardwareId(const QByteArray &hardwareId)
{
    m_hardwareId = hardwareId;
}

void DefaultCredentialsSecretProvider::setNAM(QNetworkAccessManager *nam)
{
    m_nam = nam;
}

void DefaultCredentialsSecretProvider::setSslConfiguration(const QSslConfiguration &configuration)
{
    m_sslConfiguration = configuration;
}

}
