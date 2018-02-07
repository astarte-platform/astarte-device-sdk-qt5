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
