#include "reportitemstest.h"
#include "coveredfilefactory.h"
#include "../coveredfile.h"
#include "../reportmodel.h"

#include "testutils.h"

using Veritas::CoveredFile;
using Veritas::ReportItemsTest;
using Veritas::ReportValueItem;
using Veritas::ReportDirData;
using Veritas::ReportDirItem;
using Veritas::ReportFileItem;
using Veritas::ReportModel;
using Veritas::Test::CoveredFileFactory;

///////////////////////////// fixture ////////////////////////////////////////

void ReportItemsTest::init()
{}

void ReportItemsTest::cleanup()
{
    qDeleteAll(m_garbage);
    m_garbage.clear();
    qDeleteAll(m_garbageFiles);
    m_garbageFiles.clear();
}

///////////////////////////// commands ////////////////////////////////////////

void ReportItemsTest::constructDoubleValueItem()
{
    ReportValueItem doubleItem(5.0);
    KVERIFY(ReportModel::Value == doubleItem.type());
    KOMPARE(5.0, doubleItem.value());
    KOMPARE(QString("5.0"), doubleItem.data(Qt::DisplayRole));
    KVERIFY(! doubleItem.isEditable());
    KVERIFY(! doubleItem.isSelectable());
    KVERIFY(! doubleItem.isCheckable());
}

void ReportItemsTest::constructIntValueItem()
{
    ReportValueItem intItem(5);
    KVERIFY(ReportModel::Value == intItem.type());
    KOMPARE(5, intItem.value());
    KOMPARE(QString("5"), intItem.data(Qt::DisplayRole));
    KVERIFY(! intItem.isEditable());
    KVERIFY(! intItem.isSelectable());
    KVERIFY(! intItem.isCheckable());
}

void ReportItemsTest::constructDirItem()
{
    ReportDirItem dir("/my/dir");
    KVERIFY(ReportModel::Dir == dir.type());
    KOMPARE(0, dir.sloc());
    KOMPARE(0, dir.instrumented());
    KOMPARE(0, dir.coverage());
    KVERIFY(! dir.isEditable());
    KVERIFY(dir.isSelectable());
    KVERIFY(! dir.isCheckable());
}

void ReportItemsTest::addCoverageDataTo(ReportDirItem& dir, const QString& path, int sloc, int instrumented)
{
    CoveredFile* f = CoveredFileFactory::create(KUrl(path), sloc, instrumented);
    ReportFileItem* fItem = new ReportFileItem(KUrl(path));
    fItem->addCoverageData(f);
    dir.appendRow(fItem);
    dir.updateStats();
    m_garbage << fItem->coverageItem() << fItem->slocItem() << fItem->instrumentedItem();
    m_garbageFiles << f;
}

void ReportItemsTest::addFileToDirItem()
{
    ReportDirItem* dir = new ReportDirItem("/my/dir");
    addCoverageDataTo(*dir, "/my/dir/foo.cpp", 10, 5);
    KOMPARE(10, dir->sloc());
    KOMPARE(5, dir->instrumented());
    KVERIFY(qAbs(100*5/10 - dir->coverage()) < 0.1);
    delete dir;
}

void ReportItemsTest::addMultipleFilesToDir()
{
    ReportDirItem* dir = new ReportDirItem("/my/dir");
    addCoverageDataTo(*dir, "/my/dir/foo.cpp", 10, 5);
    addCoverageDataTo(*dir, "/my/dir/bar.cpp", 20, 5);
    KOMPARE(10+20, dir->sloc());
    KOMPARE(5+5, dir->instrumented());
    KVERIFY(qAbs(100*((double)dir->instrumented()/dir->sloc()) - dir->coverage()) < 0.1);
    delete dir;
}

void ReportItemsTest::constructDirData()
{
    ReportDirData dirData;
    KOMPARE(0, dirData.sloc());
    KOMPARE(0, dirData.instrumented());
}

void ReportItemsTest::dirDataSetSloc()
{
    ReportDirData dirData;
    dirData.setSloc(16);
    KOMPARE(16, dirData.sloc());
}

void ReportItemsTest::dirDataSetInstrumented()
{
    ReportDirData dirData;
    dirData.setInstrumented(23);
    KOMPARE(23, dirData.instrumented());
}

void ReportItemsTest::dirDataCoverage()
{
    ReportDirData dirData;
    KOMPARE(0, dirData.coverage());

    dirData.setInstrumented(21);
    dirData.setSloc(42);
    KOMPARE(50, dirData.coverage());

    dirData.setInstrumented(4);
    dirData.setSloc(2500);
    QCOMPARE(0.16, dirData.coverage());
}

QTEST_KDEMAIN( ReportItemsTest, NoGUI)
#include "reportitemstest.moc"
