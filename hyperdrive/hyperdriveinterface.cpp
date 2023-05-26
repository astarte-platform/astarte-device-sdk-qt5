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

#include "hyperdriveinterface.h"

#include <QtCore/QSharedData>
#include <QtCore/QJsonObject>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(hyperdriveInterfaceDC, "hyperdrive.interface", DEBUG_MESSAGES_DEFAULT_LEVEL)

namespace Hyperdrive {

class InterfaceData : public QSharedData
{
public:
    InterfaceData() : interfaceType(Hyperdrive::Interface::Type::Unknown), interfaceQuality(Hyperdrive::Interface::Quality::Unknown), interfaceAggregation(Hyperdrive::Interface::Aggregation::Unknown) { }
    InterfaceData(const QByteArray &interface, int versionMajor, int versionMinor, Interface::Type interfaceType, Interface::Quality interfaceQuality, Interface::Aggregation interfaceAggregation)
        : interface(interface), versionMajor(versionMajor), versionMinor(versionMinor), interfaceType(interfaceType)
        , interfaceQuality(interfaceQuality) , interfaceAggregation(interfaceAggregation) { }
    InterfaceData(const InterfaceData &other)
        : QSharedData(other), interface(other.interface), versionMajor(other.versionMajor), versionMinor(other.versionMinor)
        , interfaceType(other.interfaceType), interfaceQuality(other.interfaceQuality), interfaceAggregation(other.interfaceAggregation) { }
    ~InterfaceData() { }

    QByteArray interface;
    int versionMajor;
    int versionMinor;
    Interface::Type interfaceType;
    Interface::Quality interfaceQuality;
    Interface::Aggregation interfaceAggregation;
};

Interface::Interface()
    : d(new InterfaceData())
{
}

Interface::Interface(const Interface &other)
    : d(other.d)
{
}

Interface::Interface(const QByteArray &interface, int versionMajor, int versionMinor, Type interfaceType, Quality interfaceQuality, Aggregation interfaceAggregation)
    : d(new InterfaceData(interface, versionMajor, versionMinor, interfaceType, interfaceQuality, interfaceAggregation))
{
}

Interface::~Interface()
{
}

Interface& Interface::operator=(const Interface &rhs)
{
    if (this == &rhs) {
        // Protect against self-assignment
        return *this;
    }

    d = rhs.d;
    return *this;
}

bool Interface::operator==(const Interface &other) const
{
    return d->interface == other.interface() && d->versionMajor == other.versionMajor() && d->versionMinor == other.versionMinor()
           && d->interfaceType == other.interfaceType() && d->interfaceQuality == other.interfaceQuality();
}

QByteArray Interface::interface() const
{
    return d->interface;
}

void Interface::setInterface(const QByteArray &i)
{
    d->interface = i;
}

int Interface::versionMajor() const
{
    return d->versionMajor;
}

void Interface::setVersionMajor(int v)
{
    d->versionMajor = v;
}

int Interface::versionMinor() const
{
    return d->versionMinor;
}

void Interface::setVersionMinor(int v)
{
    d->versionMinor = v;
}

Interface::Type Interface::interfaceType() const
{
    return d->interfaceType;
}

void Interface::setInterfaceType(Type t)
{
    d->interfaceType = t;
}

Interface::Quality Interface::interfaceQuality() const
{
    return d->interfaceQuality;
}

void Interface::setInterfaceQuality(Quality q)
{
    d->interfaceQuality = q;
}

Interface::Aggregation Interface::interfaceAggregation() const
{
    return d->interfaceAggregation;
}

void Interface::setInterfaceAggregation(Aggregation a)
{
    d->interfaceAggregation = a;
}


bool Interface::isValid() const
{
    return !d->interface.isEmpty();
}

Interface Interface::fromJson(const QJsonObject &jsonObj)
{
    QByteArray interface;
    if (jsonObj.contains(QStringLiteral("interface_name"))) {
        interface = jsonObj.value(QStringLiteral("interface_name")).toString().toLatin1();
    } else if (jsonObj.contains(QStringLiteral("interface"))) {
        interface = jsonObj.value(QStringLiteral("interface")).toString().toLatin1();
        qCWarning(hyperdriveInterfaceDC) << "interface is deprecated, use interface_name";
    } else {
        qCWarning(hyperdriveInterfaceDC) << "interface_name missing in Interface JSON object";
        return Interface();
    }

    int versionMajor;
    if (jsonObj.contains(QStringLiteral("version_major"))) {
        versionMajor = jsonObj.value(QStringLiteral("version_major")).toInt();
    } else {
        qCWarning(hyperdriveInterfaceDC) << "version_major missing in Interface JSON object for interface " << interface;
        return Interface();
    }

    int versionMinor;
    if (jsonObj.contains(QStringLiteral("version_minor"))) {
        versionMinor = jsonObj.value(QStringLiteral("version_minor")).toInt();
    } else {
        qCWarning(hyperdriveInterfaceDC) << "version_minor missing in Interface JSON object for interface " << interface;
        return Interface();
    }

    Type interfaceType;
    if (jsonObj.contains(QStringLiteral("type"))) {
        QString typeString = jsonObj.value(QStringLiteral("type")).toString();
        if (typeString == QStringLiteral("datastream")) {
            interfaceType = Interface::Type::DataStream;
        } else if (typeString == QStringLiteral("properties")) {
            interfaceType = Interface::Type::Properties;
        } else {
            qCWarning(hyperdriveInterfaceDC) << "Invalid type in Interface JSON object for interface " << interface;
            return Interface();
        }
    } else {
        qCWarning(hyperdriveInterfaceDC) << "type missing in Interface JSON object for interface " << interface;
        return Interface();
    }

    Quality interfaceQuality;
    if (jsonObj.contains(QStringLiteral("ownership"))) {
        QString ownershipString = jsonObj.value(QStringLiteral("ownership")).toString();
        if (ownershipString == QStringLiteral("device")) {
            interfaceQuality = Interface::Quality::Producer;
        } else if (ownershipString == QStringLiteral("server")) {
            interfaceQuality = Interface::Quality::Consumer;
        } else {
            qCWarning(hyperdriveInterfaceDC) << "Invalid ownership in Interface JSON object for interface " << interface;
            return Interface();
        }
    } else if (jsonObj.contains(QStringLiteral("quality"))) {
        qCWarning(hyperdriveInterfaceDC) << "quality is deprecated, use \"ownership\": \"device\"/\"server\"";
        QString qualityString = jsonObj.value(QStringLiteral("quality")).toString();
        if (qualityString == QStringLiteral("producer")) {
            interfaceQuality = Interface::Quality::Producer;
        } else if (qualityString == QStringLiteral("consumer")) {
            interfaceQuality = Interface::Quality::Consumer;
        } else {
            qCWarning(hyperdriveInterfaceDC) << "Invalid quality in Interface JSON object for interface " << interface;
            return Interface();
        }
    } else {
        qCWarning(hyperdriveInterfaceDC) << "ownership missing in Interface JSON object for interface " << interface;
        return Interface();
    }

    Aggregation interfaceAggregation;
    if (jsonObj.contains(QStringLiteral("aggregation"))) {
        QString AggregationString = jsonObj.value(QStringLiteral("aggregation")).toString();
        if (AggregationString == QStringLiteral("object")) {
            interfaceAggregation = Interface::Aggregation::Object;
        } else {
            interfaceAggregation = Interface::Aggregation::Individual;
        }
    } else {
        interfaceAggregation = Interface::Aggregation::Individual;
    }

    return Interface(interface, versionMajor, versionMinor, interfaceType, interfaceQuality, interfaceAggregation);
}

}
