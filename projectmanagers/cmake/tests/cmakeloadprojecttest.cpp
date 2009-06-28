/***************************************************************************
 *   Copyright 2009 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "cmakeloadprojecttest.h"

#include <qtest_kde.h>
#include <QtTest/QtTest>

#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <kdebug.h>
#include <kapplication.h>

#include <shell/core.h>
#include <interfaces/iprojectcontroller.h>

CMakeLoadProjectTest::CMakeLoadProjectTest()
{
    s_instance = this;
}

QString CMakeLoadProjectTest::xmlFile()
{
    return "kdevelopui.rc";
}
KDevelop::AreaParams CMakeLoadProjectTest::defaultArea()
{
    KDevelop::AreaParams p = { "code", i18n("Code" ) };
    return p;
}
QString CMakeLoadProjectTest::projectFileExtension()
{
    return "kdev4";
}
QString CMakeLoadProjectTest::projectFileDescription()
{
    return i18n("KDevelop4 Project files");
}
QStringList CMakeLoadProjectTest::defaultPlugins()
{
    return QStringList() << "KDevCMakeManager" << "KDevProjectManagerView";
}

int main(int argc, char** argv)
{
    KAboutData aboutdata( "cmakeloadprojecttest", 0, ki18n("cmakeloadprojecttest"), "0.0.0", ki18n("small utility to do manual performance checks of cmake project load times"), KAboutData::License_GPL);
    KCmdLineArgs::init( argc, argv, &aboutdata );
    KCmdLineOptions options;
    options.add( "project <project>", ki18n("url to project") );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    KApplication app;
    CMakeLoadProjectTest shell;
    Core::initialize();
    Core* core = Core::self();
    QString project = args->getOption( "project" );
    if( project.isEmpty() ) 
    {
        qWarning() << "Need a project to load";
        exit(1);
    }
    core->projectController()->openProject( KUrl( project ) );
    args->clear();
    return app.exec();

}


