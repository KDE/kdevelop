/* KDevelop xUnit plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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


#include "reportfileitemtest.h"
#include "coveredfilefactory.h"
#include "../reportmodel.h"
#include "../coveredfile.h"

#include "testutils.h"

using Veritas::ReportFileItemTest;
using Veritas::ReportFileItem;
using Veritas::CoveredFile;
using Veritas::Test::CoveredFileFactory;

void ReportFileItemTest::init()
{
    m_url = KUrl("/path/to/foo.cpp");
    m_fileName = m_url.fileName();
    m_file = new ReportFileItem(m_url);
}

void ReportFileItemTest::cleanup()
{
    if (m_file) {
        delete m_file->coverageItem();
        delete m_file->instrumentedItem();
        delete m_file->slocItem();
        delete m_file;
    }
    qDeleteAll(m_garbage);
    m_garbage.clear();
}

void ReportFileItemTest::construct()
{
    KVERIFY(ReportModel::File == m_file->type());
    KOMPARE(m_url, m_file->url());

    // QStandardItem properties
    KVERIFY(! m_file->isEditable());
    KVERIFY(m_file->isSelectable());
    KVERIFY(! m_file->isCheckable());

    // Neighbour items should be initialized on 0
    KOMPARE(0, m_file->slocItem());
    KOMPARE(0, m_file->coverageItem());
    KOMPARE(0, m_file->instrumentedItem());
}

void ReportFileItemTest::addCoverage()
{
    int sloc = 2, nrofCovered = 1;
    CoveredFile* f = CoveredFileFactory::create(m_url, sloc, nrofCovered);
    m_file->addCoverageData(f);

    assertSlocItem(2);
    assertInstrumentedItem(1);
    assertCoverageItem(100*(double)1/2);
    
    m_garbage << f;
}

void ReportFileItemTest::assertInstrumentedItem(int instrumented)
{
    ReportValueItem* instrumentedItem = m_file->instrumentedItem();
    KOMPARE(instrumented, instrumentedItem->value());
}

void ReportFileItemTest::assertSlocItem(int sloc)
{
    ReportValueItem* slocItem = m_file->slocItem();
    KOMPARE(sloc, slocItem->value());
}

void ReportFileItemTest::assertCoverageItem(double coverage)
{
    ReportValueItem* coverageItem = m_file->coverageItem();
    double actual = coverageItem->value();
    KVERIFY_MSG(qAbs(coverage - actual) < 0.1,
                QString("\nWrong coverage percentage for coverage item.\n"
                        "expected %1 but got %2").arg(coverage).arg(actual));
}

void ReportFileItemTest::addMultipleCoverage()
{
    QList<int> covered; covered << 1 << 2 << 3;
    QList<int> notCovered; notCovered << 4;
    CoveredFile* f = CoveredFileFactory::create(m_url, covered, notCovered);
    m_file->addCoverageData(f);

    covered.clear(); covered << 3 << 4;
    notCovered.clear(); notCovered << 1 << 2;
    CoveredFile* f2 = CoveredFileFactory::create(m_url, covered, notCovered);
    m_file->addCoverageData(f2);

    assertInstrumentedItem(4);
    assertSlocItem(4);
    assertCoverageItem(100.0);
    
    m_garbage << f << f2;
}


QTEST_KDEMAIN( ReportFileItemTest, NoGUI)
#include "reportfileitemtest.moc"
