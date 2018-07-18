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

#include "test_vcsrevision.h"

#include <QTest>

#include <vcs/vcsrevision.h>

using namespace KDevelop;

void TestVcsRevision::testCopyConstructor()
{
    // copy invalid
    {
        VcsRevision revisionA;
        VcsRevision revisionB(revisionA);
        QCOMPARE(revisionA.revisionType(), VcsRevision::Invalid);
        QCOMPARE(revisionB.revisionType(), VcsRevision::Invalid);
        QVERIFY(revisionB == revisionA);
        QVERIFY(revisionA == revisionB);
    }

    const VcsRevision::RevisionType revisionType = VcsRevision::GlobalNumber;
    const QString globalNumberValue("A");

    // test plain copy
    {
        VcsRevision revisionA;
        revisionA.setRevisionValue(globalNumberValue, revisionType);

        VcsRevision revisionB(revisionA);
        QCOMPARE(revisionA.revisionType(), revisionType);
        QCOMPARE(revisionA.revisionValue().toString(), globalNumberValue);
        QCOMPARE(revisionB.revisionType(), revisionType);
        QCOMPARE(revisionB.revisionValue().toString(), globalNumberValue);
        QVERIFY(revisionB == revisionA);
        QVERIFY(revisionA == revisionB);
    }

    const VcsRevision::RevisionType revisionTypeNew = VcsRevision::FileNumber;
    const qlonglong fileNumberValueNew = 2;

    // test detach after changing A
    {
        VcsRevision revisionA;
        revisionA.setRevisionValue(globalNumberValue, revisionType);

        VcsRevision revisionB(revisionA);
        revisionA.setRevisionValue(fileNumberValueNew, revisionTypeNew);

        QCOMPARE(revisionA.revisionType(), revisionTypeNew);
        QCOMPARE(revisionA.revisionValue().toLongLong(), fileNumberValueNew);
        QCOMPARE(revisionB.revisionType(), revisionType);
        QCOMPARE(revisionB.revisionValue().toString(), globalNumberValue);
        QVERIFY(!(revisionB == revisionA));
        QVERIFY(!(revisionA == revisionB));
    }
}

void TestVcsRevision::testAssignOperator()
{
    // assign invalid
    {
        const VcsRevision::RevisionType typeB = VcsRevision::FileNumber;
        const qlonglong fileNumberB = 2;

        VcsRevision revisionA;

        VcsRevision revisionB;
        revisionB.setRevisionValue(fileNumberB, typeB);

        revisionB = revisionA;

        QCOMPARE(revisionA.revisionType(), VcsRevision::Invalid);
        QCOMPARE(revisionB.revisionType(), VcsRevision::Invalid);
        QVERIFY(revisionB == revisionA);
        QVERIFY(revisionA == revisionB);
    }

    const VcsRevision::RevisionType revisionType = VcsRevision::GlobalNumber;
    const QString globalNumberValue("A");

    // test plain assign
    {
        VcsRevision revisionA;
        revisionA.setRevisionValue(globalNumberValue, revisionType);

        VcsRevision revisionB;
        revisionB = revisionA;

        QCOMPARE(revisionA.revisionType(), revisionType);
        QCOMPARE(revisionA.revisionValue().toString(), globalNumberValue);
        QCOMPARE(revisionB.revisionType(), revisionType);
        QCOMPARE(revisionB.revisionValue().toString(), globalNumberValue);
        QVERIFY(revisionB == revisionA);
        QVERIFY(revisionA == revisionB);
    }

    const VcsRevision::RevisionType revisionTypeNew = VcsRevision::FileNumber;
    const qlonglong fileNumberValueNew = 2;

    // test detach after changing A
    {
        VcsRevision revisionA;
        revisionA.setRevisionValue(globalNumberValue, revisionType);

        VcsRevision revisionB;
        revisionB = revisionA;
        revisionA.setRevisionValue(fileNumberValueNew, revisionTypeNew);

        QCOMPARE(revisionA.revisionType(), revisionTypeNew);
        QCOMPARE(revisionA.revisionValue().toLongLong(), fileNumberValueNew);
        QCOMPARE(revisionB.revisionType(), revisionType);
        QCOMPARE(revisionB.revisionValue().toString(), globalNumberValue);
        QVERIFY(!(revisionB == revisionA));
        QVERIFY(!(revisionA == revisionB));
    }
}

QTEST_GUILESS_MAIN(TestVcsRevision)
