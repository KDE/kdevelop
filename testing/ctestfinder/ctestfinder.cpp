/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    CTestTestfile.cmake parsing uses code from the xUnit plugin
    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
    Copyright 2010 Daniel Calviño Sánchez <danxuliu@gmail.com>

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

#include "ctestfinder.h"
#include "ctestsuite.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocale>
#include <QFile>
#include <../kdevplatform/interfaces/itestcontroller.h>

K_PLUGIN_FACTORY(CTestFinderFactory, registerPlugin<CTestFinder>(); )
K_EXPORT_PLUGIN(CTestFinderFactory(KAboutData("kdevctestfinder","kdevctestfinder", ki18n("CTest Finder"), "0.1", ki18n("Finds CTest unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;

CTestFinder::CTestFinder(QObject* parent, const QList<QVariant>& args): IPlugin(CTestFinderFactory::componentData(), parent)
{
    Q_UNUSED(args);
    foreach (IProject* project, core()->projectController()->projects())
    {
        findTestsForProject(project);
    }
    connect (core()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), SLOT(findTestsForProject(KDevelop::IProject*)));
}

CTestFinder::~CTestFinder()
{

}

void CTestFinder::findTestsForProject(IProject* project)
{
    IBuildSystemManager* bm = project->buildSystemManager();
    if (!bm)
    {
        return;
    }
    findTestsInDirectory(bm->buildDirectory(project->projectItem()));
}

void CTestFinder::findTestsInDirectory(const KUrl& directory)
{
    KUrl fileUrl = directory;
    fileUrl.addPath("CTestTestfile.cmake");
    QFile file(fileUrl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    while (!file.atEnd())
    {
        QString line = file.readLine();
        if (line.startsWith('#'))
        {
            continue;
        }
        else if (line.startsWith("SUBDIRS("))
        {
            line = line.trimmed();
            line.remove("SUBDIRS(");
            line.remove(')');

            KUrl subDirUrl = directory;
            subDirUrl.cd(line);
            findTestsInDirectory(subDirUrl);
        }
        else if (line.startsWith("ADD_TEST("))
        {
            line = line.trimmed();
            line.remove("ADD_TEST(");
            line.remove(')');
            int firstSpace = line.indexOf(' ');
            if (firstSpace == -1)
            {
                continue;
            }
            QString name = line.mid(0, firstSpace);
            line.remove(0, firstSpace);
            QStringList args = line.split('\"');
            QMutableStringListIterator it(args);
            while(it.hasNext())
            {
                QString arg = it.next().trimmed();
                if (arg.isEmpty())
                {
                    it.remove();
                }
            }
            int n = args.size();
            if (n < 1)
            {
                continue;
            }
            QString exe = args.takeFirst();
            if (!exe.startsWith(directory.toLocalFile()))
            {
                exe = directory.toLocalFile(KUrl::AddTrailingSlash) + exe;
            }
            CTestSuite* suite = new CTestSuite(name, exe, args);
            core()->testController()->addTestSuite(suite);
        }
    }
}

