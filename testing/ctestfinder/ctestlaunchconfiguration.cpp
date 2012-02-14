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

#include "ctestlaunchconfiguration.h"
#include "ctestsuite.h"
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <shell/runcontroller.h>

using namespace KDevelop;

CTestLaunchConfiguration::CTestLaunchConfiguration(const CTestSuite* suite, const QStringList& cases) :
m_testSuite(suite),
m_cases(cases)
{
    QStringList parameters = QStringList() << suite->url().toLocalFile() << cases;
    m_config = suite->project()->projectConfiguration()->group("CTest").group(parameters.join(", "));
}

CTestLaunchConfiguration::~CTestLaunchConfiguration()
{
    m_config.deleteGroup();
}

KDevelop::IProject* CTestLaunchConfiguration::project() const
{
    return m_testSuite->project();
}

QString CTestLaunchConfiguration::name() const
{
    return m_testSuite->name();
}

KDevelop::LaunchConfigurationType* CTestLaunchConfiguration::type() const
{
    return ICore::self()->runController()->launchConfigurationTypeForId("CTest");
}

KConfigGroup CTestLaunchConfiguration::config()
{
    return m_config;
}

const KConfigGroup CTestLaunchConfiguration::config() const
{
    return m_config;
}

const CTestSuite* CTestLaunchConfiguration::suite() const
{
    return m_testSuite;
}
