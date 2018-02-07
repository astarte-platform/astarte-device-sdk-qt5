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

#ifndef VALIDATEINTERFACESOPERATION_H
#define VALIDATEINTERFACESOPERATION_H

#include <HemeraCore/Operation>

class QJsonSchemaChecker;

class ValidateInterfaceOperation : public Hemera::Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(ValidateInterfaceOperation)

public:
    explicit ValidateInterfaceOperation(const QString &interface, QObject *parent = nullptr);
    virtual ~ValidateInterfaceOperation();

protected:
    virtual void startImpl() override final;

private:
    QJsonSchemaChecker *m_checker;
    QString m_path;
};

#endif
