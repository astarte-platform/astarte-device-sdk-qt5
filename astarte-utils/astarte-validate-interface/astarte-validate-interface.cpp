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
