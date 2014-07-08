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

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <KDebug>

using namespace KDevelop;

namespace
{
/// @param projectFile projectName.kdev4 file
IProject* loadProject( const QString& projectFile, const QString& projectName )
{
    KDevSignalSpy* projectSpy = new KDevSignalSpy( ICore::self()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ) );
    ICore::self()->projectController()->openProject( projectFile );

    if( !projectSpy->wait( 5000 ) ) {
        kFatal() << "Expected project to be loaded within 5 seconds, but this didn't happen";
    }
    IProject* project = ICore::self()->projectController()->findProjectByName( projectName );

    return project;
}

void createFile( QFile& file )
{
    file.remove();
    if ( !file.open( QIODevice::ReadWrite ) ) {
        kFatal() << "Cannot create the file " << file.fileName();
    }
}
}

IProject* ProjectsGenerator::GenerateSimpleProject()
{
    // directory structure:
    // ./simpleproject.kdev4
    // ./src/main.cpp
    // ./.kdev4/simpleproject.kdev4

    const QString sp = QLatin1String( "simpleproject" );
    auto rootFolder = QDir::temp();
    QDir(rootFolder.absolutePath() + "/" + sp).removeRecursively();
    rootFolder.mkdir( sp );
    rootFolder.cd( sp );
    rootFolder.mkdir( "src" );
    rootFolder.mkdir( ".kdev4" );

    {
        QFile file( rootFolder.filePath( "simpleproject.kdev4" ) );
        createFile( file );
        QTextStream stream1( &file );
        stream1 << "[Project]\nName=SimpleProject\nManager=KDevCustomBuildSystem";
    }
    {
        QFile file( rootFolder.filePath( "src/main.cpp" ) );
        createFile( file );
    }
    {
        QFile file( rootFolder.filePath( ".kdev4/simpleproject.kdev4" ) );
        createFile( file );
        QTextStream stream( &file );
        stream << "[Buildset]\n" <<
        "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x1a\\x00S\\x00i\\x00m\\x00p\\x00l\\x00e\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n" <<
        "[CustomBuildSystem]\n" << "CurrentConfiguration=BuildConfig0\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0]\n" <<
        "Defines=\\x00\\x00\\x00\\x02\\x00\\x00\\x00\\x0c\\x00_\\x00D\\x00E\\x00B\\x00U\\x00G\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x10\\x00V\\x00A\\x00R\\x00I\\x00A\\x00B\\x00L\\x00E\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\n\\x00V\\x00A\\x00L\\x00U\\x00E\n" <<
        "Includes=\\x00\\x00\\x00\\x01\\x00\\x00\\x00$\\x00/\\x00u\\x00s\\x00r\\x00/\\x00i\\x00n\\x00c\\x00l\\x00u\\x00d\\x00e\\x00/\\x00m\\x00y\\x00d\\x00i\\x00r\n" <<
        "Path=/\n" <<
        "[Project]\n" <<
        "VersionControlSupport=\n";
    }
    return loadProject( QDir::tempPath() + "/simpleproject/simpleproject.kdev4", "SimpleProject" );
}

IProject* ProjectsGenerator::GenerateMultiPathProject()
{
    // directory structure:
    // ./multipathproject.kdev4
    // ./src/main.cpp
    // ./anotherFolder/tst.h
    // ./.kdev4/multipathproject.kdev4

    const QString mp = QLatin1String( "multipathproject" );
    auto rootFolder = QDir::temp();
    QDir(rootFolder.absolutePath() + "/" + mp).removeRecursively();
    rootFolder.mkdir( mp );
    rootFolder.cd( mp );
    rootFolder.mkdir( "src" );
    rootFolder.mkdir( ".kdev4" );
    rootFolder.mkdir( "anotherFolder" );

    {
        QFile file( rootFolder.filePath( "multipathproject.kdev4" ) );
        createFile( file );
        QTextStream stream1( &file );
        stream1 << "[Project]\nName=MultiPathProject\nManager=KDevCustomBuildSystem";
        ;
    }
    {
        QFile file1( rootFolder.filePath( "src/main.cpp" ) );
        createFile( file1 );
        QFile file2( rootFolder.filePath( "anotherFolder/tst.h" ) );
        createFile( file2 );
    }
    {
        QFile file( rootFolder.filePath( ".kdev4/multipathproject.kdev4" ) );
        createFile( file );
        QTextStream stream( &file );
        stream << "[Buildset]\n" <<
        "BuildItems=@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00 \\x00M\\x00u\\x00l\\x00t\\x00i\\x00P\\x00a\\x00t\\x00h\\x00P\\x00r\\x00o\\x00j\\x00e\\x00c\\x00t)\n" <<
        "[CustomBuildSystem]\n" <<
        "CurrentConfiguration=BuildConfig0\n" <<
        "[CustomDefinesAndIncludes][ProjectPath0]\n" <<
        "Defines=\\x00\\x00\\x00\\x02\\x00\\x00\\x00\\n\\x00_\\x00C\\x00O\\x00P\\x00Y\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x0c\\x00S\\x00O\\x00U\\x00R\\x00C\\x00E\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\x0e\\x00C\\x00O\\x00N\\x00T\\x00E\\x00N\\x00T\n" <<
        "Includes=\\x00\\x00\\x00\\x01\\x00\\x00\\x00*\\x00/\\x00u\\x00s\\x00r\\x00/\\x00i\\x00n\\x00c\\x00l\\x00u\\x00d\\x00e\\x00/\\x00o\\x00t\\x00h\\x00e\\x00r\\x00d\\x00i\\x00r\n" <<
        "Path=.\n" <<
        "[CustomDefinesAndIncludes][ProjectPath1]\n" <<
        "Defines=\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\n\\x00B\\x00U\\x00I\\x00L\\x00D\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\n\\x00d\\x00e\\x00b\\x00u\\x00g\n" <<
        "Includes=\\x00\\x00\\x00\\x01\\x00\\x00\\x000\\x00/\\x00u\\x00s\\x00r\\x00/\\x00l\\x00o\\x00c\\x00a\\x00l\\x00/\\x00i\\x00n\\x00c\\x00l\\x00u\\x00d\\x00e\\x00/\\x00m\\x00y\\x00d\\x00i\\x00r\n" <<
        "Path=src\n" <<
        "[CustomDefinesAndIncludes][ProjectPath2]\n" <<
        "Defines=\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0c\\x00H\\x00I\\x00D\\x00D\\x00E\\x00N\\x00\\x00\\x00\\n\\x00\\x00\\x00\\x00\\x00\n" <<
        "Path=anotherFolder\n" <<
        "[Project]\n" <<
        "VersionControlSupport=\n";
    }
    return loadProject( QDir::tempPath() + "/multipathproject/multipathproject.kdev4", "MultiPathProject" );
}
