/**

MIT License

Copyright (c) 2014 hyperion team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

// Utils-Jsonschema includes
#include "QJsonSchemaChecker.h"

#include <QtCore/QRegularExpression>

#include <cmath>

QJsonSchemaChecker::QJsonSchemaChecker()
{
    // empty
}

QJsonSchemaChecker::~QJsonSchemaChecker()
{
    // empty
}

bool QJsonSchemaChecker::setSchema(const QJsonObject & schema)
{
    _qSchema = schema;

    // TODO: check the schema

    return true;
}

bool QJsonSchemaChecker::validate(const QJsonObject & value, bool ignoreRequired)
{
    // initialize state
    _ignoreRequired = ignoreRequired;
    _error = false;
    _messages.clear();
    _currentPath.clear();
    _currentPath.push_back(QStringLiteral("[root]"));

    // validate
    validate(value, _qSchema);

    return !_error;
}

void QJsonSchemaChecker::validate(const QJsonValue & value, const QJsonObject &schema)
{
    // check the current json value
    for (QJsonObject::const_iterator i = schema.begin(); i != schema.end(); ++i)
    {
        QString attribute = i.key();
        const QJsonValue & attributeValue = *i;

        if (attribute == QStringLiteral("type"))
            checkType(value, attributeValue);
        else if (attribute == QStringLiteral("properties"))
        {
            if (value.isObject())
                checkProperties(value.toObject(), attributeValue.toObject());
            else
            {
                _error = true;
                setMessage(QStringLiteral("properties attribute is only valid for objects"));
                continue;
            }
        }
        else if (attribute == QStringLiteral("additionalProperties"))
        {
            if (value.isObject())
            {
                // ignore the properties which are handled by the properties attribute (if present)
                QStringList ignoredProperties;
                if (schema.contains(QStringLiteral("properties"))) {
                    const QJsonObject & props = schema[QStringLiteral("properties")].toObject();
                    ignoredProperties = props.keys();
                }

                checkAdditionalProperties(value.toObject(), attributeValue, ignoredProperties);
            }
            else
            {
                _error = true;
                setMessage(QStringLiteral("additional properties attribute is only valid for objects"));
                continue;
            }
        }
        else if (attribute == QStringLiteral("minimum"))
            checkMinimum(value, attributeValue);
        else if (attribute == QStringLiteral("maximum"))
            checkMaximum(value, attributeValue);
        else if (attribute == QStringLiteral("items"))
        {
            if (value.isArray())
                checkItems(value, attributeValue.toObject());
            else
            {
                _error = true;
                setMessage(QStringLiteral("items only valid for arrays"));
                continue;
            }
        }
        else if (attribute == QStringLiteral("minItems"))
            checkMinItems(value, attributeValue);
        else if (attribute == QStringLiteral("maxItems"))
            checkMaxItems(value, attributeValue);
        else if (attribute == QStringLiteral("uniqueItems"))
            checkUniqueItems(value, attributeValue);
        else if (attribute == QStringLiteral("enum"))
            checkEnum(value, attributeValue);
        else if (attribute == QStringLiteral("pattern"))
            checkPattern(value, attributeValue);
        else if (attribute == QStringLiteral("minLength"))
            checkMinLength(value, attributeValue);
        else if (attribute == QStringLiteral("maxLength"))
            checkMaxLength(value, attributeValue);
        else if (attribute == QStringLiteral("required"))
            checkRequired(value, attributeValue);
        else if (attribute == QStringLiteral("id"))
            ; // references have already been collected
        else if (attribute == QStringLiteral("title") || attribute == QStringLiteral("description")  || attribute == QStringLiteral("default") || attribute == QStringLiteral("format")
                || attribute == QStringLiteral("defaultProperties") || attribute == QStringLiteral("propertyOrder") || attribute == QStringLiteral("append")
                || attribute == QStringLiteral("step") || attribute == QStringLiteral("access") || attribute == QStringLiteral("options") || attribute == QStringLiteral("$schema"))
            ; // nothing to do.
        else
        {
            // no check function defined for this attribute
            _error = true;
            setMessage(QStringLiteral("No check function defined for attribute %1").arg(attribute));
            continue;
        }
    }
}

void QJsonSchemaChecker::setMessage(const QString & message)
{
    _messages.push_back(QStringLiteral("%1: %2").arg(_currentPath.join(QStringLiteral("")), message));
}

QStringList QJsonSchemaChecker::getMessages() const
{
    return _messages;
}

void QJsonSchemaChecker::checkType(const QJsonValue & value, const QJsonValue & schema)
{
    QString type = schema.toString();

    bool wrongType = false;
    if (type == QStringLiteral("string"))
        wrongType = !value.isString();
    else if (type == QStringLiteral("number"))
        wrongType = !value.isDouble();
    else if (type == QStringLiteral("integer"))
        wrongType = (std::rint(value.toDouble()) != value.toDouble());
    else if (type == QStringLiteral("double"))
        wrongType = !value.isDouble();
    else if (type == QStringLiteral("boolean"))
        wrongType = !value.isBool();
    else if (type == QStringLiteral("object"))
        wrongType = !value.isObject();
    else if (type == QStringLiteral("array"))
        wrongType = !value.isArray();
    else if (type == QStringLiteral("null"))
        wrongType = !value.isNull();
    else if (type == QStringLiteral("enum"))
        wrongType = !value.isString();
    else if (type == QStringLiteral("any"))
        wrongType = false;
    //	else
    //		assert(false);

    if (wrongType)
    {
        _error = true;
        setMessage(QStringLiteral("%1 expected").arg(type));
    }
}

void QJsonSchemaChecker::checkProperties(const QJsonObject & value, const QJsonObject & schema)
{
    for (QJsonObject::const_iterator i = schema.begin(); i != schema.end(); ++i)
    {
        QString property = i.key();

        const QJsonValue  & propertyValue = i.value();

        _currentPath.push_back(QStringLiteral(".%1").arg(property));

        if (value.contains(property))
        {
            validate(value[property], propertyValue.toObject());
        }

        _currentPath.pop_back();
    }
}

void QJsonSchemaChecker::checkAdditionalProperties(const QJsonObject & value, const QJsonValue & schema, const QStringList & ignoredProperties)
{
    for (QJsonObject::const_iterator i = value.begin(); i != value.end(); ++i)
    {
        QString property = i.key();
        if (std::find(ignoredProperties.begin(), ignoredProperties.end(), property) == ignoredProperties.end())
        {
            // property has no property definition. check against the definition for additional properties
            _currentPath.push_back(QStringLiteral(".%1").arg(property));
            if (schema.isBool())
            {
                if (schema.toBool() == false)
                {
                    _error = true;
                    setMessage(QStringLiteral("no schema definition"));
                }
            }
            else
            {
                validate(value[property].toObject(), schema.toObject());
            }
            _currentPath.pop_back();
        }
    }
}

void QJsonSchemaChecker::checkMinimum(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isDouble())
    {
        // only for numeric
        _error = true;
        setMessage(QStringLiteral("minimum check only for numeric fields"));
        return;
    }

    if (value.toDouble() < schema.toDouble())
    {
        _error = true;
        setMessage(QStringLiteral("value is too small (minimum=%1)").arg(schema.toDouble()));
    }
}

void QJsonSchemaChecker::checkMaximum(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isDouble())
    {
        // only for numeric
        _error = true;
        setMessage(QStringLiteral("maximum check only for numeric fields"));
        return;
    }

    if (value.toDouble() > schema.toDouble())
    {
        _error = true;
        setMessage(QStringLiteral("value is too large (maximum=%1)").arg(schema.toDouble()));
    }
}

void QJsonSchemaChecker::checkPattern(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isString())
    {
        // only for strings
        _error = true;
        setMessage(QStringLiteral("pattern check only for string fields"));
        return;
    }

    QRegularExpression re(schema.toString());

    if (!re.isValid()) {
        _error = true;
        setMessage(QStringLiteral("Invalid schema pattern=%1").arg(schema.toString()));
        return;
    }

    if (!re.match(value.toString()).hasMatch())
    {
        _error = true;
        setMessage(QStringLiteral("Value %1 does not match pattern %2").arg(value.toString(), schema.toString()));
    }
}

void QJsonSchemaChecker::checkMinLength(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isString())
    {
        // only for strings
        _error = true;
        setMessage(QStringLiteral("minLength check only for string fields"));
        return;
    }

    if (value.toString().length() < schema.toInt())
    {
        _error = true;
        setMessage(QStringLiteral("Value %1 is shorter than minLength %2").arg(value.toString(), schema.toInt()));
    }
}

void QJsonSchemaChecker::checkMaxLength(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isString())
    {
        // only for strings
        _error = true;
        setMessage(QStringLiteral("maxLength check only for string fields"));
        return;
    }

    if (value.toString().length() > schema.toInt())
    {
        _error = true;
        setMessage(QStringLiteral("Value %1 is longer than maxLength %2").arg(value.toString(), schema.toInt()));
    }
}

void QJsonSchemaChecker::checkItems(const QJsonValue & value, const QJsonObject & schema)
{
    if (!value.isArray())
    {
        // only for arrays
        _error = true;
        setMessage(QStringLiteral("items only valid for arrays"));
        return;
    }

    QJsonArray jArray = value.toArray();
    for(int i = 0; i < jArray.size(); ++i)
    {
        // validate each item
        _currentPath.push_back(QStringLiteral("[%1]").arg(i));
        validate(jArray[i], schema);
        _currentPath.pop_back();
    }
}

void QJsonSchemaChecker::checkMinItems(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isArray())
    {
        // only for arrays
        _error = true;
        setMessage(QStringLiteral("minItems only valid for arrays"));
        return;
    }

    int minimum = schema.toInt();

    QJsonArray jArray = value.toArray();
    if (static_cast<int>(jArray.size()) < minimum)
    {
        _error = true;
        setMessage(QStringLiteral("array is too small (minimum=%1)").arg(minimum));
    }
}

void QJsonSchemaChecker::checkMaxItems(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isArray())
    {
        // only for arrays
        _error = true;
        setMessage(QStringLiteral("maxItems only valid for arrays"));
        return;
    }

    int maximum = schema.toInt();

    QJsonArray jArray = value.toArray();
    if (static_cast<int>(jArray.size()) > maximum)
    {
        _error = true;
        setMessage(QStringLiteral("array is too large (maximum=%1)").arg(maximum));
    }
}

void QJsonSchemaChecker::checkUniqueItems(const QJsonValue & value, const QJsonValue & schema)
{
    if (!value.isArray())
    {
        // only for arrays
        _error = true;
        setMessage(QStringLiteral("uniqueItems only valid for arrays"));
        return;
    }

    if (schema.toBool() == true)
    {
        // make sure no two items are identical

        QJsonArray jArray = value.toArray();
        for(int i = 0; i < jArray.size(); ++i)
        {
            for (int j = i+1; j < jArray.size(); ++j)
            {
                if (jArray[i] == jArray[j])
                {
                    // found a value twice
                    _error = true;
                    setMessage(QStringLiteral("array must have unique values"));
                }
            }
        }
    }
}

void QJsonSchemaChecker::checkRequired(const QJsonValue & value, const QJsonValue & schema)
{
    if (!schema.isArray()) {
        _error = true;
        setMessage(QStringLiteral("required field must be an array in the schema: %1").arg(schema.toString()));
        return;
    }

    QJsonObject valueObj = value.toObject();

    for (const QJsonValue & requiredValue : schema.toArray()) {
        if (!valueObj.contains(requiredValue.toString())) {
            _error = true;
            setMessage(QStringLiteral("required field %1 not present").arg(requiredValue.toString()));
        }
    }
}

void QJsonSchemaChecker::checkEnum(const QJsonValue & value, const QJsonValue & schema)
{
    if (schema.isArray())
    {
        QJsonArray jArray = schema.toArray();
        for(int i = 0; i < jArray.size(); ++i)
        {
            if (jArray[i] == value)
            {
                // found enum value. done.
                return;
            }
        }
    }

    // nothing found
    _error = true;
    QJsonDocument doc(schema.toArray());
    setMessage(QStringLiteral("Unknown enum value (allowed values are: %1)").arg(QLatin1String(doc.toJson(QJsonDocument::Compact))));
}
