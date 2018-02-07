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

#ifndef HYPERSPACE_REBOUND_H
#define HYPERSPACE_REBOUND_H

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>

#include <HyperspaceCore/Global>
#include <HyperspaceCore/Wave>

namespace Hyperspace {

class ReboundData;

class Rebound
{
public:
    /// Creates an empty wave with a new unique UUID
    /**
     * @brief Constructs a Rebound from a Wave ID
     *
     * @p waveId The ID of the Wave associated to this Rebound.
     * @p code The response code of this Rebound.
     */
    Rebound(quint64 waveId, ResponseCode code = ResponseCode::InvalidCode);
    /**
     * @brief Constructs a Rebound from a Wave
     *
     * @p waveId The Wave associated to this Rebound.
     * @p code The response code of this Rebound.
     */
    Rebound(const Wave &wave, ResponseCode code = ResponseCode::InvalidCode);
    Rebound(const Rebound &other);
    ~Rebound();

    Rebound &operator=(const Rebound &rhs);
    bool operator==(const Rebound &other) const;
    inline bool operator!=(const Rebound &other) const { return !operator==(other); }

    /// @returns The rebound's id.
    quint64 id() const;
    void setId(quint64 id);

    /// The Rebound's response code.
    ResponseCode response() const;
    void setResponse(ResponseCode r);

    /// The Rebound's attributes.
    ByteArrayHash attributes() const;
    void setAttributes(const ByteArrayHash &attributes);
    void addAttribute(const QByteArray &attribute, const QByteArray &value);
    bool removeAttribute(const QByteArray &attribute);

    /// The Rebound's payload, if any.
    QByteArray payload() const;
    void setPayload(const QByteArray &p);

    QByteArray serialize() const;
    static Rebound fromBinary(const QByteArray &data);

private:
    QSharedDataPointer<ReboundData> d;
};

}

#endif // HYPERSPACE_REBOUND_H
