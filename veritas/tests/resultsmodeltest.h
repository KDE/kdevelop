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

#ifndef VERITAS_RESULTSMODELTEST_H
#define VERITAS_RESULTSMODELTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "../testresult.h"

namespace Veritas
{
class ResultsModel;
class Test;

class ResultsModelTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testFromIndex();

    void addSingleResult();
    void addMultipleResults();
    void addSubResult();
    void fetchDataFromEmpty();
    void fetchDataIllegalRow();
    void fetchDataNoOwner();
    void clear();
    void constructEmpty();
    void addNullResult();

private:
    void assertColumnHeader(const QVariant& expected, int index);
    void assertRowDataEquals(ResultsModel* model, int rowNumber, TestResult* expected);
    void assertNrofItemsEquals(int expected, ResultsModel* model);
    TestResult* createTestResult(TestState state, const QString& testName, const KUrl& location, int line, const QString& message);

private:
    ResultsModel* m_model;
    QList<Test*> m_garbage;
};

}


#endif // VERITAS_RESULTSMODELTEST_H
