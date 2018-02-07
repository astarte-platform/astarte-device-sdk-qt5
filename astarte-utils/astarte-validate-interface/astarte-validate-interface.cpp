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

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTextStream>

#include <HemeraCore/Operation>

#include <ValidateInterfaceOperation.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setApplicationName(QObject::tr("Astarte interface validator"));
    app.setOrganizationDomain(QStringLiteral("com.ispirata.Hemera"));
    app.setOrganizationName(QStringLiteral("Ispirata"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Astarte interface manager"));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("interface"), QObject::tr("The path to the interface JSON"));

    parser.addOptions({
        {
            QStringList{QStringLiteral("f"), QStringLiteral("force")},
            QObject::tr("Force interface registration even if it overwrites an already existing interface file")
        }
    });

    parser.process(app);

    if (parser.positionalArguments().length() < 1) {
        parser.showHelp();
    } else {
        QStringList arguments = parser.positionalArguments();
        QString interface = arguments.value(0);

        if (interface.isEmpty()) {
            QTextStream(stderr) << QObject::tr("You must supply an interface file to validate\n");
            return -1;
        }

        ValidateInterfaceOperation *op = new ValidateInterfaceOperation(interface);
        QObject::connect(op, &Hemera::Operation::finished, [op, &app] {
            if (op->isError()) {
                QTextStream(stderr) << QObject::tr("Validation failed:\n%1\n")
                                       .arg(op->errorMessage());
                app.exit(1);
            } else {
                QTextStream(stdout) << QObject::tr("Valid interface\n");
                app.quit();
            }
        });

        return app.exec();
    }

}
