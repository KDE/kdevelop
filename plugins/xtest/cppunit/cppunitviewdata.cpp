/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cppunitviewdata.h"
#include <KSharedConfig>
#include <KConfigGroup>
#include <interfaces/iproject.h>
#include "../cppunit/register.h"
#include "testroot.h"
#include "testsuite.h"

using namespace KDevelop;
using namespace Veritas;
using namespace CppUnit;

CppUnitViewData::CppUnitViewData(ITestFramework* framework) : ITestRunner(framework)
{
}

CppUnitViewData::~CppUnitViewData() {}

void CppUnitViewData::registerTests()
{
    Register<TestRoot, TestSuite> reg;
    KUrl testExe = KUrl(fetchExe());
    if (testExe.isEmpty()) {
        return; // TODO failure message
    }
    reg.addFromExe(testExe);
    reg.rootItem()->setExecutable(testExe.path());
    emit registerFinished(reg.rootItem());
}

QString CppUnitViewData::fetchExe()
{
    if (project() == 0) return QString();
    KSharedConfig::Ptr cfg = project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "Veritas");
    QStringList executables;
    executables = group.readEntry<QStringList>("executables", executables);
    if (executables.isEmpty()) return QString();
    return executables[0];
}

#include "cppunitviewdata.moc"
