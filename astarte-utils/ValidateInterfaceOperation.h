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

#ifndef VALIDATEINTERFACESOPERATION_H
#define VALIDATEINTERFACESOPERATION_H

#include <HemeraCore/Operation>

class QJsonSchemaChecker;

class ValidateInterfaceOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(ValidateInterfaceOperation)

public:
    explicit ValidateInterfaceOperation(const QString &interface, QObject *parent = nullptr);
    virtual ~ValidateInterfaceOperation();

protected:
    virtual void startImpl() override final;

private:
    QJsonSchemaChecker *m_checker;
    QString m_path;
};

#endif
