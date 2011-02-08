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

#include "interactivedifftest.h"

#include <QTest>
#include "../diffmodel.h"
#include "../parser.h"

using namespace Diff2;

typedef QHash<int, QPair<QStringList, QStringList> > DifferenceHash;
Q_DECLARE_METATYPE(DifferenceHash);
typedef QHash<int, QPair<int, int> > LineNumberHash;
Q_DECLARE_METATYPE(LineNumberHash);

void InteractiveDiffTest::CompareDifferenceStringList(const DifferenceStringList& actual, const QStringList& expected)
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
void InteractiveDiffTest::testOneLineChange()
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

void InteractiveDiffTest::testSameLine()
{
    DiffModel* model = new DiffModel();
    QStringList newLines;
    newLines << "oldline2\n";
    QStringList oldLines;
    oldLines << "oldline1\n" << "oldline2\n";
    model->linesChanged(oldLines, newLines, 2);

    QCOMPARE(model->differenceCount(), 1);
    const Difference* diff = model->differenceAt(0);
    CompareDifferenceStringList(diff->sourceLines(), QStringList() << "oldline1\n");
    CompareDifferenceStringList(diff->destinationLines(), QStringList());
    QCOMPARE(diff->type(), int(Difference::Delete));
}

void InteractiveDiffTest::testDifferenceContents()
{
    QFETCH(QStringList, patch);
    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);

    QFETCH(QStringList, oldLines);
    QFETCH(QStringList, newLines);
    QFETCH(int, editLineNumber);
    QFETCH(bool, isAlreadyApplied);
    model->applyAllDifferences(isAlreadyApplied);
    model->linesChanged(oldLines, newLines, editLineNumber);
    QFETCH(int, expectedDifferenceCount);
    QCOMPARE(model->differenceCount(), expectedDifferenceCount);

    QFETCH(DifferenceHash, expectedDifferences);
    for (DifferenceHash::ConstIterator iter = expectedDifferences.constBegin(); iter != expectedDifferences.constEnd(); ++iter) {
        const Difference* diff = model->differenceAt(iter.key());
        CompareDifferenceStringList(diff->sourceLines(), iter.value().first);
        CompareDifferenceStringList(diff->destinationLines(), iter.value().second);
    }
}

void InteractiveDiffTest::testDifferenceContents_data()
{
    QTest::addColumn<QStringList>("patch");
    QTest::addColumn<QStringList>("oldLines");     // lines that are replaced
    QTest::addColumn<QStringList>("newLines");     // replacement lines
    QTest::addColumn<int>("editLineNumber");
    QTest::addColumn<bool>("isAlreadyApplied");
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

        QTest::newRow("Merge adjacent differences") << patch << oldLines << newLines << 3 << true << 1 << expectedDifferences;
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
        newLines << "newline1\n";
        QStringList oldLines;
        oldLines << "efgh\n";
        QStringList sourceLines;
        sourceLines << "delete1\n" << "delete2\n" << "efgh\n";
        QStringList destinationLines;
        destinationLines << "insert1\n" << "insert2\n" << "newline1\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(0, qMakePair(sourceLines, destinationLines));

        // Append a line to a multiline diff
        QTest::newRow("Append multiline") << patch << oldLines << newLines << 4 << true << 1 << expectedDifferences;
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
        QTest::newRow("Revert existing difference") << patch << oldLines << newLines << 2 << true << 0 << DifferenceHash();
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,3 +1,2 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "abcd\n" << "delete1\n";
        QStringList oldLines;
        oldLines << "abcd\n";
        QTest::newRow("Revert deletion") << patch << oldLines << newLines << 1 << true << 0 << DifferenceHash();
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

        QTest::newRow("Partial reversion") << patch << oldLines << newLines << 3 << true << 2 << expectedDifferences;
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
        QTest::newRow("First inside") << patch << oldLines << newLines << 1 << true << 1 << expectedDifferences;
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
        QTest::newRow("First intersects") << patch << oldLines << newLines << 3 << true << 1 << expectedDifferences;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,2 +1,3 @@\n" <<
        " abcd\n" <<
        "+\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "a\n";
        QStringList oldLines;
        oldLines << "\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(0, qMakePair(QStringList(), QStringList() << "a\n"));

        QTest::newRow("Replace empty line") << patch << oldLines << newLines << 2 << true << 1 << expectedDifferences;
    }
    {
        QStringList patch;
        patch <<
        "--- file1\t2011-01-01 20:23:45.000000000 +0300\n" <<
        "+++ file2\t2011-01-01 20:24:02.000000000 +0300\n" <<
        "@@ -1,3 +1,3 @@\n" <<
        " abcd\n" <<
        "+insert1\n" <<
        "+insert2\n" <<
        "+insert3\n" <<
        "+insert4\n" <<
        "+insert5\n" <<
        " efgh\n" <<
        "@@ -10,3 +15,3 @@\n" <<
        " abcd\n" <<
        "-delete1\n" <<
        "+insert1\n" <<
        " efgh\n";

        QStringList newLines;
        newLines << "newline1\n";
        QStringList oldLines;
        oldLines << "delete1\n";
        DifferenceHash expectedDifferences;
        expectedDifferences.insert(1, qMakePair(QStringList() << "delete1\n", QStringList() << "newline1\n"));

        QTest::newRow("Replace line in source") << patch << oldLines << newLines << 11 << false << 2 << expectedDifferences;
    }
}

void InteractiveDiffTest::testLineNumbers_data()
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

void InteractiveDiffTest::testLineNumbers()
{
    QFETCH(QStringList, patch);
    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);
    model->applyAllDifferences(true);

    QFETCH(QStringList, oldLines);
    QFETCH(QStringList, newLines);
    QFETCH(int, editLineNumber);
    model->linesChanged(oldLines, newLines, editLineNumber);
    QFETCH(int, expectedDifferenceCount);
    QCOMPARE(model->differenceCount(), expectedDifferenceCount);

    QFETCH(LineNumberHash, expectedLineNumbers);
    for (LineNumberHash::ConstIterator iter = expectedLineNumbers.constBegin(); iter != expectedLineNumbers.constEnd(); ++iter) {
        const Difference* diff = model->differenceAt(iter.key());
        QCOMPARE(diff->sourceLineNumber(), iter.value().first);
        QCOMPARE(diff->trackingDestinationLineNumber(), iter.value().second);
    }
}

// When the new diff and an existing unapplied one are on neighbour lines, do not merge the unapplied with the new.
void InteractiveDiffTest::testAppliedTouch()
{
    Difference* first = new Difference(2, 2);
    first->addSourceLine(QString("delete1"));
    first->addDestinationLine(QString("insert1"));
    first->apply(false);
    Difference* second = new Difference(4, 4);
    second->addSourceLine(QString("delete2"));
    second->addDestinationLine(QString("insert2"));
    second->apply(false);
    DiffModel model;
    model.addDiff(first);
    model.addDiff(second);
    model.linesChanged(QStringList() << "oldline\n", QStringList() << "newline\n", 3);
    QCOMPARE(model.differenceCount(), 3);
    QCOMPARE(model.differenceAt(0), first);
    QCOMPARE(model.differenceAt(2), second);
}

// When the new diff and an existing unapplied one intersect, the unapplied one should be removed
void InteractiveDiffTest::testAppliedIntersect()
{
    Difference* first = new Difference(2, 2);
    first->addSourceLine(QString("delete1"));
    first->addSourceLine(QString("delete2"));
    first->addDestinationLine(QString("insert1"));
    first->addDestinationLine(QString("insert2"));
    first->apply(false);
    Difference* second = new Difference(5, 5);
    second->addSourceLine(QString("delete3"));
    second->addSourceLine(QString("delete4"));
    second->addDestinationLine(QString("insert3"));
    second->addDestinationLine(QString("insert4"));
    second->apply(false);
    DiffModel model;
    model.addDiff(first);
    model.addDiff(second);
    QStringList removedLines;
    removedLines << "delete2\n" << "oldline1\n" << "delete3\n";
    QStringList insertedLines;
    insertedLines << "newline1\n";
    model.linesChanged(removedLines, insertedLines, 3);
    QCOMPARE(model.differenceCount(), 1);
    const Difference* newDiff = model.differenceAt(0);
    QCOMPARE(newDiff->applied(), true);
    QCOMPARE(newDiff->sourceLineNumber(), 3);
    QCOMPARE(newDiff->trackingDestinationLineNumber(), 3);
    CompareDifferenceStringList(newDiff->sourceLines(), removedLines);
    CompareDifferenceStringList(newDiff->destinationLines(), insertedLines);
}

void InteractiveDiffTest::testExistingAndApplied()
{
    Difference* first = new Difference(2, 2);
    first->addSourceLine(QString("delete1"));
    first->addDestinationLine(QString("insert1"));
    first->apply(true);
    Difference* second = new Difference(3, 3);
    second->addSourceLine(QString("delete2"));
    second->addDestinationLine(QString("insert2"));
    second->apply(false);
    DiffModel model;
    model.addDiff(first);
    model.addDiff(second);
    QStringList removedLines;
    removedLines << "delete1\n";
    QStringList insertedLines;
    insertedLines << "newline1\n";
    model.linesChanged(removedLines, insertedLines, 2);
    QCOMPARE(model.differenceCount(), 2);
    QVERIFY(model.differenceAt(0)->applied());
    QVERIFY(!model.differenceAt(1)->applied());
}

void InteractiveDiffTest::testOneLineDeletionUnapplied()
{
    Difference* unappliedDeletion = new Difference(1, 1);
    unappliedDeletion->addSourceLine("delete1\n");
    unappliedDeletion->apply(false);
    DiffModel model;
    model.addDiff(unappliedDeletion);
    QStringList removedLines;
    removedLines << "delete1\n";
    QStringList insertedLines;
    insertedLines << "newline1\n";
    model.linesChanged(removedLines, insertedLines, 1);
    QCOMPARE(model.differenceCount(), 1);
    const Difference* actual = model.differenceAt(0);
    CompareDifferenceStringList(actual->sourceLines(), removedLines);
    CompareDifferenceStringList(actual->destinationLines(), insertedLines);
}

void InteractiveDiffTest::testApplyUnapply()
{
    QStringList patch;
    patch <<
    "--- file1\t2011-01-01 20:23:45 +0300\n" <<
    "+++ file2\t2011-01-01 20:24:02 +0300\n" <<
    "@@ -1,3 +1,4 @@\n" <<
    " line1\n" <<
    "-delete1\n" <<
    "+insert1\n" <<
    "+insert2\n" <<
    " line2\n" <<
    "@@ -11,4 +12,5 @@\n" <<
    " line3\n" <<
    "-delete2\n" <<
    "-delete3\n" <<
    "+insert3\n" <<
    "+insert4\n" <<
    "+insert5\n" <<
    " line4\n" <<
    "@@ -21,4 +23,2 @@\n" <<
    " line5\n" <<
    "-delete4\n" <<
    "-delete5\n" <<
    " line6\n" <<
    "@@ -31,3 +31,3 @@\n" <<
    " line7\n" <<
    "-delete6\n" <<
    "+insert6\n" <<
    " line8\n";
    Parser parser(0);
    DiffModelList* models = parser.parse(patch);
    QCOMPARE(models->size(), 1);
    DiffModel* model = models->at(0);
    QCOMPARE(model->differenceCount(), 4);
    model->applyAllDifferences(true);

    foreach( Difference* diff, *model->differences() )
    {
        QVERIFY(diff->applied());
    }
    model->applyAllDifferences(false);
    QVERIFY(!model->differenceAt(0)->applied());
    QCOMPARE(model->differenceAt(0)->sourceLineNumber(), 2);
    QCOMPARE(model->differenceAt(0)->trackingDestinationLineNumber(), 2);
    QVERIFY(!model->differenceAt(1)->applied());
    QCOMPARE(model->differenceAt(1)->sourceLineNumber(), 12);
    QCOMPARE(model->differenceAt(1)->trackingDestinationLineNumber(), 12);
    QVERIFY(!model->differenceAt(2)->applied());
    QCOMPARE(model->differenceAt(2)->sourceLineNumber(), 22);
    QCOMPARE(model->differenceAt(2)->trackingDestinationLineNumber(), 22);
    QVERIFY(!model->differenceAt(3)->applied());
    QCOMPARE(model->differenceAt(3)->sourceLineNumber(), 32);
    QCOMPARE(model->differenceAt(3)->trackingDestinationLineNumber(), 32);

    model->differenceAt(1)->apply(true);
    QVERIFY(model->differenceAt(1)->applied());
    QCOMPARE(model->differenceAt(1)->sourceLineNumber(), 12);
    QCOMPARE(model->differenceAt(1)->trackingDestinationLineNumber(), 12);
    QVERIFY(!model->differenceAt(2)->applied());
    QCOMPARE(model->differenceAt(2)->sourceLineNumber(), 22);
    QCOMPARE(model->differenceAt(2)->trackingDestinationLineNumber(), 23);
    QVERIFY(!model->differenceAt(3)->applied());
    QCOMPARE(model->differenceAt(3)->sourceLineNumber(), 32);
    QCOMPARE(model->differenceAt(3)->trackingDestinationLineNumber(), 33);

    model->differenceAt(1)->apply(true);
    QVERIFY(model->differenceAt(1)->applied());
    QCOMPARE(model->differenceAt(1)->sourceLineNumber(), 12);
    QCOMPARE(model->differenceAt(1)->trackingDestinationLineNumber(), 12);
    QVERIFY(!model->differenceAt(2)->applied());
    QCOMPARE(model->differenceAt(2)->sourceLineNumber(), 22);
    QCOMPARE(model->differenceAt(2)->trackingDestinationLineNumber(), 23);
    QVERIFY(!model->differenceAt(3)->applied());
    QCOMPARE(model->differenceAt(3)->sourceLineNumber(), 32);
    QCOMPARE(model->differenceAt(3)->trackingDestinationLineNumber(), 33);

    model->differenceAt(2)->apply(true);
    QVERIFY(model->differenceAt(2)->applied());
    QCOMPARE(model->differenceAt(2)->sourceLineNumber(), 22);
    QCOMPARE(model->differenceAt(2)->trackingDestinationLineNumber(), 23);
    QVERIFY(!model->differenceAt(3)->applied());
    QCOMPARE(model->differenceAt(3)->sourceLineNumber(), 32);
    QCOMPARE(model->differenceAt(3)->trackingDestinationLineNumber(), 31);

    model->applyAllDifferences(true);
    QVERIFY(model->differenceAt(0)->applied());
    QCOMPARE(model->differenceAt(0)->sourceLineNumber(), 2);
    QCOMPARE(model->differenceAt(0)->trackingDestinationLineNumber(), 2);
    QVERIFY(model->differenceAt(1)->applied());
    QCOMPARE(model->differenceAt(1)->sourceLineNumber(), 12);
    QCOMPARE(model->differenceAt(1)->trackingDestinationLineNumber(), 13);
    QVERIFY(model->differenceAt(2)->applied());
    QCOMPARE(model->differenceAt(2)->sourceLineNumber(), 22);
    QCOMPARE(model->differenceAt(2)->trackingDestinationLineNumber(), 24);
    QVERIFY(model->differenceAt(3)->applied());
    QCOMPARE(model->differenceAt(3)->sourceLineNumber(), 32);
    QCOMPARE(model->differenceAt(3)->trackingDestinationLineNumber(), 32);

    model->applyAllDifferences(true);
    QVERIFY(model->differenceAt(0)->applied());
    QCOMPARE(model->differenceAt(0)->sourceLineNumber(), 2);
    QCOMPARE(model->differenceAt(0)->trackingDestinationLineNumber(), 2);
    QVERIFY(model->differenceAt(1)->applied());
    QCOMPARE(model->differenceAt(1)->sourceLineNumber(), 12);
    QCOMPARE(model->differenceAt(1)->trackingDestinationLineNumber(), 13);
    QVERIFY(model->differenceAt(2)->applied());
    QCOMPARE(model->differenceAt(2)->sourceLineNumber(), 22);
    QCOMPARE(model->differenceAt(2)->trackingDestinationLineNumber(), 24);
    QVERIFY(model->differenceAt(3)->applied());
    QCOMPARE(model->differenceAt(3)->sourceLineNumber(), 32);
    QCOMPARE(model->differenceAt(3)->trackingDestinationLineNumber(), 32);
}

QTEST_MAIN(InteractiveDiffTest);
