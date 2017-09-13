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
#include "ctestfindjob.h"
#include <debug.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <QDir>

using namespace KDevelop;

// TODO: we are lacking introspection into targets, to see what files belong to each target.
static CMakeTarget targetByName(const QHash< KDevelop::Path, QVector<CMakeTarget>>& targets, const QString& name)
{
    for (const auto &subdir: targets.values()) {
        for (const auto &target: subdir) {
            if (target.name == name)
                return target;
        }
    }
    return {};
}

void CTestUtils::createTestSuites(const QVector<Test>& testSuites, const QHash< KDevelop::Path, QVector<CMakeTarget>>& targets, KDevelop::IProject* project)
{
    foreach (const Test& test, testSuites) {
        KDevelop::Path executablePath;
        if (QDir::isAbsolutePath(test.executable)) {
            executablePath = KDevelop::Path(test.executable);
        } else {
            const auto target = targetByName(targets, test.executable);
            if (target.artifacts.isEmpty()) {
                continue;
            }
            executablePath = target.artifacts.constFirst();
        }

        CTestSuite* suite = new CTestSuite(test.name, executablePath, {}, project, test.arguments, test.properties);
        ICore::self()->runController()->registerJob(new CTestFindJob(suite));
    }
}
