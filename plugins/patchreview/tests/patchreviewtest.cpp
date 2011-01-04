/*
 * This file is part of KDevelop
 * Copyright 2011 Dmitry Risenberg <dmitry.risenberg@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "patchreviewtest.h"

#include <QTest>
#include "../libdiff2/diffmodel.h"
#include "../libdiff2/parser.h"

using namespace Diff2;

typedef QHash<int, QPair<QStringList, QStringList> > DifferenceHash;
Q_DECLARE_METATYPE(DifferenceHash);
typedef QHash<int, QPair<int, int> > LineNumberHash;
Q_DECLARE_METATYPE(LineNumberHash);

void PatchReviewTest::CompareDifferenceStringList(const DifferenceStringList& actual, const QStringList& expected)
{
    DifferenceStringListConstIterator actualIter;
    QStringList::const_iterator expectedIter;
    for(actualIter = actual.constBegin(), expectedIter = expected.constBegin(); actualIter != actual.constEnd() && expectedIter != expected.constEnd(); ++actualIter, ++expectedIter) {
        QCOMPARE((*actualIter)->string(), *expectedIter);
    }
    if (actualIter != actual.constEnd()) {
        QFAIL(QString("Actual has too many items, starting with '%1', line %2").arg((*actualIter)->string()).arg(actualIter - actual.constBegin()).toAscii());
    }
    if (expectedIter != expected.constEnd()) {
        QFAIL(QString("Actual has too few items, no match for '%1', line %2").arg(*expectedIter).arg(expectedIter - expected.constBegin()).toAscii());
    }
}

// The most basic test - something is actually working
void PatchReviewTest::testOneLineChange()
{
    DiffModel* model = new DiffModel();
    QStringList newLines;
    newLines << "newline\n";
    QStringList oldLines;
    oldLines << "oldline\n";
    model->linesChanged(oldLines, newLines, 2);
    QCOMPARE(model->differences()->size(), 1);

    QCOMPARE(model->differenceCount(), 1);
    const Difference* diff = model->differenceAt(0);
    CompareDifferenceStringList(diff->sourceLines(), oldLines);
    CompareDifferenceStringList(diff->destinationLines(), newLines);
    QCOMPARE(diff->type(), int(Difference::Change));
}

void PatchReviewTest::testDifferenceContents()
{
    QFETCH(QStringList, patch);
    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);

    QFETCH(QStringList, oldLines);
    QFETCH(QStringList, newLines);
    QFETCH(int, editLineNumber);
    model->linesChanged(oldLines, newLines, editLineNumber);
    QFETCH(int, expectedDifferenceCount);
    QCOMPARE(model->differenceCount(), expectedDifferenceCount);

    QFETCH(DifferenceHash, expectedDifferences);
    for (DifferenceHash::ConstIterator iter = expectedDifferences.begin(); iter != expectedDifferences.end(); ++iter) {
        const Difference* diff = model->differenceAt(iter.key());
        CompareDifferenceStringList(diff->sourceLines(), iter.value().first);
        CompareDifferenceStringList(diff->destinationLines(), iter.value().second);
    }
}

void PatchReviewTest::testDifferenceContents_data()
{
    QTest::addColumn<QStringList>("patch");
    QTest::addColumn<QStringList>("oldLines");     // lines that are replaced
    QTest::addColumn<QStringList>("newLines");     // replacement lines
    QTest::addColumn<int>("editLineNumber");
    QTest::addColumn<int>("expectedDifferenceCount");
    QTest::addColumn<DifferenceHash>("expectedDifferences");

    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,5 +1,5 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        " efgh\n" <<
        "-delete2\n" <<
        "+insert2\n" <<
        " ijkl\n";

        QStringList newLines;
        newLines << "newline1\n" << "newline2\n";
        QStringList oldLines;
        oldLines << "efgh\n";
        QStringList sourceLines;
        sourceLines << "delete1\n" << "efgh\n" << "delete2\n";
        QStringList destinationLines;
        destinationLines << "insert1\n" << "newline1\n" << "newline2\n" << "insert2\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(0, qMakePair(sourceLines, destinationLines));

        QTest::newRow("Merge adjacent differences") << patch << oldLines << newLines << 3 << 1 << expectedDifferences;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,3 +1,3 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "delete1\n";
        QStringList oldLines;
        oldLines << "insert1\n";
        QTest::newRow("Revert existing difference") << patch << oldLines << newLines << 2 << 0 << DifferenceHash();
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,4 +1,5 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        "+insert3\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "delete2\n";
        QStringList oldLines;
        oldLines << "insert2\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(0, qMakePair(QStringList() << "delete1\n", QStringList() << "insert1\n"));
        expectedDifferences.insert(1, qMakePair(QStringList(), QStringList() << "insert3\n"));

        QTest::newRow("Partial reversion") << patch << oldLines << newLines << 3 << 2 << expectedDifferences;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,4 +1,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n" << "newline2\n";
        QStringList oldLines;
        oldLines << "abcd\n" << "insert1\n" << "insert2\n" << "efgh\n";
        QStringList sourceLines;
        sourceLines << "abcd\n" << "delete1\n" << "delete2\n" << "efgh\n";
        QStringList destinationLines;
        destinationLines << "newline1\n" << "newline2\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(0, qMakePair(sourceLines, destinationLines));

        // The first exisiting difference inside the edit
        QTest::newRow("First inside") << patch << oldLines << newLines << 1 << 1 << expectedDifferences;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,4 +1,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n" << "newline2\n";
        QStringList oldLines;
        oldLines << "insert2\n" << "efgh\n";
        QStringList sourceLines;
        sourceLines << "delete1\n" << "delete2\n" << "efgh\n";
        QStringList destinationLines;
        destinationLines << "insert1\n" << "newline1\n" << "newline2\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(0, qMakePair(sourceLines, destinationLines));

        // The first existing difference intersects with the edit
        QTest::newRow("First intersects") << patch << oldLines << newLines << 3 << 1 << expectedDifferences;
    }
}

void PatchReviewTest::testLineNumbers_data()
{
    QTest::addColumn<QStringList>("patch");
    QTest::addColumn<QStringList>("oldLines");     // lines that are replaced
    QTest::addColumn<QStringList>("newLines");     // replacement lines
    QTest::addColumn<int>("editLineNumber");
    QTest::addColumn<int>("expectedDifferenceCount");
    QTest::addColumn<LineNumberHash>("expectedLineNumbers");

    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,4 +1,6 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        "+insert3\n" <<
        "+insert4\n" <<
        " efgh\n" <<
        "@@ -15,3 +17,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n" << "newline2\n" << "newline2\n";
        QStringList oldLines;
        oldLines << "oldline1\n";
        LineNumberHash expectedLineNumbers;
        expectedLineNumbers.insert(0, qMakePair(2, 2));
        expectedLineNumbers.insert(2, qMakePair(16, 20));
        QTest::newRow("Update existing line numbers") << patch << oldLines << newLines << 10 << 3 << expectedLineNumbers;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,4 +1,6 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        "+insert3\n" <<
        "+insert4\n" <<
        " efgh\n" <<
        "@@ -15,3 +17,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n";
        QStringList oldLines;
        oldLines << "oldline1\n";
        LineNumberHash expectedLineNumbers;
        expectedLineNumbers.insert(2, qMakePair(22, 25));

        // Line numbers assigned to new difference when it is inserted after all existing differences
        QTest::newRow("Last edit line number") << patch << oldLines << newLines << 25 << 3 << expectedLineNumbers;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,4 +1,6 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        "+insert3\n" <<
        "+insert4\n" <<
        " efgh\n" <<
        "@@ -15,3 +17,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n";
        QStringList oldLines;
        oldLines << "oldline1\n";
        LineNumberHash expectedLineNumbers;
        expectedLineNumbers.insert(1, qMakePair(11, 13));

        // Line numbers assigned to new difference when it is inserted between existing differences
        QTest::newRow("Middle edit line number") << patch << oldLines << newLines << 13 << 3 << expectedLineNumbers;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -10,4 +10,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "-delete2\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n";
        QStringList oldLines;
        oldLines << "oldline1\n";
        LineNumberHash expectedLineNumbers;
        expectedLineNumbers.insert(0, qMakePair(5, 5));

        // Line numbers assigned to new difference when it is inserted before all existing differences
        QTest::newRow("First edit line number") << patch << oldLines << newLines << 5 << 2 << expectedLineNumbers;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,3 +1,4 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        " efgh\n" <<
        "@@ -11,4 +12,5 @@\n" <<
        " abcd\n" <<
        "-delete2\n" <<
        "-delete3\n" <<
        "+insert3\n" <<
        "+insert4\n" <<
        "+insert5\n" <<
        " efgh\n" <<
        "@@ -21,4 +23,3 @@\n" <<
        " abcd\n" <<
        "-delete4\n" <<
        "-delete5\n" <<
        "+insert6\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "delete2\n";
        QStringList oldLines;
        oldLines << "insert4\n";
        LineNumberHash expectedLineNumbers;
        expectedLineNumbers.insert(0, qMakePair(2, 2));
        expectedLineNumbers.insert(1, qMakePair(12, 13));
        expectedLineNumbers.insert(2, qMakePair(13, 15));
        expectedLineNumbers.insert(3, qMakePair(22, 24));

        QTest::newRow("Partial reversion") << patch << oldLines << newLines << 14 << 4 << expectedLineNumbers;
    }
}

void PatchReviewTest::testLineNumbers()
{
    kWarning() << "OLOLO";
    QFETCH(QStringList, patch);
    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);

    QFETCH(QStringList, oldLines);
    QFETCH(QStringList, newLines);
    QFETCH(int, editLineNumber);
    model->linesChanged(oldLines, newLines, editLineNumber);
    QFETCH(int, expectedDifferenceCount);
    QCOMPARE(model->differenceCount(), expectedDifferenceCount);

    QFETCH(LineNumberHash, expectedLineNumbers);
    for (LineNumberHash::ConstIterator iter = expectedLineNumbers.begin(); iter != expectedLineNumbers.end(); ++iter) {
        const Difference* diff = model->differenceAt(iter.key());
        QCOMPARE(diff->sourceLineNumber(), iter.value().first);
        QCOMPARE(diff->destinationLineNumber(), iter.value().second);
    }
}

QTEST_MAIN(PatchReviewTest);
