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

#include <vcs/vcslocation.h>

using namespace KDevelop;

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
