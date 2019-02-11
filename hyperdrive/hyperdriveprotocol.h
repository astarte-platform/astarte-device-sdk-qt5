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

#ifndef HYPERDRIVE_PROTOCOL_H
#define HYPERDRIVE_PROTOCOL_H

#include <HyperspaceCore/Global>

#include <QtCore/QUuid>

namespace Hyperdrive {

namespace Protocol
{

enum class MessageType
{
    // Start from 1024 so we don't clash with Hyperspace
    CacheMessage = 1024
};

namespace Control
{
Q_DECL_CONSTEXPR quint8 nameExchange() { return 'y'; }
Q_DECL_CONSTEXPR quint8 listGatesForHyperdriveInterfaces() { return 'q'; }
Q_DECL_CONSTEXPR quint8 listHyperdriveInterfaces() { return 'h'; }
Q_DECL_CONSTEXPR quint8 hyperdriveHasInterface() { return 'z'; }
Q_DECL_CONSTEXPR quint8 transportsTemplateUrls() { return '8'; }
Q_DECL_CONSTEXPR quint8 introspection() { return 'i'; }
Q_DECL_CONSTEXPR quint8 cacheMessage() { return 'k'; }
Q_DECL_CONSTEXPR quint8 wave() { return 'w'; }
Q_DECL_CONSTEXPR quint8 rebound() { return 'r'; }
Q_DECL_CONSTEXPR quint8 fluctuation() { return 'f'; }
Q_DECL_CONSTEXPR quint8 interfaces() { return 'c'; }
Q_DECL_CONSTEXPR quint8 messageTerminator() { return 'T'; }
Q_DECL_CONSTEXPR quint8 bigBang() { return 'B'; }
}

namespace Discovery
{
Q_DECL_CONSTEXPR quint8 interfacesRegistered() { return '+'; }
Q_DECL_CONSTEXPR quint8 interfacesUnregistered() { return '-'; }
}

inline static QByteArray generateSecret() {
    return QUuid::createUuid().toByteArray();
}

}

}

#endif // HYPERDRIVE_PROTOCOL_H
