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

#ifndef VERITAS_COVERAGE_REPORTMODELTEST_H
#define VERITAS_COVERAGE_REPORTMODELTEST_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <KUrl>
class QModelIndex;

namespace Veritas
{

class CoveredFile;
class ReportModel;
class ReportModelTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();
    
    void sortRole();
    void addSingleCoverageData();
    void multiCoverageOneFile();
    void differentSlocSetOneFile();
    
private:
    QVariant fetchSortData(int row);
    void assertDirAtEquals(const QModelIndex& i, QString path, int sloc, int instrumented);
    void assertFileAtEquals(const QModelIndex& i, QString name, int sloc, int instrumented, double cov);

private:
    ReportModel* m_model;
    KUrl m_fileUrl;
    QString m_fileDir;
    QString m_fileName;
};

}

#endif // VERITAS_COVERAGE_REPORTMODELTEST_H
