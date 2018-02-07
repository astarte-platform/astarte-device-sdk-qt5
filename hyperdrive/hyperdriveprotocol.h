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
