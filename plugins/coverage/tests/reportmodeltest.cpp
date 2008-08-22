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

#include "reportmodeltest.h"
#include <qtest_kde.h>
#include <veritas/tests/kasserts.h>
#include "../reportmodel.h"
#include "../coveredfile.h"
#include "coveredfilefactory.h"

using Veritas::CoveredFile;
using Veritas::ReportModelTest;
using Veritas::ReportModel;
using Veritas::ReportFileItem;
using Veritas::ReportDirItem;
using Veritas::ReportValueItem;
using Veritas::Test::CoveredFileFactory;

////////////////////////// fixture ///////////////////////////////////////////

void ReportModelTest::init()
{
    m_model = new ReportModel;
    KUrl m_fileUrl = KUrl("/path/to/foo.cpp");
    QString m_fileDir = "/path/to/";
    QString m_fileName = "foo.cpp";
}

void ReportModelTest::cleanup()
{
    if (m_model) delete m_model;
}

////////////////////////// commands ///////////////////////////////////////////

void ReportModelTest::sortRole()
{
    ReportDirItem* dir = new ReportDirItem(m_fileDir);
    ReportFileItem* file = new ReportFileItem(m_fileUrl);
    ReportValueItem* val = new ReportValueItem(10);
    m_model->appendRow(dir);
    m_model->appendRow(file);
    m_model->appendRow(val);

    KOMPARE(m_fileDir, fetchSortData(0));
    KOMPARE(m_fileName, fetchSortData(1));
    KOMPARE(10.0, fetchSortData(2));
}

void ReportModelTest::addSingleCoverageData()
{
    CoveredFile* f = CoveredFileFactory::create(m_fileUrl, 10, 5);
    m_model->addCoverageData(f);

    QModelIndex dirIndex = m_model->index(0,0);
    int expSloc = 10;
    int expInstrumented = 5;
    assertDirAtEquals(dirIndex, m_fileDir, expSloc, expInstrumented); 

    QModelIndex fileIndex = m_model->index(0,0, dirIndex);
    double expCoverage = 50.0;
    assertFileAtEquals(fileIndex, m_fileName, expSloc, expInstrumented, expCoverage);
    
    KVERIFY(!m_model->index(1,0).isValid());
    KVERIFY(!m_model->index(1,0, dirIndex).isValid());
}

void ReportModelTest::multiCoverageOneFile()
{
    // The parser can emit multiple CoveredFile instances
    // for a single URL [covered by multiple 'tests']
    // The model aggregates this data.
    QList<int> covered; covered << 1 << 2;
    QList<int> notCovered; notCovered << 3 << 4;
    CoveredFile* f1 = CoveredFileFactory::create(m_fileUrl, covered, notCovered);

    covered.clear(); covered << 4;
    notCovered.clear(); notCovered << 1 << 2 << 3;
    CoveredFile* f2 = CoveredFileFactory::create(m_fileUrl, covered, notCovered);
    
    m_model->addCoverageData(f1);
    m_model->addCoverageData(f2);
    
    QModelIndex dirIndex = m_model->index(0,0);
    int expSloc = 4;
    int expInstrumented = 2 + 1;
    assertDirAtEquals(dirIndex, m_fileDir, expSloc, expInstrumented); 

    QModelIndex fileIndex = m_model->index(0,0,dirIndex);
    double expCoverage = 100*((double)expInstrumented/expSloc);
    assertFileAtEquals(fileIndex, m_fileName, expSloc, expInstrumented, expCoverage);
    
    KVERIFY(!m_model->index(1,0).isValid());
    KVERIFY(!m_model->index(1,0, dirIndex).isValid());
}

void ReportModelTest::differentSlocSetOneFile()
{
    // It can happen that a file has different sets of reachable
    // lines of code associated with it over multiple coverage
    // data parses. For example on inline'd functions of header
    // files. For such a case the SLOC should be adjust accordingly.
    QList<int> covered; covered << 1;
    QList<int> notCovered; notCovered << 2;
    CoveredFile* f1 = CoveredFileFactory::create(m_fileUrl, covered, notCovered);

    covered.clear(); covered << 2;
    notCovered.clear(); notCovered << 3;
    CoveredFile* f2 = CoveredFileFactory::create(m_fileUrl, covered, notCovered);
    
    m_model->addCoverageData(f1);
    m_model->addCoverageData(f2);
    
    QModelIndex dirIndex = m_model->index(0,0);
    int expSloc = 3; // lines 1,2,3
    int expInstrumented = 2; // lines 1 and 2
    assertDirAtEquals(dirIndex, m_fileDir, expSloc, expInstrumented); 

    QModelIndex fileIndex = m_model->index(0,0,dirIndex);
    double expCoverage = 100*((double)expInstrumented/expSloc);
    assertFileAtEquals(fileIndex, m_fileName, expSloc, expInstrumented, expCoverage);
}

///////////////////////// helpers ////////////////////////////////////////////

QVariant ReportModelTest::fetchSortData(int row)
{
   return m_model->index(row, 0).data(ReportModel::SortRole);
}

//////////////////////// custom assertions ///////////////////////////////////

void ReportModelTest::assertDirAtEquals(const QModelIndex& i, QString path, int sloc, int instrumented)
{
    KVERIFY(i.isValid());
    QStandardItem* sdir = m_model->itemFromIndex(i);
    KVERIFY(ReportModel::Dir == sdir->type());
    ReportDirItem* dir = dynamic_cast<ReportDirItem*>(sdir);
    KVERIFY(dir);
    QString actualPath = dir->data(Qt::DisplayRole).toString();
    KOMPARE_MSG(path, actualPath, 
                QString("Wrong display data for diritem: expected: %1 but got %2.").
                arg(path).arg(actualPath));
    KOMPARE(sloc, dir->sloc());
    KOMPARE(instrumented, dir->instrumented());

}

void ReportModelTest::assertFileAtEquals(const QModelIndex& i, QString name, int sloc, int instrumented, double cov)
{
    KVERIFY(i.isValid());
    QStandardItem* sfile = m_model->itemFromIndex(i);
    KVERIFY(ReportModel::File == sfile->type());
    ReportFileItem* file = dynamic_cast<ReportFileItem*>(sfile);
    KVERIFY(file);
    KOMPARE(QVariant(name), file->data(Qt::DisplayRole));
    ReportValueItem* slocItem = file->slocItem();
    KOMPARE((double)sloc, slocItem->value());
    ReportValueItem* instrumentedItem = file->instrumentedItem();
    KOMPARE((double)instrumented, instrumentedItem->value());
    ReportValueItem* coverageItem = file->coverageItem();
    KVERIFY(qAbs(cov - coverageItem->value()) < 0.1);
}

QTEST_KDEMAIN( ReportModelTest, NoGUI)
#include "reportmodeltest.moc"
