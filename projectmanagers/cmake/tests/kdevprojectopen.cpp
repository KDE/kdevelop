/* This file is part of KDevelop
    Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include <KApplication>
#include <KLocalizedString>
#include <KCmdLineArgs>
#include <KUrl>
#include <KAboutData>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <interfaces/iprojectcontroller.h>
#include "kdevprojectopen.h"
#include "testhelpers.h"
#include <QDebug>
#include <k4aboutdata.h>

using namespace KDevelop;

KDevProjectOpen::KDevProjectOpen(QObject* parent)
    : QObject(parent)
    , m_toOpen(0)
{
    AutoTestShell::init();
    TestCore::initialize();

    cleanup();
    connect(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), SLOT(projectDone(KDevelop::IProject*)));
}

void KDevProjectOpen::cleanup()
{
    foreach(IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }
    Q_ASSERT(ICore::self()->projectController()->projects().isEmpty());
}

void KDevProjectOpen::openProject(const KUrl& path)
{
    const TestProjectPaths paths = projectPaths(path.toLocalFile());
    defaultConfigure(paths);

    m_toOpen++;
    ICore::self()->projectController()->openProject(paths.projectFile);
}

void KDevProjectOpen::projectDone(IProject* )
{
    m_toOpen--;
    if(m_toOpen==0) {
        cleanup();
        qApp->exit();
    }
}

int main(int argc, char** argv)
{
    K4AboutData aboutData( "kdevprojectopen", 0, ki18n( "KDevelop" ), "0.99", ki18n("opens a project then closes it, debugging tool"), K4AboutData::License_GPL,
                          ki18n( "Copyright 1999-2012, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org/" );
    aboutData.addAuthor( ki18n("Aleix Pol Gonzalez"), ki18n( "" ), "aleixpol@kde.org" );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
    options.add("+projects", ki18n( "Projects to load" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    KApplication app;
    if(args->count()==0) {
        args->usageError("Didn't provide a project to open");
        return 0;
    }

    KDevProjectOpen opener;
    for(int i=0; i<args->count(); ++i) {
        opener.openProject(args->url(i));
    }
    args->clear();
    return app.exec();
}
