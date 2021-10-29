/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_vcsstatusinfo.h"

#include <QTest>
#include <QStandardPaths>

using namespace KDevelop;

void TestVcsStatusInfo::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestVcsStatusInfo::testCopyConstructor()
{
    // test plain copy
    const QUrl url(QStringLiteral("git://foo"));
    const VcsStatusInfo::State state = VcsStatusInfo::ItemUpToDate;

    {
        VcsStatusInfo statusInfoA;
        statusInfoA.setUrl(url);
        statusInfoA.setState(state);

        VcsStatusInfo statusInfoB(statusInfoA);

        QCOMPARE(statusInfoA.url(), url);
        QCOMPARE(statusInfoA.state(), state);
        QCOMPARE(statusInfoB.url(), url);
        QCOMPARE(statusInfoB.state(), state);
        QVERIFY(statusInfoA == statusInfoB);
        QVERIFY(statusInfoB == statusInfoA);
    }

    const QUrl urlNew(QStringLiteral("svn://bar"));

    // test detach after changing A
    {
        VcsStatusInfo statusInfoA;
        statusInfoA.setUrl(url);
        statusInfoA.setState(state);

        VcsStatusInfo statusInfoB(statusInfoA);
        // change a property of A
        statusInfoA.setUrl(urlNew);

        QCOMPARE(statusInfoA.url(), urlNew);
        QCOMPARE(statusInfoA.state(), state);
        QCOMPARE(statusInfoB.url(), url);
        QCOMPARE(statusInfoB.state(), state);
        QVERIFY(statusInfoA != statusInfoB);
        QVERIFY(statusInfoB != statusInfoA);
    }
}

void TestVcsStatusInfo::testAssignOperator()
{
    // test plain copy
    const QUrl url(QStringLiteral("git://foo"));
    const VcsStatusInfo::State state = VcsStatusInfo::ItemUpToDate;

    {
        VcsStatusInfo statusInfoA;
        statusInfoA.setUrl(url);
        statusInfoA.setState(state);

        VcsStatusInfo statusInfoB;
        statusInfoB = statusInfoA;

        QCOMPARE(statusInfoA.url(), url);
        QCOMPARE(statusInfoA.state(), state);
        QCOMPARE(statusInfoB.url(), url);
        QCOMPARE(statusInfoB.state(), state);
        QVERIFY(statusInfoA == statusInfoB);
        QVERIFY(statusInfoB == statusInfoA);
    }

    const QUrl urlNew(QStringLiteral("svn://bar"));

    // test detach after changing A
    {
        VcsStatusInfo statusInfoA;
        statusInfoA.setUrl(url);
        statusInfoA.setState(state);

        VcsStatusInfo statusInfoB;
        statusInfoB = statusInfoA;
        // change a property of A
        statusInfoA.setUrl(urlNew);

        QCOMPARE(statusInfoA.url(), urlNew);
        QCOMPARE(statusInfoA.state(), state);
        QCOMPARE(statusInfoB.url(), url);
        QCOMPARE(statusInfoB.state(), state);
        QVERIFY(statusInfoA != statusInfoB);
        QVERIFY(statusInfoB != statusInfoA);
    }
}

QTEST_GUILESS_MAIN(TestVcsStatusInfo)
