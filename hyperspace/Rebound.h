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
