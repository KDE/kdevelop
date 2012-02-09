/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#include "ctestlaunchconfigurationtype.h"

#include "interfaces/icore.h"
#include "interfaces/itestcontroller.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KIcon>
#include "ctestlauncher.h"

using namespace KDevelop;

CTestLaunchConfigurationType::CTestLaunchConfigurationType()
{
    addLauncher(new CTestLauncher);
}

CTestLaunchConfigurationType::~CTestLaunchConfigurationType()
{

}

bool CTestLaunchConfigurationType::canLaunch(const KUrl& file) const
{
    return ICore::self()->testController()->testSuiteForUrl(file);
}

void CTestLaunchConfigurationType::configureLaunchFromCmdLineArguments(KConfigGroup config, const QStringList& args) const
{
    // TODO
}

void CTestLaunchConfigurationType::configureLaunchFromItem(KConfigGroup config, KDevelop::ProjectBaseItem* item) const
{
    // TODO
}

bool CTestLaunchConfigurationType::canLaunch(KDevelop::ProjectBaseItem* item) const
{
    return false;
}

KIcon CTestLaunchConfigurationType::icon() const
{
    return KIcon("cmake");
}

QList< KDevelop::LaunchConfigurationPageFactory* > CTestLaunchConfigurationType::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString CTestLaunchConfigurationType::name() const
{
    return i18n("CTest unit test");
}

QString CTestLaunchConfigurationType::id() const
{
    return "CTest";
}

