/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakedriver.h"

#include <KAboutData>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    KAboutData aboutData(QLatin1String("QMake Parser"), QLatin1String("qmake-parser"), QLatin1String("1.0"));
    aboutData.setShortDescription(QLatin1String("Parse QMake project files"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QLatin1String("debug"), QLatin1String("Enable output of the debug AST")));
    parser.addPositionalArgument(QLatin1String("files"), QLatin1String("QMake project files"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
        return EXIT_FAILURE;
    }

    const bool debug = parser.isSet(QLatin1String("debug"));

    foreach (const auto arg, parser.positionalArguments()) {
        QMake::Driver driver;
        if (!driver.readFile(arg)) {
            exit(EXIT_FAILURE);
        }
        driver.setDebug(debug);

        QMake::ProjectAST* ast = nullptr;
        if (!driver.parse(&ast)) {
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}
