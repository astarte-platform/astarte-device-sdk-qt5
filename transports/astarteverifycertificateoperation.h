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

#ifndef ASTARTEVERIFYCERTIFICATEOPERATION_H
#define ASTARTEVERIFYCERTIFICATEOPERATION_H

#include "astarteendpoint.h"
#include "astartecrypto.h"

#include <QtCore/QUrl>

#include <QtNetwork/QSslConfiguration>

#include <HemeraCore/Operation>

class QFile;

namespace Astarte {

class HTTPEndpoint;

class VerifyCertificateOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(VerifyCertificateOperation)

public:
    explicit VerifyCertificateOperation(QFile &certFile, HTTPEndpoint *parent);
    explicit VerifyCertificateOperation(const QByteArray &certificate, HTTPEndpoint *parent);
    explicit VerifyCertificateOperation(const QSslCertificate &certificate, HTTPEndpoint *parent);
    virtual ~VerifyCertificateOperation();

protected:
    void startImpl();

private:
    void verify(const QByteArray &certificate);

    QByteArray m_certificate;
    HTTPEndpoint *m_endpoint;
};
}

#endif // ASTARTEVERIFYCERTIFICATEOPERATION_H
