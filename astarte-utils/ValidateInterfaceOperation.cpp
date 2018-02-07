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

#include "ValidateInterfaceOperation.h"

#include "QJsonSchemaChecker.h"

#include <hyperdriveconfig.h>

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <HemeraCore/Literals>


ValidateInterfaceOperation::ValidateInterfaceOperation(const QString &path, QObject *parent)
    : m_checker(new QJsonSchemaChecker())
    , m_path(path)
{
}

ValidateInterfaceOperation::~ValidateInterfaceOperation()
{
}

void ValidateInterfaceOperation::startImpl()
{
    QFile schemaFile(QStringLiteral("%1/interface.json").arg(
                QLatin1String(Hyperdrive::StaticConfig::transportAstarteDataDir())));
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::notFound()),
                             QStringLiteral("Schema file %1 does not exist").arg(schemaFile.fileName()));
        return;
    }

    QJsonDocument schemaJson = QJsonDocument::fromJson(schemaFile.readAll());
    if (!schemaJson.isObject()) {
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                             QStringLiteral("Schema file %1 does not contain a JSON object").arg(schemaFile.fileName()));
        return;
    }

    m_checker->setSchema(schemaJson.object());

    QFile interfaceFile(m_path);
    if (!interfaceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::notFound()),
                             QStringLiteral("Interface file %1 does not exist").arg(interfaceFile.fileName()));
        return;
    }

    QJsonDocument interfaceJson = QJsonDocument::fromJson(interfaceFile.readAll());
    if (!interfaceJson.isObject()) {
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                             QStringLiteral("Interface file %1 does not contain a JSON object").arg(interfaceFile.fileName()));
        return;
    }

    if (m_checker->validate(interfaceJson.object())) {
        setFinished();
    } else {
        setFinishedWithError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                             m_checker->getMessages().join(QStringLiteral("\n")));
    }
}
