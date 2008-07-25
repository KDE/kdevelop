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

CppUnitViewData::CppUnitViewData(QObject* parent) : TestViewData(parent)
{
    m_id = CppUnitViewData::id;
    CppUnitViewData::id += 1;
}

CppUnitViewData::~CppUnitViewData() {}

Test* CppUnitViewData::registerTests()
{
    Register<TestRoot, TestSuite> reg;
    reg.addFromExe(QFileInfo(fetchExe()));
    reg.rootItem()->setExecutable(fetchExe());
    return reg.rootItem();
}

QString CppUnitViewData::fetchExe()
{
    if (project() == 0) {
        return "";
    }
    KSharedConfig::Ptr cfg = project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "CppUnit");
    return KUrl(group.readEntry("Test Registration")).pathOrUrl();
}

QString CppUnitViewData::resultsViewId()
{
    return QString("org.kdevelop.CppUnitResultsView") + QString::number(m_id);
}

int CppUnitViewData::id = 0;

#include "cppunitviewdata.moc"
