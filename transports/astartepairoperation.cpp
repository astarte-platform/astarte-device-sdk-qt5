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

#include "astartepairoperation.h"

#include "astartehttpendpoint.h"
#include "astartehttpendpoint_p.h"

#include <HemeraCore/Literals>

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSettings>

#include <QtNetwork/QNetworkReply>

Q_LOGGING_CATEGORY(astartePairOperationDC, "astarte.pairingoperation", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Astarte {

PairOperation::PairOperation(HTTPEndpoint *parent)
    : Hemera::Operation(parent)
    , m_endpoint(parent)
{
}

PairOperation::~PairOperation()
{
}

void PairOperation::startImpl()
{
    // Before anything else, we need to check if we have an available keystore.
    if (!Crypto::instance()->isKeyStoreAvailable()) {
        // Let's build one.
        Hemera::Operation *op = Crypto::instance()->generateAstarteKeyStore();
        connect(op, &Hemera::Operation::finished, this, [this, op] {
            if (op->isError()) {
                // That's ugly.
                setFinishedWithError(op->errorName(), op->errorMessage());
                return;
            }

            initiatePairing();
        });
    } else {
        // Let's just go
        initiatePairing();
    }
}

void PairOperation::initiatePairing()
{
    // FIXME: This should be done using Global configuration!!
    QSettings settings(QStringLiteral("%1/endpoint_crypto.conf").arg(m_endpoint->pathToAstarteEndpointConfiguration(m_endpoint->d_func()->endpointName)),
                       QSettings::IniFormat);
    if (settings.value(QStringLiteral("apiKey")).toString().isEmpty()) {
        performFakeAgentPairing();
    } else {
        performPairing();
    }
}

void PairOperation::performFakeAgentPairing()
{
    qWarning() << "Fake agent pairing!";
    QJsonObject o;
    o.insert(QStringLiteral("hwId"), QLatin1String(m_endpoint->d_func()->hardwareId));

    QByteArray deviceIDPayload = QJsonDocument(o).toJson(QJsonDocument::Compact);
    QNetworkReply *r = m_endpoint->sendRequest(QStringLiteral("/devices/apikeysFromDevice"), deviceIDPayload, Crypto::CustomerAuthenticationDomain);

    qCDebug(astartePairOperationDC) << "I'm sending: " << deviceIDPayload.constData();

    connect(r, &QNetworkReply::finished, this, [this, r, deviceIDPayload] {
        if (r->error() != QNetworkReply::NoError) {
            qCWarning(astartePairOperationDC) << "Pairing error! Error: " << r->error();
            setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()), r->errorString());
            r->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
        r->deleteLater();
        qCDebug(astartePairOperationDC) << "Got the ok!";
        if (!doc.isObject()) {
            qCWarning(astartePairOperationDC) << "Parsing pairing result error!";
            setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()), QStringLiteral("Parsing pairing result error!"));
            return;
        }

        qCDebug(astartePairOperationDC) << "Payload is " << doc.toJson().constData();

        QJsonObject pairData = doc.object();
        if (!pairData.contains(QStringLiteral("apiKey"))) {
            qCWarning(astartePairOperationDC) << "Missing apiKey in the pairing routine!";
            setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()),
                                 QStringLiteral("Missing apiKey in the pairing routine!"));
            return;
        }

        // Ok, we need to write the files now.
        {
            QSettings settings(QStringLiteral("%1/endpoint_crypto.conf").arg(m_endpoint->pathToAstarteEndpointConfiguration(m_endpoint->d_func()->endpointName)),
                               QSettings::IniFormat);
            settings.setValue(QStringLiteral("apiKey"), pairData.value(QStringLiteral("apiKey")).toString());
        }

        // That's all, folks!
        performPairing();
    });
}

void PairOperation::performPairing()
{
    QFile csr(Crypto::instance()->pathToCertificateRequest());
    if (!csr.open(QIODevice::ReadOnly)) {
        qCWarning(astartePairOperationDC) << "Could not open CSR for reading! Aborting.";
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::notFound()), QStringLiteral("Could not open CSR for reading! Aborting."));
    }

    QByteArray deviceIDPayload = csr.readAll();
    QNetworkReply *r = m_endpoint->sendRequest(QStringLiteral("/pairing"), deviceIDPayload, Crypto::DeviceAuthenticationDomain);

    qCDebug(astartePairOperationDC) << "I'm sending: " << deviceIDPayload.constData();

    connect(r, &QNetworkReply::finished, this, [this, r, deviceIDPayload] {
        if (r->error() != QNetworkReply::NoError) {
            qCWarning(astartePairOperationDC) << "Pairing error!";
            setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()), r->errorString());
            r->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(r->readAll());
        r->deleteLater();
        qCDebug(astartePairOperationDC) << "Got the ok!";
        if (!doc.isObject()) {
            qCWarning(astartePairOperationDC) << "Parsing pairing result error!";
            setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()), QStringLiteral("Parsing pairing result error!"));
            return;
        }

        qCDebug(astartePairOperationDC) << "Payload is " << doc.toJson().constData();

        QJsonObject pairData = doc.object();
        if (!pairData.contains(QStringLiteral("clientCrt"))) {
            qCWarning(astartePairOperationDC) << "Missing certificate in the pairing routine!";
            setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()), QStringLiteral("Missing certificate in the pairing routine!"));
            return;
        }

        // Ok, we need to write the files now.
        {
            QFile generatedCertificate(QStringLiteral("%1/mqtt_broker.crt").arg(m_endpoint->pathToAstarteEndpointConfiguration(m_endpoint->d_func()->endpointName)));
            if (!generatedCertificate.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                qCWarning(astartePairOperationDC) << "Could not write certificate!";
                setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()), generatedCertificate.errorString());
                return;
            }
            generatedCertificate.write(pairData.value(QStringLiteral("clientCrt")).toVariant().toByteArray());
            generatedCertificate.flush();
            generatedCertificate.close();
        }
        {
            QSettings settings(QStringLiteral("%1/mqtt_broker.conf").arg(m_endpoint->pathToAstarteEndpointConfiguration(m_endpoint->d_func()->endpointName)),
                               QSettings::IniFormat);
        }

        // That's all, folks!
        setFinished();
    });
}

}
