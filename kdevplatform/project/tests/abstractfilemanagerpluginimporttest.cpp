/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <project/abstractfilemanagerplugin.h>

#include <tests/autotestshell.h>
#include <tests/plugintesthelpers.h>
#include <tests/testcore.h>
#include <tests/testproject.h>

#include <util/path.h>

#include <KJob>
#include <KPluginMetaData>

#include <QCoreApplication>
#include <QDebug>

using namespace KDevelop;

int main(int argc, char** argv)
{
    if (argc != 2) {
        qWarning() << "Missing argument for directory path to list.";
        return 1;
    }
    QCoreApplication app(argc, argv);

    AutoTestShell::init();
    auto core = TestCore::initialize(Core::NoUi);

    const auto pluginMetaData = makeTestPluginMetaData("ImportTestAbstractFileManager");
    auto plugin = new AbstractFileManagerPlugin({}, core, pluginMetaData);
    auto project = new TestProject(Path(QString::fromUtf8(argv[1])));
    auto root = plugin->import(project);
    auto import = plugin->createImportJob(root);
    QObject::connect(import, &KJob::finished,
                     &app, [project] {
                        qDebug() << "loaded project with" << project->fileSet().size() << "files";
                        QCoreApplication::instance()->quit();
                     });
    import->start();

    return app.exec();
}
