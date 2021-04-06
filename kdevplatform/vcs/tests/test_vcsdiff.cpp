/* This file is part of KDevelop
 *
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "test_vcsdiff.h"

#include <QTest>
#include <QStandardPaths>

#include <vcs/vcslocation.h>

using namespace KDevelop;

void TestVcsDiff::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QFINDTESTDATA("sample.diff");
    QFile in(path);
    QCOMPARE(in.open(QIODevice::ReadOnly), true);
    sampleDiff = QString::fromUtf8(in.readAll());
    QCOMPARE(sampleDiff.length(), 6235);
}

void TestVcsDiff::setDiff(VcsDiff& diff,
                          const QString& diffString,
                          const QUrl& baseDiff,
                          uint depth)
{
    diff.setDiff(diffString);
    diff.setBaseDiff(baseDiff);
    diff.setDepth(depth);
}

void TestVcsDiff::compareDiff(const VcsDiff& diff,
                              const QString& diffString,
                              const QUrl& baseDiff,
                              uint depth)
{
    QCOMPARE(diff.diff(), diffString);
    QCOMPARE(diff.baseDiff(), baseDiff);
    QCOMPARE(diff.depth(), depth);
}

void TestVcsDiff::testFileNames()
{
    VcsDiff diff;
    diff.setDiff(sampleDiff);
    const QVector<VcsDiff::FilePair> expected = {
        {"kdevplatform/vcs/vcsdiff.cpp", "kdevplatform/vcs/vcsdiff.cpp"},
        {"kdevplatform/vcs/vcsdiff.h", "kdevplatform/vcs/vcsdiff.h"}
    };
    QCOMPARE(diff.fileNames(), expected);
}

void TestVcsDiff::testSubDiff()
{
    /**************************
     * Test a real world diff *
     **************************/
    VcsDiff diff;
    diff.setDiff(sampleDiff);
    QString expected(R"diff(--- a/kdevplatform/vcs/vcsdiff.cpp
+++ b/kdevplatform/vcs/vcsdiff.cpp
@@ -39,7 +39,7 @@ public:
        , oldCount
        , newStart
        , newCount
-       , firstLineIdx
+       , firstLineIdx  /**< The 0-based line number (in the whole diff) of the hunk header line (the one starting with `@@`) */
        ;
     QString srcFile    /**< The source filename */
           , tgtFile    /**< The target filename */
)diff");
    const auto subHunkDiffs = {
        diff.subDiffHunk(15),
    };
    for(auto df: subHunkDiffs) {
        QVERIFY(df.fileNames().count()>0);
        QCOMPARE(df.fileNames().front().source, "kdevplatform/vcs/vcsdiff.cpp");
        QCOMPARE(df.diff(), expected);
    }

    VcsDiff subHunkDiff = diff.subDiff(15, 50);
    QVERIFY(subHunkDiff.fileNames().count()>0);
    QCOMPARE(subHunkDiff.fileNames().front().source, "kdevplatform/vcs/vcsdiff.cpp");

    /*********************************
     * Test start offset computation *
     *********************************/
    VcsDiff skipDiff;
    skipDiff.setDiff(R"diff(--- a/skip
+++ b/skip
@@ -1,2 +3,2 @@ heading
+ increase offset

+ dont increase offset
- deletion
- second deletion
)diff");
    expected = QStringLiteral(R"diff(--- a/skip
+++ b/skip
@@ -2,3 +4,2 @@ heading

- deletion
  second deletion
)diff");
    auto subDiff = skipDiff.subDiff(6,6);
    QCOMPARE(subDiff.diff(), expected);

}

void TestVcsDiff::testConflicts()
{
    VcsDiff conflictDiff;
    conflictDiff.setDiff(R"diff(--- a/skip
+++ b/skip
@@ -1,2 +3,2 @@ conflict
+ increase offset            3     2
                             4  0  3
+ dont increase offset       5     4
- deletion                   6  1
<<<<<<< HEAD                 7
blablabla                    8  2
=======                      9
yadayadayada                10     5
>>>>>>> commit              11
- second deletion           12  3
)diff");
    QCOMPARE(conflictDiff.diffLineToSourceLine(8), 2);
    QCOMPARE(conflictDiff.diffLineToTargetLine(10), 5);
    QCOMPARE(conflictDiff.diffLineToSourceLine(7), -1);
    QCOMPARE(conflictDiff.diffLineToSourceLine(9), -1);
    QCOMPARE(conflictDiff.diffLineToSourceLine(11), -1);
}


void TestVcsDiff::testLineMapping()
{
    VcsDiff diff;
    diff.setDiff(sampleDiff);

    // We have no way to map the headings
    QCOMPARE(diff.diffLineToSourceLine(169-1), -1);
    QCOMPARE(diff.diffLineToTargetLine(169-1), -1);

    QCOMPARE(diff.diffLineToSourceLine(15-1), 42-1);
    QCOMPARE(diff.diffLineToTargetLine(15-1), -1);

    QCOMPARE(diff.diffLineToTargetLine(147-1), 180-1);
    QCOMPARE(diff.diffLineToSourceLine(147-1), 150-1);
}


 void TestVcsDiff::testCopyConstructor()
 {
    // test plain copy
    const QString diffString("diff");
    const QUrl baseDiff("git://1");
    const uint depth = 1;
    const VcsLocation location("server");

    {
        VcsDiff diffA;
        setDiff(diffA, 
                diffString, baseDiff, depth);

        VcsDiff diffB(diffA);
        compareDiff(diffA,
                    diffString, baseDiff, depth);
        compareDiff(diffB,
                    diffString, baseDiff, depth);
    }

    const QString diffStringNew("diffNew");

    // test detach after changing A
    {
        VcsDiff diffA;
        setDiff(diffA, 
                diffString, baseDiff, depth);

        VcsDiff diffB(diffA);
        // change a property of A
        diffA.setDiff(diffStringNew);

        compareDiff(diffA,
                    diffStringNew, baseDiff, depth);
        compareDiff(diffB,
                    diffString, baseDiff, depth);
    }
}

void TestVcsDiff::testAssignOperator()
{
    // test plain copy
    const QString diffString("diff");
    const QUrl baseDiff("git://1");
    const uint depth = 1;
    const VcsLocation location("server");

    {
        VcsDiff diffA;
        setDiff(diffA, 
                diffString, baseDiff, depth);

        VcsDiff diffB;
        diffB = diffA;
        compareDiff(diffA,
                    diffString, baseDiff, depth);
        compareDiff(diffB,
                    diffString, baseDiff, depth);
    }

    const QString diffStringNew("diffNew");

    // test detach after changing A
    {
        VcsDiff diffA;
        setDiff(diffA, 
                diffString, baseDiff, depth);

        VcsDiff diffB;
        diffB = diffA;
        // change a property of A
        diffA.setDiff(diffStringNew);

        compareDiff(diffA,
                    diffStringNew, baseDiff, depth);
        compareDiff(diffB,
                    diffString,    baseDiff, depth);
    }
}

QTEST_GUILESS_MAIN(TestVcsDiff)
