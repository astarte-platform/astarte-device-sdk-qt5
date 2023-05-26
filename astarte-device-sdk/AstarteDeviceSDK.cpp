/*
 * This file is part of Astarte.
 *
 * Copyright 2017-2021 Ispirata Srl
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

#include "AstarteDeviceSDK.h"
#include "Utils.h"

#include <astartetransport.h>

#include <hyperdriveconfig.h>
#include <hyperdriveinterface.h>

#include <hemerafakehardwareidoperation.h>
#include <HemeraCore/Literals>
#include <HemeraCore/Operation>

#include <AstarteGenericConsumer.h>
#include <AstarteGenericProducer.h>
#include <QJsonSchemaChecker.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(astarteDeviceSDKDC, "astarte-device-sdk", DEBUG_MESSAGES_DEFAULT_LEVEL)

AstarteDeviceSDK::AstarteDeviceSDK(const QString &configurationPath, const QString &interfacesDir,
                                   const QByteArray &hardwareId, QObject *parent)
    : Hemera::AsyncInitObject(parent)
    , m_hardwareId(hardwareId)
    , m_checker(new QJsonSchemaChecker())
    , m_configurationPath(configurationPath)
    , m_interfacesDir(interfacesDir)
{
}

AstarteDeviceSDK::~AstarteDeviceSDK()
{
}

void AstarteDeviceSDK::initImpl()
{
    int decodedLen = QByteArray::fromBase64(m_hardwareId, QByteArray::Base64UrlEncoding).count();
    if (decodedLen != 16) {
        setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::badRequest()), QStringLiteral("Invalid hardware ID"));
        qCWarning(astarteDeviceSDKDC) << "Invalid device ID: " << m_hardwareId << ", decoded len: " << decodedLen;
        return;
    }
    Hemera::FakeHardwareIDOperation::setHardwareId(m_hardwareId);

    m_astarteTransport = new Hyperdrive::AstarteTransport(m_configurationPath, this);
    connect(m_astarteTransport->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op) {
        if (op->isError()) {
            setInitError(op->errorName(), op->errorMessage());
        } else {
            setReady();
        }
    });
    connect(m_astarteTransport, &Hyperdrive::AstarteTransport::connectionStatusChanged, this, &AstarteDeviceSDK::connectionStatusChanged);

    QFile schemaFile(QStringLiteral("%1/interface.json").arg(
                QLatin1String(Hyperdrive::StaticConfig::transportAstarteDataDir())));
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::notFound()),
                     QStringLiteral("Schema file %1 does not exist").arg(schemaFile.fileName()));
        return;
    }

    QJsonDocument schemaJson = QJsonDocument::fromJson(schemaFile.readAll());
    if (!schemaJson.isObject()) {
        setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                     QStringLiteral("Schema file %1 does not contain a JSON object").arg(schemaFile.fileName()));
        return;
    }

    m_checker->setSchema(schemaJson.object());

    loadInterfaces();
}

void AstarteDeviceSDK::loadInterfaces()
{
    QHash< QByteArray, Hyperdrive::Interface > introspection;

    QDir interfacesDirectory(m_interfacesDir);
    interfacesDirectory.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    interfacesDirectory.setNameFilters(QStringList { QStringLiteral("*.json") } );

    QList<QFileInfo> interfaceFiles = interfacesDirectory.entryInfoList();

    for (const QFileInfo &fileInfo : interfaceFiles) {
        QString filePath = fileInfo.absoluteFilePath();
        qCDebug(astarteDeviceSDKDC) << "Loading interface " << filePath;

        QFile interfaceFile(filePath);
        if (!interfaceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCWarning(astarteDeviceSDKDC) << "Error opening interface file " << filePath;
            continue;
        }
        QJsonDocument interfaceJson = QJsonDocument::fromJson(interfaceFile.readAll());
        if (!interfaceJson.isObject()) {
            qCWarning(astarteDeviceSDKDC) << "interface file " << filePath << " doesn't contain a JSON object";
            continue;
        }

        QJsonObject interfaceObject = interfaceJson.object();

        if (m_checker->validate(interfaceObject)) {
            Hyperdrive::Interface interface = Hyperdrive::Interface::fromJson(interfaceJson.object());
            introspection.insert(interface.interface(), interface);
            switch (interface.interfaceQuality()) {
                case Hyperdrive::Interface::Quality::Producer:
                    createProducer(interface, interfaceObject);
                    break;
                case Hyperdrive::Interface::Quality::Consumer:
                    createConsumer(interface, interfaceObject);
                    break;
                default:
                    qCWarning(astarteDeviceSDKDC) << "Invalid interface quality";
            }
            qCDebug(astarteDeviceSDKDC) << "Interface loaded " << interface.interface();
        } else {
            qCWarning(astarteDeviceSDKDC) << "Error loading interface " << filePath
                                          << ": " << m_checker->getMessages().join(QStringLiteral("\n"))
                                          << ". Skipping it";
        }
    }

    m_astarteTransport->setIntrospection(introspection);
}

void AstarteDeviceSDK::createConsumer(const Hyperdrive::Interface &interface, const QJsonObject &consumerObject)
{
    QHash<QByteArray, QByteArrayList> mappingToTokens;
    QHash<QByteArray, QVariant::Type> mappingToType;
    QHash<QByteArray, QVariant::Type> mappingToArrayType;
    QHash<QByteArray, bool> mappingToAllowUnset;

    for (const QJsonValue &value : consumerObject.value(QStringLiteral("mappings")).toArray()) {
        QJsonObject mappingObj = value.toObject();

        QByteArray endpoint = mappingObj.value(QStringLiteral("endpoint")).toString().toLatin1();
        QByteArrayList tokens = endpoint.mid(1).split('/');
        mappingToTokens.insert(endpoint, tokens);

        QString typeString = mappingObj.value(QStringLiteral("type")).toString();

        QPair<EndpointType, QVariant::Type> ty = typeStringToVariantType(typeString);
        switch (ty.first) {
            case EndpointType::AstarteScalarType:
                mappingToType.insert(endpoint, ty.second);
                break;

            case EndpointType::AstarteArrayType:
                mappingToArrayType.insert(endpoint, ty.second);
                break;
        }

        if (interface.interfaceType() == Hyperdrive::Interface::Type::Properties && mappingObj.contains(QStringLiteral("allow_unset"))) {
            bool allowUnset = mappingObj.value(QStringLiteral("allow_unset")).toBool();
            mappingToAllowUnset.insert(endpoint, allowUnset);
        }
    }

    AstarteGenericConsumer *consumer = new AstarteGenericConsumer(interface.interface(),
        interface.interfaceType(), interface.interfaceAggregation(), m_astarteTransport, this);
    consumer->setMappingToTokens(mappingToTokens);
    consumer->setMappingToType(mappingToType);
    consumer->setMappingToArrayType(mappingToArrayType);
    consumer->setMappingToAllowUnset(mappingToAllowUnset);

    m_consumers.insert(interface.interface(), consumer);
    qCDebug(astarteDeviceSDKDC) << "Consumer for interface " << interface.interface() << " successfully initialized";
}

void AstarteDeviceSDK::createProducer(const Hyperdrive::Interface &interface, const QJsonObject &producerObject)
{
    QHash<QByteArray, QByteArrayList> mappingToTokens;
    QHash<QByteArray, QVariant::Type> mappingToType;
    QHash<QByteArray, QVariant::Type> mappingToArrayType;
    QHash<QByteArray, Hyperspace::Retention> mappingToRetention;
    QHash<QByteArray, Hyperspace::Reliability> mappingToReliability;
    QHash<QByteArray, int> mappingToExpiry;
    QHash<QByteArray, bool> mappingToAllowUnset;

    for (const QJsonValue &value : producerObject.value(QStringLiteral("mappings")).toArray()) {
        QJsonObject mappingObj = value.toObject();

        QByteArray endpoint = mappingObj.value(QStringLiteral("endpoint")).toString().toLatin1();
        QByteArrayList tokens = endpoint.mid(1).split('/');
        mappingToTokens.insert(endpoint, tokens);

        QString typeString = mappingObj.value(QStringLiteral("type")).toString();
        QPair<EndpointType, QVariant::Type> ty = typeStringToVariantType(typeString);

        switch (ty.first) {
            case EndpointType::AstarteScalarType:
                mappingToType.insert(endpoint, ty.second);
                break;
            case EndpointType::AstarteArrayType:
                mappingToArrayType.insert(endpoint, ty.second);
                break;
        }

        if (interface.interfaceType() == Hyperdrive::Interface::Type::DataStream) {
            if (mappingObj.contains(QStringLiteral("retention"))) {
                QString retention = mappingObj.value(QStringLiteral("retention")).toString();
                mappingToRetention.insert(endpoint, retentionStringToRetention(retention));
            }
            if (mappingObj.contains(QStringLiteral("reliability"))) {
                QString reliability = mappingObj.value(QStringLiteral("reliability")).toString();
                mappingToReliability.insert(endpoint, reliabilityStringToReliability(reliability));
            }
            if (mappingObj.contains(QStringLiteral("expiry"))) {
                int expiry = mappingObj.value(QStringLiteral("expiry")).toInt();
                mappingToExpiry.insert(endpoint, expiry);
            }
        } else if (interface.interfaceType() == Hyperdrive::Interface::Type::Properties && mappingObj.contains(QStringLiteral("allow_unset"))) {
            bool allowUnset = mappingObj.value(QStringLiteral("allow_unset")).toBool();
            mappingToAllowUnset.insert(endpoint, allowUnset);
        }
    }

    AstarteGenericProducer *producer = new AstarteGenericProducer(interface.interface(), interface.interfaceType(),
                                                                  m_astarteTransport, this);
    producer->setMappingToTokens(mappingToTokens);
    producer->setMappingToType(mappingToType);
    producer->setMappingToArrayType(mappingToArrayType);
    producer->setMappingToRetention(mappingToRetention);
    producer->setMappingToReliability(mappingToReliability);
    producer->setMappingToExpiry(mappingToExpiry);
    producer->setMappingToAllowUnset(mappingToAllowUnset);

    m_producers.insert(interface.interface(), producer);
    qCDebug(astarteDeviceSDKDC) << "Producer for interface " << interface.interface() << " successfully initialized";
}


QPair<EndpointType, QVariant::Type> AstarteDeviceSDK::typeStringToVariantType(const QString &typeString) const
{
    if (typeString == QStringLiteral("integer")) {
        return  { EndpointType::AstarteScalarType, QVariant::Int };
    } else if (typeString == QStringLiteral("longinteger")) {
        return  { EndpointType::AstarteScalarType, QVariant::LongLong };
    } else if (typeString == QStringLiteral("double")) {
        return  { EndpointType::AstarteScalarType, QVariant::Double };
    } else if (typeString == QStringLiteral("datetime")) {
        return  { EndpointType::AstarteScalarType, QVariant::DateTime };
    } else if (typeString == QStringLiteral("string")) {
        return  { EndpointType::AstarteScalarType, QVariant::String };
    } else if (typeString == QStringLiteral("boolean")) {
        return  { EndpointType::AstarteScalarType, QVariant::Bool };
    } else if (typeString == QStringLiteral("binaryblob")) {
        return  { EndpointType::AstarteScalarType, QVariant::ByteArray };
    } else if (typeString == QStringLiteral("integerarray")) {
        return  { EndpointType::AstarteArrayType, QVariant::Int };
    } else if (typeString == QStringLiteral("longintegerarray")) {
        return  { EndpointType::AstarteArrayType, QVariant::LongLong };
    } else if (typeString == QStringLiteral("doublearray")) {
        return  { EndpointType::AstarteArrayType, QVariant::Double };
    } else if (typeString == QStringLiteral("datetimearray")) {
        return  { EndpointType::AstarteArrayType, QVariant::DateTime };
    } else if (typeString == QStringLiteral("stringarray")) {
        return  { EndpointType::AstarteArrayType, QVariant::String };
    } else if (typeString == QStringLiteral("booleanarray")) {
        return  { EndpointType::AstarteArrayType, QVariant::Bool };
    } else if (typeString == QStringLiteral("binaryblobarray")) {
        return  { EndpointType::AstarteArrayType, QVariant::ByteArray };
    } else {
        qCWarning(astarteDeviceSDKDC) << QStringLiteral("Type %1 unspecified!").arg(typeString);
        return  { EndpointType::AstarteScalarType, QVariant::Invalid };
    }
}

Hyperspace::Retention AstarteDeviceSDK::retentionStringToRetention(const QString &retentionString) const
{
    if (retentionString == QStringLiteral("stored")) {
        return Hyperspace::Retention::Stored;
    } else if (retentionString == QStringLiteral("volatile")) {
        return Hyperspace::Retention::Volatile;
    } else {
        return Hyperspace::Retention::Discard;
    }
}

Hyperspace::Reliability AstarteDeviceSDK::reliabilityStringToReliability(const QString &reliabilityString) const
{
    if (reliabilityString == QStringLiteral("unique")) {
        return Hyperspace::Reliability::Unique;
    } else if (reliabilityString == QStringLiteral("guaranteed")) {
        return Hyperspace::Reliability::Guaranteed;
    } else {
        return Hyperspace::Reliability::Unreliable;
    }
}

bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QVariant &value, const QDateTime &timestamp, const QVariantHash &metadata)
{
    if (!m_producers.contains(interface)) {
        qCWarning(astarteDeviceSDKDC) << "No producers for interface " << interface;
        return false;
    }

    return m_producers.value(interface)->sendData(value, path, timestamp, metadata);
}

bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QVariant &value, const QVariantHash &metadata)
{
    return sendData(interface, path, value, QDateTime(), metadata);
}

bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QVariantHash &value, const QVariantHash &metadata)
{
    return sendData(interface, value, QDateTime(), metadata);
}

bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QVariantHash &value, const QDateTime &timestamp,
                                const QVariantHash &metadata)
{
    if (!m_producers.contains(interface)) {
        qCWarning(astarteDeviceSDKDC) << "No producers for interface " << interface;
        return false;
    }
    // Verify mappings
    QHash< QByteArray, QVariant::Type > mappingToType = m_producers.value(interface)->mappingToType();
    QHash< QByteArray, QVariant::Type > mappingToArrayType = m_producers.value(interface)->mappingToArrayType();

    if ((mappingToType.size() + mappingToArrayType.size()) != value.size()) {
        qCWarning(astarteDeviceSDKDC) << "You have to provide exactly all the values of the aggregated interface!";
        return false;
    }

    QStringList pathTokens = value.constBegin().key().mid(1).split(QStringLiteral("/"));
    pathTokens.removeLast();

    QString trailingPath;
    if (pathTokens.size() > 0) {
        trailingPath = QStringLiteral("/%1").arg(pathTokens.join(QStringLiteral("/")));
    }

    // There's a number of checks we have to do. Let's build a map first
    QHash<QByteArray, QByteArrayList> tokens;
    for (QVariantHash::const_iterator i = value.constBegin(); i != value.constEnd(); ++i) {
        QByteArrayList singleTokens;
        for (const QString &token : i.key().mid(1).split(QStringLiteral("/"))) {
            singleTokens.append(token.toLatin1());
        }
        tokens.insert(i.key().toLatin1(), singleTokens);
    }
    if (!Utils::verifySplitTokenMatch(tokens, m_producers.value(interface)->mappingToTokens())) {
        qCWarning(astarteDeviceSDKDC) << "Provided hash does not match interface definition!";
        return false;
    }

    QVariantHash normalizedValues;
    for (QVariantHash::const_iterator i = value.constBegin(); i != value.constEnd(); ++i) {
        // TODO: check that types match wath we expect
        if (!trailingPath.isEmpty() && !i.key().startsWith(trailingPath)) {
            qCWarning(astarteDeviceSDKDC) << "Your path is malformed - this probably means you mistyped your parameters." << i.key() << "was expected to start with" << trailingPath;
            return false;
        }
        QString normalizedPath = i.key();
        normalizedPath.remove(0, trailingPath.size() + 1);
        normalizedValues.insert(normalizedPath, i.value());

        QVariant value = i.value();

        if (value.type() == QVariant::List){
            QList<QVariant> valueList = value.toList();
            for (int j = 0; j < valueList.length(); j++) {
                if (valueList.at(j).type() != valueList.at(0).type()) {
                    qCWarning(astarteDeviceSDKDC) << "Inconsistent types in QVariant array";
                    return false;
                }
            }
        }

    }
    // If we got here, verification was ok. Let's go.

    return m_producers.value(interface)->sendData(normalizedValues, trailingPath.toLatin1(), timestamp, metadata);
}

bool AstarteDeviceSDK::sendUnset(const QByteArray &interface, const QByteArray &path)
{
    if (!m_producers.contains(interface)) {
        qCWarning(astarteDeviceSDKDC) << "No producers for interface " << interface;
        return false;
    }

    return m_producers.value(interface)->unsetPath(path);
}

void AstarteDeviceSDK::unsetValue(const QByteArray &interface, const QByteArray &path)
{
    Q_EMIT unsetReceived(interface, path);
}

void AstarteDeviceSDK::receiveValue(const QByteArray &interface, const QByteArray &path, const QVariant &value)
{
    Q_EMIT dataReceived(interface, path, value);
}

AstarteDeviceSDK::ConnectionStatus AstarteDeviceSDK::connectionStatus() const
{
    if (!m_astarteTransport) {
        return AstarteDeviceSDK::DisconnectedStatus;
    }

    return static_cast<AstarteDeviceSDK::ConnectionStatus>(m_astarteTransport->connectionStatus());
}

bool AstarteDeviceSDK::connectToAstarte()
{
  if (!m_astarteTransport) {
    qCDebug(astarteDeviceSDKDC) << "Not yet initialized, cannot connect to broker.";
    return false;
  }

  return m_astarteTransport->connectToBroker();
}

bool AstarteDeviceSDK::disconnectFromAstarte()
{
  if (!m_astarteTransport) {
    qCDebug(astarteDeviceSDKDC) << "Not initialized, cannot disconnect from broker.";
    return false;
  }

  return m_astarteTransport->disconnectFromBroker();
}

template <typename T> bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path,
                                                      const QList<T> &valueList, const QDateTime &timestamp, const QVariantHash &metadata)
{
    if (!m_producers.contains(interface)) {
        qCWarning(astarteDeviceSDKDC) << "No producers for interface " << interface;
        return false;
    }

    QList<QVariant> variantValue;
    variantValue.reserve(valueList.length());

    for (int i = 0; i < valueList.length(); i++){
        variantValue.append(QVariant(valueList[i]));
    }

    return m_producers.value(interface)->sendData(QVariant(variantValue), path, timestamp, metadata);
}

template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<QByteArray> &value, const QDateTime &timestamp, const QVariantHash &metadata);
template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<int> &value, const QDateTime &timestamp, const QVariantHash &metadata);
template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<qlonglong> &value, const QDateTime &timestamp, const QVariantHash &metadata);
template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<double> &value, const QDateTime &timestamp, const QVariantHash &metadata);
template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<bool> &value, const QDateTime &timestamp, const QVariantHash &metadata);
template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<QDateTime> &value, const QDateTime &timestamp, const QVariantHash &metadata);
template bool AstarteDeviceSDK::sendData(const QByteArray &interface, const QByteArray &path, const QList<QString> &value, const QDateTime &timestamp, const QVariantHash &metadata);
