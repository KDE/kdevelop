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

#include "ctestutils.h"
#include "ctestsuite.h"

#include <interfaces/iproject.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

CTestUtils::createTestSuite(const QString& name, const QString& executable, const QStringList& files, IProject* project, const QStringList& arguments)
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
    
    ITestController* tc = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>();
    if (tc)
    {
        tc->addTestSuite(suite);
    }
}
