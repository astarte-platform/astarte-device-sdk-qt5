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
    QJsonObject data;
    data.insert(QStringLiteral("client_crt"), QString::fromLatin1(certificate));
    QJsonObject payload;
    payload.insert(QStringLiteral("data"), data);

    QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *r = m_endpoint->sendRequest(QStringLiteral("/protocols/astarte_mqtt_v1/credentials/verify"),
                       payloadBytes, Crypto::DeviceAuthenticationDomain);

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

            QJsonObject verifyData = doc.object().value(QStringLiteral("data")).toObject();
            QString timestamp = verifyData.value(QStringLiteral("timestamp")).toString();
            qCDebug(verifyCertDC) << "Verification server timestamp: " << timestamp;
            if (verifyData.value(QStringLiteral("valid")).toBool()) {
                qCDebug(verifyCertDC) << "Valid certificate, until" << verifyData.value(QStringLiteral("until")).toString();
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
