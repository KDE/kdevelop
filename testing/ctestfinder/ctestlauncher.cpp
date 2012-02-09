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

#include "ctestlauncher.h"
#include <KLocalizedString>

KJob* CTestLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    if (!cfg || launchMode != "test")
    {
        return 0;
    }
    
    // TODO: Implement
    return 0;
}

QList< KDevelop::LaunchConfigurationPageFactory* > CTestLauncher::configPages() const
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QStringList CTestLauncher::supportedModes() const
{
    return QStringList() << "test";
}

QString CTestLauncher::description() const
{
    return i18n("Runs CTest unit tests");
}

QString CTestLauncher::name() const
{
    return i18n("CTest unit test");
}

QString CTestLauncher::id()
{
    return "CTestLauncher";
}

