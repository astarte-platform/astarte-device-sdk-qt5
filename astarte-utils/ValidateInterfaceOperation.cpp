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
