#include "reportitemstest.h"
#include <qtest_kde.h>
#include <veritas/tests/kasserts.h>
#include "coveredfilefactory.h"
#include "../coveredfile.h"
#include "../reportmodel.h"

using Veritas::CoveredFile;
using Veritas::ReportItemsTest;
using Veritas::ReportValueItem;
using Veritas::ReportDirItem;
using Veritas::ReportFileItem;
using Veritas::ReportModel;
using Veritas::Test::CoveredFileFactory;

///////////////////////////// fixture ////////////////////////////////////////

void ReportItemsTest::init()
{}

void ReportItemsTest::cleanup()
{}

///////////////////////////// commands ////////////////////////////////////////

void ReportItemsTest::constructDoubleValueItem()
{
    ReportValueItem doubleItem(5.0);
    KVERIFY(ReportModel::Value == doubleItem.type());
    KOMPARE(5.0, doubleItem.value());
    KOMPARE(QString("5.0"), doubleItem.data(Qt::DisplayRole));
    KVERIFY(not doubleItem.isEditable());
    KVERIFY(not doubleItem.isSelectable());
    KVERIFY(not doubleItem.isCheckable());
}

void ReportItemsTest::constructIntValueItem()
{
    ReportValueItem intItem(5);
    KVERIFY(ReportModel::Value == intItem.type());
    KOMPARE(5, intItem.value());
    KOMPARE(QString("5"), intItem.data(Qt::DisplayRole));
    KVERIFY(not intItem.isEditable());
    KVERIFY(not intItem.isSelectable());
    KVERIFY(not intItem.isCheckable());
}

void ReportItemsTest::constructDirItem()
{
    ReportDirItem dir("/my/dir");
    KVERIFY(ReportModel::Dir == dir.type());
    KOMPARE(0, dir.sloc());
    KOMPARE(0, dir.instrumented());
    KOMPARE(0, dir.coverage());
    KVERIFY(not dir.isEditable());
    KVERIFY(dir.isSelectable());
    KVERIFY(not dir.isCheckable());
}

void addCoverageDataTo(ReportDirItem& dir, const QString& path, int sloc, int instrumented)
{
    CoveredFile* f = CoveredFileFactory::create(KUrl(path), sloc, instrumented);
    ReportFileItem* fItem = new ReportFileItem(KUrl(path));
    fItem->addCoverageData(f);
    dir.appendRow(fItem);
    dir.updateStats();
}

void ReportItemsTest::addFileToDirItem()
{
    ReportDirItem dir("/my/dir");
    addCoverageDataTo(dir, "/my/dir/foo.cpp", 10, 5);
    KOMPARE(10, dir.sloc());
    KOMPARE(5, dir.instrumented());
    KVERIFY(qAbs(100*5/10 - dir.coverage()) < 0.1);
}

void ReportItemsTest::addMultipleFilesToDir()
{
    ReportDirItem dir("/my/dir");
    addCoverageDataTo(dir, "/my/dir/foo.cpp", 10, 5);
    addCoverageDataTo(dir, "/my/dir/bar.cpp", 20, 5);
    KOMPARE(10+20, dir.sloc());
    KOMPARE(5+5, dir.instrumented());
    KVERIFY(qAbs(100*((double)dir.instrumented()/dir.sloc()) - dir.coverage()) < 0.1);
}

QTEST_KDEMAIN( ReportItemsTest, NoGUI )
#include "reportitemstest.moc"
