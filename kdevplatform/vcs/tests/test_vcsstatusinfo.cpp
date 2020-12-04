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
