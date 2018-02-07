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

#ifndef HYPERSPACE_GLOBAL_H
#define HYPERSPACE_GLOBAL_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>

#include <HemeraCore/Global>

#include <QtCore/QtGlobal>

#ifdef BUILDING_HYPERSPACE_QT5
#  define HYPERSPACE_QT5_EXPORT Q_DECL_EXPORT
#else
#  define HYPERSPACE_QT5_EXPORT Q_DECL_IMPORT
#endif

#if !defined(Q_OS_WIN) && defined(QT_VISIBILITY_AVAILABLE)
#  define HYPERSPACE_QT5_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#ifndef HYPERSPACE_QT5_NO_EXPORT
#  define HYPERSPACE_QT5_NO_EXPORT
#endif

namespace Hyperspace {

typedef QHash<QByteArray, QByteArray> ByteArrayHash;

/**
 * @enum ResponseCode
 * @ingroup HyperspaceCore
 * @headerfile HyperspaceCore/hyperspaceglobal.h <HyperspaceCore/Global>*
 *
 * @brief Possible response codes for Rebounds
 *
 * This enum replicates the HTTP specification for return codes.
 */
enum class ResponseCode : quint16
{
    InvalidCode = 0,
    OK = 200,
    Created = 201,
    Accepted = 202,
    PartialInformation = 203,
    NoResponse = 204,
    Moved = 301,
    Method = 303,
    NotModified = 304,
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    InternalError = 500,
    NotImplemented = 501,
    ServiceUnavailable = 503,
};

namespace Protocol
{

enum class Type : quint8 {
    Unknown = 0,
    Control = 1,
    Discovery = 2,
};

enum class MessageType
{
    Invalid = 0,
    Wave = 1,
    Rebound = 2,
    Fluctuation = 3,
    Waveguide = 4
};

namespace Discovery
{
Q_DECL_CONSTEXPR quint8 announced() { return 'a'; }
Q_DECL_CONSTEXPR quint8 changed() { return 'm'; }
Q_DECL_CONSTEXPR quint8 expired() { return 'e'; }
Q_DECL_CONSTEXPR quint8 introspectRequest() { return 'i'; }
Q_DECL_CONSTEXPR quint8 introspectReply() { return 'b'; }
Q_DECL_CONSTEXPR quint8 purged() { return 'p'; }
Q_DECL_CONSTEXPR quint8 scan() { return 's'; }
Q_DECL_CONSTEXPR quint8 subscribe() { return 't'; }
Q_DECL_CONSTEXPR quint8 unsubscribe() { return 'u'; }
}

Q_DECL_CONSTEXPR Type type(quint8 p) {
    /// This code looks ugly, but before you try to change it, remember it's a constexpr, and it has to be 1 line.
    return
        // Case Discovery
        p == Discovery::announced() || p == Discovery::changed() || p == Discovery::expired() || p == Discovery::introspectReply() ||
        p == Discovery::introspectRequest() || p == Discovery::purged() || p == Discovery::scan() ||
        p == Discovery::subscribe() || p == Discovery::unsubscribe() ? Type::Discovery :
        // Case Unknown (default)
        Type::Unknown;
}

}

}

#endif // HYPERSPACE_NETWORK_GLOBAL_H
