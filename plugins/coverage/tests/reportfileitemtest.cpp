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
    if (m_file) delete m_file;
}

void ReportFileItemTest::construct()
{
    KVERIFY(ReportModel::File == m_file->type());
    KOMPARE(m_url, m_file->url());

    // QStandardItem properties
    KVERIFY(not m_file->isEditable());
    KVERIFY(m_file->isSelectable());
    KVERIFY(not m_file->isCheckable());

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
}


QTEST_KDEMAIN( ReportFileItemTest, NoGUI)
#include "reportfileitemtest.moc"
