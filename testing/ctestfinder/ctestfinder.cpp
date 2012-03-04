/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    CTestTestfile.cmake parsing uses code from the xUnit plugin
    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
    Copyright 2010 Daniel Calviño Sánchez <danxuliu@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ctestfinder.h"
#include "ctestsuite.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/itestcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocale>
#include <KDebug>
#include "ctestfindjob.h"
#include <util/executecompositejob.h>

K_PLUGIN_FACTORY(CTestFinderFactory, registerPlugin<CTestFinder>(); )
K_EXPORT_PLUGIN(CTestFinderFactory(KAboutData("kdevctestfinder","kdevctestfinder", ki18n("CTest Integration"), "0.1", ki18n("Finds and executes CTest unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;

CTestFinder::CTestFinder(QObject* parent, const QList<QVariant>& args): IPlugin(CTestFinderFactory::componentData(), parent)
{
    Q_UNUSED(args);

    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ITestProvider )
    KDEV_USE_EXTENSION_INTERFACE( ICTestProvider )
}

CTestFinder::~CTestFinder()
{

}

void CTestFinder::unload()
{
}

void CTestFinder::createTestSuite(const QString& name, const QString& executable, const QStringList& files, IProject* project, const QStringList& arguments)
{
    // CMake parser replaces all references to the project build directory by #[bin_dir]
    // We replace it back for the test executable and arguments
    
    QString exe = executable;
    QString binDir = project->buildSystemManager()->buildDirectory(project->projectItem()).toLocalFile();
    exe.replace("#[bin_dir]", binDir);
    
    QStringList args = arguments;
    for (QStringList::iterator it = args.begin(); it != args.end(); ++it)
    {
        (*it).replace("#[bin_dir]", binDir);
    }
    
    CTestSuite* suite = new CTestSuite(name, exe, files, project, args);
    m_pendingSuites << suite;
}

KJob* CTestFinder::findTests()
{
    kDebug() << "Finding tests with" << m_pendingSuites.size() << "pending suites"; 
    QList<KJob*> jobs;
    foreach (CTestSuite* suite, m_pendingSuites)
    {
        jobs << new CTestFindJob(suite, this);
    }
    if (jobs.isEmpty())
    {
        return 0;
    }
    m_pendingSuites.clear();
    return new KDevelop::ExecuteCompositeJob(this, jobs);
}


