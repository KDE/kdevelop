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
                          VcsDiff::Type type,
                          VcsDiff::Content contentType,
                          const QString& diffString,
                          const QUrl& baseDiff,
                          uint depth,
                          const QHash<VcsLocation,QByteArray>& leftBinaries,
                          const QHash<VcsLocation,QByteArray>& rightBinaries,
                          const QHash<VcsLocation,QString>& leftTexts,
                          const QHash<VcsLocation,QString>& rightTexts)
{
    diff.setType(type);
    diff.setContentType(contentType);
    diff.setDiff(diffString);
    diff.setBaseDiff(baseDiff);
    diff.setDepth(depth);
    for (auto it = leftBinaries.begin(); it != leftBinaries.end(); ++it)
        diff.addLeftBinary(it.key(), it.value());
    for (auto it = rightBinaries.begin(); it != rightBinaries.end(); ++it)
        diff.addRightBinary(it.key(), it.value());
    for (auto it = leftTexts.begin(); it != leftTexts.end(); ++it)
        diff.addLeftText(it.key(), it.value());
    for (auto it = rightTexts.begin(); it != rightTexts.end(); ++it)
        diff.addRightText(it.key(), it.value());
}

void TestVcsDiff::compareDiff(const VcsDiff& diff,
                              VcsDiff::Type type,
                              VcsDiff::Content contentType,
                              const QString& diffString,
                              const QUrl& baseDiff,
                              uint depth,
                              const QHash<VcsLocation,QByteArray>& leftBinaries,
                              const QHash<VcsLocation,QByteArray>& rightBinaries,
                              const QHash<VcsLocation,QString>& leftTexts,
                              const QHash<VcsLocation,QString>& rightTexts)
{
    QCOMPARE(diff.type(), type);
    QCOMPARE(diff.contentType(), contentType);
    QCOMPARE(diff.diff(), diffString);
    QCOMPARE(diff.baseDiff(), baseDiff);
    QCOMPARE(diff.depth(), depth);
    QCOMPARE(diff.leftBinaries(), leftBinaries);
    QCOMPARE(diff.rightBinaries(), rightBinaries);
    QCOMPARE(diff.leftTexts(), leftTexts);
    QCOMPARE(diff.rightTexts(), rightTexts);
}

void TestVcsDiff::testCopyConstructor()
{
    // test plain copy
    const VcsDiff::Type type = VcsDiff::DiffRaw;
    const VcsDiff::Content contentType = VcsDiff::Binary;
    const QString diffString("diff");
    const QUrl baseDiff("git://1");
    const uint depth = 1;
    const VcsLocation location("server");
    const QHash<VcsLocation,QByteArray> leftBinaries({{location, "leftbinary"}});
    const QHash<VcsLocation,QByteArray> rightBinaries({{location, "rightbinary"}});
    const QHash<VcsLocation,QString> leftTexts({{location, "lefttext"}});
    const QHash<VcsLocation,QString> rightTexts({{location, "righttext"}});

    {
        VcsDiff diffA;
        setDiff(diffA, 
                type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);

        VcsDiff diffB(diffA);
        compareDiff(diffA,
                    type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
        compareDiff(diffB,
                    type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
    }

    const VcsDiff::Type typeNew = VcsDiff::DiffUnified;

    // test detach after changing A
    {
        VcsDiff diffA;
        setDiff(diffA, 
                type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);

        VcsDiff diffB(diffA);
        // change a property of A
        diffA.setType(typeNew);

        compareDiff(diffA,
                    typeNew, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
        compareDiff(diffB,
                    type,    contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
    }
}

void TestVcsDiff::testAssignOperator()
{
    // test plain copy
    const VcsDiff::Type type = VcsDiff::DiffRaw;
    const VcsDiff::Content contentType = VcsDiff::Binary;
    const QString diffString("diff");
    const QUrl baseDiff("git://1");
    const uint depth = 1;
    const VcsLocation location("server");
    const QHash<VcsLocation,QByteArray> leftBinaries({{location, "leftbinary"}});
    const QHash<VcsLocation,QByteArray> rightBinaries({{location, "rightbinary"}});
    const QHash<VcsLocation,QString> leftTexts({{location, "lefttext"}});
    const QHash<VcsLocation,QString> rightTexts({{location, "righttext"}});

    {
        VcsDiff diffA;
        setDiff(diffA, 
                type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);

        VcsDiff diffB;
        diffB = diffA;
        compareDiff(diffA,
                    type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
        compareDiff(diffB,
                    type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
    }

    const VcsDiff::Type typeNew = VcsDiff::DiffUnified;

    // test detach after changing A
    {
        VcsDiff diffA;
        setDiff(diffA, 
                type, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);

        VcsDiff diffB;
        diffB = diffA;
        // change a property of A
        diffA.setType(typeNew);

        compareDiff(diffA,
                    typeNew, contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
        compareDiff(diffB,
                    type,    contentType, diffString, baseDiff, depth, leftBinaries, rightBinaries, leftTexts, rightTexts);
    }
}

QTEST_GUILESS_MAIN(TestVcsDiff);
