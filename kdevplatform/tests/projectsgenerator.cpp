/*
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "projectsgenerator.h"

#include <tests/kdevsignalspy.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <util/path.h>

#include <QString>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>


using namespace KDevelop;

namespace
{
/// @param projectFile projectName.kdev4 file
IProject* loadProject( const QString& projectFile, const QString& projectName )
{
    KDevSignalSpy projectSpy(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    ICore::self()->projectController()->openProject( QUrl::fromLocalFile(projectFile) );

    if (!projectSpy.wait(5000)) {
        qFatal("Expected project to be loaded within 5 seconds, but this didn't happen");
    }
    IProject* project = ICore::self()->projectController()->findProjectByName( projectName );

    return project;
}

void createFile( QFile& file )
{
    file.remove();
    if ( !file.open( QIODevice::ReadWrite ) ) {
        qFatal("Cannot create the file %s", file.fileName().toUtf8().constData());
    }
}
}

namespace KDevelop {
IProject* ProjectsGenerator::GenerateSimpleProject()
{
    const QString sp = QStringLiteral( "simpleproject" );
    auto rootFolder = QDir::temp();
    QDir(rootFolder.absolutePath() + QStringLiteral( "/" ) + sp).removeRecursively();
    rootFolder.mkdir( sp );
    rootFolder.cd( sp );
    rootFolder.mkdir( QStringLiteral("src") );
    rootFolder.mkdir( QStringLiteral(".kdev4") );

    {
        QFile file( rootFolder.filePath( QStringLiteral("simpleproject.kdev4") ) );
        createFile( file );
        QTextStream stream1( &file );
        stream1 << "[Project]\nName=SimpleProject\nManager=KDevCustomBuildSystem";
    }
    {
        QFile file( rootFolder.filePath( QStringLiteral("src/main.cpp") ) );
        createFile( file );
    }
    {
        QFile file( rootFolder.filePath( QStringLiteral(".kdev4/simpleproject.kdev4") ) );
        createFile( file );
        QTextStream stream( &file );
        stream << "[Buildset]\n" <<
        "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x1a\\x00S\\x00i\\x00m\\x00p\\x00l\\x00e\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0]\n" <<
        "Path=/\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0][Defines]\n" <<
        "_DEBUG=\n" <<
        "VARIABLE=VALUE\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0][Includes]\n" <<
        "1=" << QDir::rootPath() << "usr/include/mydir\n" <<
        "[Project]\n" <<
        "VersionControlSupport=\n";
    }
    return loadProject( QDir::tempPath() + QStringLiteral( "/simpleproject/simpleproject.kdev4" ), QStringLiteral("SimpleProject") );
}

IProject* ProjectsGenerator::GenerateMultiPathProject()
{
    const QString mp = QStringLiteral( "multipathproject" );
    auto rootFolder = QDir::temp();
    QDir(rootFolder.absolutePath() + QStringLiteral( "/" ) + mp).removeRecursively();
    rootFolder.mkdir( mp );
    rootFolder.cd( mp );
    rootFolder.mkdir( QStringLiteral("src") );
    rootFolder.mkdir( QStringLiteral(".kdev4") );
    rootFolder.mkdir( QStringLiteral("anotherFolder") );

    {
        QFile file( rootFolder.filePath( QStringLiteral("multipathproject.kdev4") ) );
        createFile( file );
        QTextStream stream1( &file );
        stream1 << "[Project]\nName=MultiPathProject\nManager=KDevCustomBuildSystem";
        ;
    }
    {
        QFile file1( rootFolder.filePath( QStringLiteral("src/main.cpp") ) );
        createFile( file1 );
        QFile file2( rootFolder.filePath( QStringLiteral("anotherFolder/tst.h") ) );
        createFile( file2 );
    }
    {
        QFile file( rootFolder.filePath( QStringLiteral(".kdev4/multipathproject.kdev4") ) );
        createFile( file );
        QTextStream stream( &file );
        stream << "[Buildset]\n" <<
        "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00 \\x00M\\x00u\\x00l\\x00t\\x00i\\x00P\\x00a\\x00t\\x00h\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0]\n" <<
        "Path=.\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0][Defines]\n" <<
        "SOURCE=CONTENT\n" <<
        "_COPY=\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0][Includes]\n" <<
        "1=" << QDir::rootPath() << "usr/include/otherdir\n" <<
        "[CustomDefinesAndIncludes][ProjectPath1]\n" <<
        "Path=src\n" <<
        "[CustomDefinesAndIncludes][ProjectPath1][Defines]\n" <<
        "BUILD=debug\n" <<
        "[CustomDefinesAndIncludes][ProjectPath1][Includes]\n" <<
        "1=" << QDir::rootPath() << "usr/local/include/mydir\n" <<
        "[CustomDefinesAndIncludes][ProjectPath2]\n" <<
        "Path=anotherFolder\n" <<
        "[CustomDefinesAndIncludes][ProjectPath2][Defines]\n" <<
        "HIDDEN=\n" <<
        "[Project]\n" <<
        "VersionControlSupport=\n";
    }
    return loadProject( QDir::tempPath() + QStringLiteral( "/multipathproject/multipathproject.kdev4" ), QStringLiteral("MultiPathProject") );
}

IProject* ProjectsGenerator::GenerateSimpleProjectWithOutOfProjectFiles()
{
    auto project = GenerateSimpleProject();
    Q_ASSERT(project);

    auto rootFolder = QDir(project->path().path());
    const QString includePaths = QStringLiteral(".kdev_include_paths");

    QFile file(rootFolder.filePath(includePaths));
    createFile(file);
    QTextStream stream( &file );
    stream << QStringLiteral(".") + QDir::separator() + QStringLiteral("include1.h") << endl << rootFolder.canonicalPath() + QDir::separator() + QStringLiteral("include2.h");

    return project;
}

IProject* ProjectsGenerator::GenerateEmptyProject()
{
    const QString ep = QStringLiteral("emptyproject");
    auto rootFolder = QDir::temp();
    QDir(rootFolder.absolutePath() + QStringLiteral("/") + ep).removeRecursively();
    rootFolder.mkdir(ep);
    rootFolder.cd(ep);
    rootFolder.mkdir(QStringLiteral(".kdev4"));

    {
        QFile file(rootFolder.filePath(QStringLiteral("emptyproject.kdev4")));
        createFile(file);
        QTextStream stream(&file);
        stream << "[Project]\nName=EmptyProject\nManager=KDevCustomBuildSystem";
    }

    {
        QFile file(rootFolder.filePath(QStringLiteral(".kdev4/emptyproject.kdev4")));
        createFile(file);
        QTextStream stream(&file);
        stream << "[Project]\n" << "VersionControlSupport=\n";
    }
    return loadProject(QDir::tempPath() + QStringLiteral( "/emptyproject/emptyproject.kdev4" ), QStringLiteral("EmptyProject"));
}
} // KDevelop
