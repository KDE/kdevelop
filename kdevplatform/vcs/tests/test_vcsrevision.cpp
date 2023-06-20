/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_vcsrevision.h"

#include <QTest>
#include <QStandardPaths>

#include <vcs/vcsrevision.h>

using namespace KDevelop;

void TestVcsRevision::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

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

#include "moc_test_vcsrevision.cpp"
