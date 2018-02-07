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
