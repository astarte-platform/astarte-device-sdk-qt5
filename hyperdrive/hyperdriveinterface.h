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

    enum class Aggregation : quint8 {
        Unknown = 0,
        Individual = 1,
        Object = 2,
    };

    Interface();
    Interface(const Interface &other);
    Interface(const QByteArray &interface, int versionMajor, int versionMinor, Type interfaceType, Quality interfaceQuality, Aggregation interfaceAggregation);
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

    Aggregation interfaceAggregation() const;
    void setInterfaceAggregation(Aggregation a);

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
    int interfaceAggregation;

    s >> interface >> versionMajor >> versionMinor >> interfaceType >> interfaceQuality >> interfaceAggregation;
    i.setInterface(interface);
    i.setVersionMajor(versionMajor);
    i.setVersionMinor(versionMinor);
    i.setInterfaceType(static_cast<Hyperdrive::Interface::Type>(interfaceType));
    i.setInterfaceQuality(static_cast<Hyperdrive::Interface::Quality>(interfaceQuality));
    i.setInterfaceAggregation(static_cast<Hyperdrive::Interface::Aggregation>(interfaceAggregation));

    return s;
}

inline QDataStream &operator<<(QDataStream &s, const Hyperdrive::Interface &i)
{
    return s << i.interface() << i.versionMajor() << i.versionMinor() << static_cast<int>(i.interfaceType()) << static_cast<int>(i.interfaceQuality());
}

#endif // HYPERDRIVE_INTERFACE_H
