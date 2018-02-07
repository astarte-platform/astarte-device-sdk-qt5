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

#include "astarteverifycertificateoperation.h"

#include "astartehttpendpoint.h"

#include <hyperdriveutils.h>

#include <QtCore/QLoggingCategory>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>

#include <QtNetwork/QNetworkReply>

#include <HemeraCore/Literals>

#define RETRY_INTERVAL 15000

Q_LOGGING_CATEGORY(verifyCertDC, "astarte.httpendpoint.verifycert", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Astarte {

VerifyCertificateOperation::VerifyCertificateOperation(QFile &certFile, HTTPEndpoint *parent)
    : Hemera::Operation(parent)
    , m_endpoint(parent)
{
    if (certFile.open(QIODevice::ReadOnly)) {
        m_certificate = certFile.readAll();
    } else {
        qCWarning(verifyCertDC) << "Cannot open " << certFile.fileName() << " reason: " << certFile.error();
    }
}

VerifyCertificateOperation::VerifyCertificateOperation(const QByteArray &certificate, HTTPEndpoint *parent)
    : Hemera::Operation(parent)
    , m_certificate(certificate)
    , m_endpoint(parent)
{
}

VerifyCertificateOperation::VerifyCertificateOperation(const QSslCertificate &certificate, HTTPEndpoint *parent)
    : Hemera::Operation(parent)
    , m_certificate(certificate.toPem())
    , m_endpoint(parent)
{
}

VerifyCertificateOperation::~VerifyCertificateOperation()
{
}

void VerifyCertificateOperation::startImpl()
{
    if (m_certificate.isEmpty()) {
        qCWarning(verifyCertDC) << "Empty certificate";
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()), QStringLiteral("Invalid certificate"));
        return;
    }

    verify(m_certificate);
}

void VerifyCertificateOperation::verify(const QByteArray &certificate)
{
    QNetworkReply *r = m_endpoint->sendRequest(QStringLiteral("/verifyCertificate"), certificate, Crypto::DeviceAuthenticationDomain);

    connect(r, &QNetworkReply::finished, this, [this, certificate, r] {
        if (r->error() != QNetworkReply::NoError) {
            int retryInterval = Hyperdrive::Utils::randomizedInterval(RETRY_INTERVAL, 1.0);
            qCWarning(verifyCertDC) << "Error while verifying certificate! Retrying in " << (retryInterval / 1000) << " seconds. error: " << r->error();

            // We never give up. If we couldn't connect, we reschedule this in 15 seconds.
            QTimer::singleShot(retryInterval, this, [this, certificate] {
                verify(certificate);
            });
            r->deleteLater();
            return;
        } else {
            QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
            r->deleteLater();
            if (!doc.isObject()) {
                int retryInterval = Hyperdrive::Utils::randomizedInterval(RETRY_INTERVAL, 1.0);
                qCWarning(verifyCertDC) << "Parsing error, resending request in " << (retryInterval / 1000) << " seconds.";
                QTimer::singleShot(retryInterval, this, [this, certificate] {
                    verify(certificate);
                });
                return;
            }

            // TODO: handle timestamp field
            QJsonObject verifyData = doc.object();
            if (verifyData.value(QStringLiteral("valid")).toBool()) {
                qCDebug(verifyCertDC) << "Valid certificate";
                setFinished();
                return;
            } else {
                qCWarning(verifyCertDC) << "Invalid certificate";
                setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                                     QStringLiteral("Invalid certificate (%1): %2")
                                     .arg(verifyData.value(QStringLiteral("cause")).toString(), verifyData.value(QStringLiteral("details")).toString()));
                return;
            }
        }
    });
}

}
