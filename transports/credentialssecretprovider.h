/*
 * This file is part of Astarte.
 *
 * Copyright 2018 Ispirata Srl
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

#ifndef ASTARTE_CREDENTIALSSECRETPROVIDER_H
#define ASTARTE_CREDENTIALSSECRETPROVIDER_H

#include <QtCore/QObject>

#include "astartehttpendpoint.h"

namespace Astarte {

class CredentialsSecretProvider : public QObject
{
    Q_OBJECT

public:
    CredentialsSecretProvider(HTTPEndpoint *parent);
    virtual ~CredentialsSecretProvider();

    virtual QByteArray credentialsSecret() const = 0;

    virtual void ensureCredentialsSecret() = 0;

Q_SIGNALS:
    void credentialsSecretReady();

protected:
    HTTPEndpoint *m_endpoint;
};

}

#endif
