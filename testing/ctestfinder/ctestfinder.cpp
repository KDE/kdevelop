/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KLocale>

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

}


