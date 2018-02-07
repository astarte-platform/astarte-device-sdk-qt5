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

#ifndef HYPERSPACE_FLUCTUATION_H
#define HYPERSPACE_FLUCTUATION_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>

#include <HyperspaceCore/Global>

namespace Hyperspace {

class FluctuationData;
/**
 * @class Fluctuation
 * @ingroup HyperspaceCore
 * @headerfile HyperspaceCore/hyperspaceglobal.h <HyperspaceCore/Global>
 *
 * @brief The base class data structure for Fluctuations.
 *
 * Fluctuation represent an instant change in a Target's internal representation. It might or might not
 * carry with it the value changed, which might be partial or not.
 *
 * Differently from Waves and Rebounds, it has no ID and no return value. It is equivalent to a
 * multicast transmission to the whole accessible Hyperspace network that a Target has changed.
 *
 * Not every transport supports Fluctuations, however several transports (MQTT) rely heavily on
 * it. It is hence advised to support Fluctuations wherever possible in your targets, or use a
 * higher-level class which inherently supports Fluctuations (e.g.: @ref Hyperspace::REST::PropertyResource)
 *
 * @sa Hyperspace::Wave
 */
class HYPERSPACE_QT5_EXPORT Fluctuation {
public:
    /**
     * @brief Constructs a Fluctuation
     */
    Fluctuation();
    Fluctuation(const Fluctuation &other);
    ~Fluctuation();

    Fluctuation &operator=(const Fluctuation &rhs);
    bool operator==(const Fluctuation &other) const;
    inline bool operator!=(const Fluctuation &other) const { return !operator==(other); }

    QByteArray interface() const;
    void setInterface(const QByteArray &i);

    /// The target emitting the Fluctuation
    QByteArray target() const;
    void setTarget(const QByteArray &t);

    /// The Fluctuation's payload, if any. It might represent a total or partial change to its internal representation.
    QByteArray payload() const;
    void setPayload(const QByteArray &p);

    QHash<QByteArray, QByteArray> attributes() const;
    void setAttributes(const QHash<QByteArray, QByteArray> &attributes);
    void addAttribute(const QByteArray &attribute, const QByteArray &value);
    bool removeAttribute(const QByteArray &attribute);
    QByteArray takeAttribute(const QByteArray &attribute);

    QByteArray serialize() const;
    static Fluctuation fromBinary(const QByteArray &data);

private:
    QSharedDataPointer<FluctuationData> d;
};

}

#endif // HYPERSPACE_FLUCTUATION_H
