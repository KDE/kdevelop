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
#include "ctestlaunchconfigurationtype.h"
#include "ctestlauncher.h"

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

K_PLUGIN_FACTORY(CTestFinderFactory, registerPlugin<CTestFinder>(); )
K_EXPORT_PLUGIN(CTestFinderFactory(KAboutData("kdevctestfinder","kdevctestfinder", ki18n("CTest Finder"), "0.1", ki18n("Finds CTest unit tests"), KAboutData::License_GPL)))

using namespace KDevelop;

CTestFinder::CTestFinder(QObject* parent, const QList<QVariant>& args): IPlugin(CTestFinderFactory::componentData(), parent)
{
    Q_UNUSED(args);

    KDEV_USE_EXTENSION_INTERFACE( ICTestProvider )

    m_controller = core()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>();
    
    m_configType = new CTestLaunchConfigurationType;
    m_configType->addLauncher(new CTestLauncher(this));
    core()->runController()->addConfigurationType(m_configType);
}

CTestFinder::~CTestFinder()
{

}

void CTestFinder::unload()
{
    core()->runController()->removeConfigurationType(m_configType);
}

void CTestFinder::createTestSuite(const QString& name, const QString& executable, IProject* project, const QStringList& arguments)
{
    QString exe = executable;
    if (exe.startsWith("#[bin_dir]"))
    {
        exe.remove("#[bin_dir]");
    }
    KUrl exeUrl = project->buildSystemManager()->buildDirectory(project->projectItem());
    exeUrl.addPath(exe);
    Q_ASSERT(exeUrl.isLocalFile());
    kDebug() << exeUrl << exeUrl.toLocalFile();
    CTestSuite* suite = new CTestSuite(name, exeUrl, project, arguments);
    suite->setTestController(m_controller);
    suite->setLaunchConfigurationType(m_configType);
    suite->loadCases();
    m_controller->addTestSuite(suite);
}


