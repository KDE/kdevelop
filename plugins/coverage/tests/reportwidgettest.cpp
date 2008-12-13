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


#include "reportwidgettest.h"

#define protected public
#define private public
#include "../reportwidget.h"
#undef private
#undef protected

#include "../coveredfile.h"
#include "../drilldownview.h"
#include "../reportmodel.h"
#include "../reportproxymodel.h"
#include <QtTest/QTest>
#include <QtTest/QTestKeyClicksEvent>
#include <qtest_kde.h>
#include <QDir>
#include <QLabel>

using Veritas::CoveredFile;
using Veritas::DrillDownView;
using Veritas::ReportDirData;
using Veritas::ReportModel;
using Veritas::ReportProxyModel;
using Veritas::ReportWidget;
using Veritas::ReportWidgetTest;

void ReportWidgetTest::init()
{
}

void ReportWidgetTest::cleanup()
{
}

void ReportWidgetTest::someCmd()
{
    ReportWidget* rw = new ReportWidget(0);
    rw->init();
//    rw->show();
//    QTest::qWait(20000);
}

void ReportWidgetTest::setCoverageStatisticsWithRawDataFloorRoundedCoverage()
{
    ReportWidget rw(0);
    rw.init();

    ReportDirData data;
    data.setSloc(10000);
    data.setInstrumented(4223);
    rw.setCoverageStatistics(data);

    assertStatistics(rw, 10000, 4223, 42.2);
}

void ReportWidgetTest::setCoverageStatisticsWithRawDataCeilRoundedCoverage()
{
    ReportWidget rw(0);
    rw.init();

    ReportDirData data;
    data.setSloc(100000);
    data.setInstrumented(42160);
    rw.setCoverageStatistics(data);

    assertStatistics(rw, 100000, 42160, 42.2);
}

void ReportWidgetTest::setCoverageStatisticsWithNewSelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    QItemSelection selection;
    QModelIndex topLeft = table(rw)->model()->index(0, 0);
    QModelIndex bottomRight = table(rw)->model()->index(3, 1);
    selection.select(topLeft, bottomRight);

    rw.setCoverageStatistics(selection, QItemSelection());

    assertStatistics(rw, 8, 4, 50.0);
}

void ReportWidgetTest::setCoverageStatisticsWithAddedSelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    setStatistics(rw, 12, 12);

    QItemSelection selection;
    QModelIndex first = table(rw)->model()->index(0, 0);
    QModelIndex second = table(rw)->model()->index(1, 0);
    selection.select(first, second);

    rw.setCoverageStatistics(selection, QItemSelection());

    assertStatistics(rw, 16, 15, 93.8);
}

void ReportWidgetTest::setCoverageStatisticsWithRemovedSelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    setStatistics(rw, 8, 6);

    QItemSelection deselection;
    QModelIndex second = table(rw)->model()->index(1, 0);
    QModelIndex third = table(rw)->model()->index(2, 0);
    deselection.select(second, third);

    rw.setCoverageStatistics(QItemSelection(), deselection);

    assertStatistics(rw, 4, 4, 100.0);
}

void ReportWidgetTest::setCoverageStatisticsWithChangedSelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    setStatistics(rw, 16, 15);

    QItemSelection selection;
    QModelIndex first = table(rw)->model()->index(0, 0);
    selection.select(first, first);

    QItemSelection deselection;
    QModelIndex fourth = table(rw)->model()->index(3, 0);
    deselection.select(fourth, fourth);

    rw.setCoverageStatistics(selection, deselection);

    assertStatistics(rw, 18, 17, 94.4);
}

void ReportWidgetTest::setCoverageStatisticsWithEmptySelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    rw.setCoverageStatistics(QItemSelection(), QItemSelection());

    assertEmptyStatistics(rw);
}

void ReportWidgetTest::setCoverageStatisticsWithInvalidSelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    QItemSelection selection;
    QModelIndex index;
    selection.select(index, index);

    rw.setCoverageStatistics(selection, QItemSelection());

    assertEmptyStatistics(rw);
}

//Shouldn't happen, but just in case... it should ignore file items and only
//take into account directory items
void ReportWidgetTest::setCoverageStatisticsWithDirectoriesAndFileSelection()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    QItemSelection selection;
    QModelIndex first = table(rw)->model()->index(0, 0);
    selection.select(first, first);

    QModelIndex third = table(rw)->model()->index(2, 1);
    QModelIndex thirdFile1 = table(rw)->model()->index(0, 0, third);
    selection.select(thirdFile1, thirdFile1);

    QModelIndex fourth = table(rw)->model()->index(3, 0);
    selection.select(fourth, fourth);

    rw.setCoverageStatistics(selection, QItemSelection());

    assertStatistics(rw, 4, 2, 50.0);
}

void ReportWidgetTest::setCoverageStatisticsWithIndex()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    rw.setCoverageStatistics(table(rw)->model()->index(0, 0));

    assertStatistics(rw, 3, 2, 66.7);
}

void ReportWidgetTest::setCoverageStatisticsWithInvalidIndex()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    rw.setCoverageStatistics(QModelIndex());

    assertEmptyStatistics(rw);
}

//Shouldn't modify the statistics
void ReportWidgetTest::setCoverageStatisticsWithFileIndex()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    QModelIndex dirIndex = table(rw)->model()->index(0, 0);
    rw.setCoverageStatistics(table(rw)->model()->index(0, 0, dirIndex));

    assertEmptyStatistics(rw);
}

//Selects first directory
void ReportWidgetTest::statisticsSelectingSingleDirectory()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    //TODO I can't get it to work without showing the widget and waiting some
    //time before triggering the click. Is there a way for this to work without
    //showing the widget and waiting?
    rw.show();
    QTest::keyClick(table(rw), Qt::Key_Space, Qt::NoModifier, 500);

    assertStatistics(rw, 3, 2, 66.7);
}

//Selects all directories
void ReportWidgetTest::statisticsSelectingSeveralDirectories()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    rw.show();
    QTest::keyClick(table(rw), Qt::Key_Space, Qt::NoModifier, 500);
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::ShiftModifier);
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::ShiftModifier);
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::ShiftModifier);

    assertStatistics(rw, 8, 4, 50.0);
}

//Selects first and second directory, and slides in second directory
void ReportWidgetTest::statisticsAfterSlidingRight()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    rw.show();
    QTest::keyClick(table(rw), Qt::Key_Space, Qt::NoModifier, 500);
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::ShiftModifier);

    QTest::keyClick(table(rw), Qt::Key_Right);

    assertStatistics(rw, 1, 1, 100.0);
}

//Selects fourth, third and second directory, slides in second directory, 
//selects first file, slides left, and selects third directory
void ReportWidgetTest::statisticsAfterSlidingLeft()
{
    ReportWidget rw(0);
    rw.init();
    setModelFor(rw);

    rw.show();
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::NoModifier, 500);
    QTest::keyClick(table(rw), Qt::Key_Down);
    QTest::keyClick(table(rw), Qt::Key_Down);
    QTest::keyClick(table(rw), Qt::Key_Up, Qt::ShiftModifier);
    QTest::keyClick(table(rw), Qt::Key_Up, Qt::ShiftModifier);

    QTest::keyClick(table(rw), Qt::Key_Right);

    //Wait to avoid the slide to eat the event
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::NoModifier, 500);

    assertStatistics(rw, 1, 1, 100.0);

    QTest::keyClick(table(rw), Qt::Key_Left);

    assertStatistics(rw, 5, 2, 40.0);

    //Wait to avoid the slide to eat the event
    QTest::keyClick(table(rw), Qt::Key_Down, Qt::NoModifier, 500);

    assertStatistics(rw, 3, 1, 33.3);
}

////////////////////////////// Asserts ////////////////////////////////////////

void ReportWidgetTest::assertStatistics(const ReportWidget& rw, int sloc,
                                        int instrumented, double coverage)
{
    QCOMPARE(rw.m_sloc->text(), QString::number(sloc));
    QCOMPARE(rw.m_instrumented->text(), QString::number(instrumented));
    QCOMPARE(rw.m_coverage->text(), QLocale().toString(coverage, 'f', 1) + " %");
}

void ReportWidgetTest::assertEmptyStatistics(const ReportWidget& rw)
{
    QCOMPARE(rw.m_sloc->text(), QString("-"));
    QCOMPARE(rw.m_instrumented->text(), QString("-"));
    QCOMPARE(rw.m_coverage->text(), QString("-"));
}

////////////////////////////// Helpers ////////////////////////////////////////

DrillDownView* ReportWidgetTest::table(const ReportWidget& rw)
{
    return rw.table();
}

void ReportWidgetTest::setStatistics(ReportWidget& rw, int sloc, int instrumented)
{
    rw.m_sloc->setText(QString::number(sloc));
    rw.m_instrumented->setText(QString::number(instrumented));
}

void ReportWidgetTest::setModelFor(ReportWidget& rw)
{
    ReportModel* model = createReportModel();
    ReportProxyModel* proxyModel = new ReportProxyModel(0);
    proxyModel->setSourceModel(model);

    rw.table()->setModel(proxyModel);
    rw.m_proxy = proxyModel;
    rw.m_model = model;

    connect(rw.table()->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            &rw,
            SLOT(dispatchSelectionSignal(QItemSelection,QItemSelection)));
}

// dir1: sloc 3, instrumented 2, coverage 66.6 %
// dir1File1: sloc 3, instrumented 2, coverage 66.6 %
// dir2: sloc 1, instrumented 1, coverage 100.0 %
// dir2File1: sloc 1, instrumented 1, coverage 100.0 %
// dir3: sloc 3, instrumented 1, coverage 33.3 %
// dir3File1: sloc 3, instrumented 1, coverage 33.3 %
// dir4: sloc 1, instrumented 0, coverage 0.0 %
// dir4File1: sloc 1, instrumented 0, coverage 0.0 %
ReportModel* ReportWidgetTest::createReportModel()
{
    ReportModel* model = new ReportModel(0);

    CoveredFile dir1File1;
    dir1File1.setUrl(KUrl("/dir1/file1"));
    dir1File1.setCallCount(4, 1);
    dir1File1.setCallCount(8, 0);
    dir1File1.setCallCount(15, 1);
    model->addCoverageData(&dir1File1);

    CoveredFile dir2File1;
    dir2File1.setUrl(KUrl("/dir2/file1"));
    dir2File1.setCallCount(16, 1);
    model->addCoverageData(&dir2File1);

    CoveredFile dir3File1;
    dir3File1.setUrl(KUrl("/dir2/dir3/file1"));
    dir3File1.setCallCount(23, 0);
    dir3File1.setCallCount(42, 0);
    dir3File1.setCallCount(108, 1);
    model->addCoverageData(&dir3File1);

    CoveredFile dir4File1;
    dir4File1.setUrl(KUrl("/dir2/dir4/file1"));
    dir4File1.setCallCount(815, 0);
    model->addCoverageData(&dir4File1);

    return model;
}

QTEST_KDEMAIN( ReportWidgetTest, GUI )
#include "reportwidgettest.moc"
