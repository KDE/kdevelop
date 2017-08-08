/* This file is part of KDevelop
    Copyright 2016 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <project/abstractfilemanagerplugin.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testproject.h>

#include <util/path.h>

#include <KJob>

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

    auto plugin = new AbstractFileManagerPlugin({}, core);
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
