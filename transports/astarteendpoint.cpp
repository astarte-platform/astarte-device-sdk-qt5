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

#include "astarteendpoint.h"
#include "astarteendpoint_p.h"

#include "astartecrypto.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <HemeraCore/CommonOperations>
#include <HemeraCore/Fingerprints>
#include <HemeraCore/Literals>

#include <hemeraasyncinitobject_p.h>

Q_LOGGING_CATEGORY(astarteEndpointDC, "astarte.endpoint", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Astarte {

Endpoint::Endpoint(EndpointPrivate &dd, const QUrl &endpoint, QObject* parent)
    : AsyncInitObject(dd, parent)
{
    Q_D(Endpoint);
    d->endpoint = endpoint;
}

Endpoint::~Endpoint()
{
}

QUrl Endpoint::endpoint() const
{
    Q_D(const Endpoint);
    return d->endpoint;
}

QString Endpoint::endpointVersion() const
{
    return QString();
}

}

#include "moc_astarteendpoint.cpp"
