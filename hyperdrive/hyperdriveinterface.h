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

#ifndef HYPERDRIVE_INTERFACE_H
#define HYPERDRIVE_INTERFACE_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QDataStream>

#include <HyperspaceCore/Global>

namespace Hyperdrive {

class InterfaceData;
class Interface
{
public:
    enum class Type : quint8 {
        Unknown = 0,
        DataStream = 1,
        Properties = 2,
    };

    enum class Quality : quint8 {
        Unknown = 0,
        Producer = 1,
        Consumer = 2,
    };

    Interface();
    Interface(const Interface &other);
    Interface(const QByteArray &interface, int versionMajor, int versionMinor, Type interfaceType, Quality interfaceQuality);
    ~Interface();

    static Interface fromJson(const QJsonObject &jsonObj);

    Interface &operator=(const Interface &rhs);
    bool operator==(const Interface &other) const;
    inline bool operator!=(const Interface &other) const { return !operator==(other); }

    QByteArray interface() const;
    void setInterface(const QByteArray &i);

    int versionMajor() const;
    void setVersionMajor(int v);

    int versionMinor() const;
    void setVersionMinor(int v);

    Type interfaceType() const;
    void setInterfaceType(Type t);

    Quality interfaceQuality() const;
    void setInterfaceQuality(Quality q);

    bool isValid() const;

private:
    QSharedDataPointer<InterfaceData> d;
};

}

inline QDataStream &operator>>(QDataStream &s, Hyperdrive::Interface &i)
{
    QByteArray interface;
    int versionMajor;
    int versionMinor;
    int interfaceType;
    int interfaceQuality;

    s >> interface >> versionMajor >> versionMinor >> interfaceType >> interfaceQuality;
    i.setInterface(interface);
    i.setVersionMajor(versionMajor);
    i.setVersionMinor(versionMinor);
    i.setInterfaceType(static_cast<Hyperdrive::Interface::Type>(interfaceType));
    i.setInterfaceQuality(static_cast<Hyperdrive::Interface::Quality>(interfaceQuality));

    return s;
}

inline QDataStream &operator<<(QDataStream &s, const Hyperdrive::Interface &i)
{
    return s << i.interface() << i.versionMajor() << i.versionMinor() << static_cast<int>(i.interfaceType()) << static_cast<int>(i.interfaceQuality());
}

#endif // HYPERDRIVE_INTERFACE_H
