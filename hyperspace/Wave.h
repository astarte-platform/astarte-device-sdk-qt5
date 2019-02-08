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

#ifndef HYPERSPACE_WAVE_H
#define HYPERSPACE_WAVE_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>

#include <HyperspaceCore/Global>

namespace Hyperspace {

class WaveData;
/**
 * @class Wave
 * @ingroup HyperspaceCore
 * @headerfile HyperspaceCore/hyperspaceglobal.h <HyperspaceCore/Global>
 *
 * @brief The base class data structure for Waves.
 *
 * Wave is the base data structure which is used for representing and serializing waves.
 * It encloses every Wave field, and is easily serializable and deserializable through
 * Data Streams.
 *
 * @sa Hyperspace::Rebound
 */
class HYPERSPACE_QT5_EXPORT Wave {
public:
    /// Creates an empty wave with a new unique UUID
    Wave();
    Wave(const Wave &other);
    ~Wave();

    Wave &operator=(const Wave &rhs);
    bool operator==(const Wave &other) const;
    inline bool operator!=(const Wave &other) const { return !operator==(other); }

    /// @returns The wave's id.
    quint64 id() const;
    void setId(quint64 id);

    /// The wave's method, as a string.
    QByteArray method() const;
    void setMethod(const QByteArray &m);

    /// The wave's interface
    QByteArray interface() const;
    void setInterface(const QByteArray &i);

    /// The wave's target, as a full absolute path.
    QByteArray target() const;
    void setTarget(const QByteArray &t);

    /// The wave's attributes.
    ByteArrayHash attributes() const;
    void setAttributes(const ByteArrayHash &attributes);
    void addAttribute(const QByteArray &attribute, const QByteArray &value);
    bool removeAttribute(const QByteArray &attribute);
    QByteArray takeAttribute(const QByteArray &attribute);

    /// The wave's payload.
    QByteArray payload() const;
    void setPayload(const QByteArray &p);

    QByteArray serialize() const;
    static Wave fromBinary(const QByteArray &data);

private:
    Wave(quint64 id);

    QSharedDataPointer<WaveData> d;
};

}

#endif // HYPERSPACE_WAVE_H
