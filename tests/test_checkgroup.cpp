/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "test_checkgroup.h"

// SUT
#include "config/checkgroup.h"
// Qt
#include <QTest>

struct CheckGroupData
{
    QString prefix;
    QStringList checkNames;

    QVector<CheckGroupData> subGroups;
};
Q_DECLARE_METATYPE(CheckGroupData)

void TestCheckGroup::testFromPlainList_data()
{
    QTest::addColumn<QStringList>("plainList");
    QTest::addColumn<CheckGroupData>("group");

    QTest::newRow("empty")
        << QStringList()
        << CheckGroupData{};
    QTest::newRow("fooandbarlevel1")
        << QStringList{"foo-one", "foo-two", "foo-two-and-half", "bar-one", "bar-two-something", "more"}
        << CheckGroupData{"", {"more"}, {
              {"foo-", {"foo-one", "foo-two", "foo-two-and-half"}, {}},
              {"bar-", {"bar-one", "bar-two-something"}, {}}
           }};
    QTest::newRow("fooandbarlevel2")
        << QStringList{"foo-one-two", "foo-one-three", "foo-a.two", "foo-a.three", "bar-no-way", "bar-no-sense"}
        << CheckGroupData{"", {}, {
              {"foo-", {}, {
                  {"foo-one-", {"foo-one-two", "foo-one-three"}, {}},
                  {"foo-a.", {"foo-a.two", "foo-a.three"}, {}},
              }},
              {"bar-", {}, {
                  {"bar-no-", {"bar-no-way", "bar-no-sense"}, {}}
              }}
           }};
}

void TestCheckGroup::doTestResult(const ClangTidy::CheckGroup* actualValue, const CheckGroupData& expectedValue)
{
    QCOMPARE(actualValue->prefix(), expectedValue.prefix);

    if (actualValue->checkNames() != expectedValue.checkNames) {
        qDebug() << "For checkgroup" << actualValue->prefix();
        qDebug() << "Actual checknames:  " << actualValue->checkNames();
        qDebug() << "Expected checknames:" << expectedValue.checkNames;
    }
    QCOMPARE(actualValue->checkNames(), expectedValue.checkNames);

    QCOMPARE(actualValue->subGroups().size(), expectedValue.subGroups.size());

    for (int i = 0; i < expectedValue.subGroups.size(); ++i) {
        const auto* actualSubGroup = actualValue->subGroups()[i];
        QCOMPARE(actualSubGroup->superGroup(), actualValue);
        doTestResult(actualSubGroup, expectedValue.subGroups[i]);
    }
}

void TestCheckGroup::testFromPlainList()
{
    QFETCH(QStringList, plainList);
    QFETCH(CheckGroupData, group);

    const ClangTidy::CheckGroup* checkGroup = ClangTidy::CheckGroup::fromPlainList(plainList);

    doTestResult(checkGroup, group);

    delete checkGroup;
}


struct CheckStateGroupData
{
    int enabledChecksCount;
    bool hasSubGroupWithExplicitEnabledState;

    QVector<CheckStateGroupData> subGroups;
};
Q_DECLARE_METATYPE(CheckStateGroupData)

void TestCheckGroup::testSetEnabledChecks_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("result");
    QTest::addColumn<CheckStateGroupData>("group");

    QTest::newRow("empty")
        << QStringList()
        << QStringList{"-*"}
        << CheckStateGroupData{0, false, {
            {0, false, { // foo-
                {0, false, {}}, // foo-one-
                {0, false, {}}, // foo-a.
            }},
            {0, false, { // bar-
                {0, false, {}}, // bar-mo-
            }},
        }};

    QTest::newRow("all")
        << QStringList{"*"}
        << QStringList{"*"}
        << CheckStateGroupData{7, false, {
            {5, false, { // foo-
                {2, false, {}}, // foo-one-
                {2, false, {}}, // foo-a.
            }},
            {2, false, { // bar-
                {2, false, {}}, // bar-mo-
            }},
        }};

    QTest::newRow("all-not-foo")
        << QStringList{"*", "-foo-*"}
        << QStringList{"*", "-foo-*"}
        << CheckStateGroupData{2, true, {
            {0, false, { // foo-
                {0, false, {}}, // foo-one-
                {0, false, {}}, // foo-a.
            }},
            {2, false, { // bar-
                {2, false, {}}, // bar-mo-
            }},
        }};

    QTest::newRow("all-not-foo-but-foo-a")
        << QStringList{"*", "-foo-*", "foo-a.*"}
        << QStringList{"*", "-foo-*", "foo-a.*"}
        << CheckStateGroupData{4, true, {
            {2, true, { // foo-
                {0, false, {}}, // foo-one-
                {2, false, {}}, // foo-a.
            }},
            {2, false, { // bar-
                {2, false, {}}, // bar-mo-
            }},
        }};

    QTest::newRow("all-not-foo-but-foo-a.two")
        << QStringList{"*", "-foo-*", "foo-a.two"}
        << QStringList{"*", "-foo-*", "foo-a.two"}
        << CheckStateGroupData{3, true, {
            {1, true, { // foo-
                {0, false, {}}, // foo-one-
                {1, true, {}}, // foo-a.
            }},
            {2, false, { // bar-
                {2, false, {}}, // bar-mo-
            }},
        }};

    QTest::newRow("nothing-but-foo-one-three")
        << QStringList{"-*", "foo-one-three"}
        << QStringList{"-*", "foo-one-three"}
        << CheckStateGroupData{1, true, {
            {1, true, { // foo-
                {1, true, {}}, // foo-one-
                {0, false, {}}, // foo-a.
            }},
            {0, false, { // bar-
                {0, false, {}}, // bar-mo-
            }},
        }};
}

void TestCheckGroup::doTestResult(const ClangTidy::CheckGroup* actualValue, const CheckStateGroupData& expectedValue)
{
    if ((actualValue->enabledChecksCount() != expectedValue.enabledChecksCount) ||
        (actualValue->hasSubGroupWithExplicitEnabledState() != expectedValue.hasSubGroupWithExplicitEnabledState)) {
        qDebug() << "For checkgroup" << actualValue->prefix();
        qDebug() << "Actual enabledChecksCount:" << actualValue->enabledChecksCount()
                 << "hasSubGroupWithExplicitEnabledState: " << actualValue->hasSubGroupWithExplicitEnabledState();
        qDebug() << "Expected enabledChecksCount:" << expectedValue.enabledChecksCount
                 << "hasSubGroupWithExplicitEnabledState: " << expectedValue.hasSubGroupWithExplicitEnabledState;
    }

    QCOMPARE(actualValue->enabledChecksCount(), expectedValue.enabledChecksCount);
    QCOMPARE(actualValue->hasSubGroupWithExplicitEnabledState(), expectedValue.hasSubGroupWithExplicitEnabledState);

    QCOMPARE(actualValue->subGroups().size(), expectedValue.subGroups.size());

    for (int i = 0; i < expectedValue.subGroups.size(); ++i) {
        const auto* actualSubGroup = actualValue->subGroups()[i];
        doTestResult(actualSubGroup, expectedValue.subGroups[i]);
    }
}

void TestCheckGroup::testSetEnabledChecks()
{
    QFETCH(QStringList, input);
    QFETCH(QStringList, result);
    QFETCH(CheckStateGroupData, group);

    ClangTidy::CheckGroup* checkGroup = ClangTidy::CheckGroup::fromPlainList({
        "foo-one-two",
        "foo-one-three",
        "foo-a.two",
        "foo-a.three",
        "foo-b",
        "bar-no-way",
        "bar-no-sense"});

    checkGroup->setEnabledChecks(input);

    QCOMPARE(checkGroup->enabledChecksRules(), result);

    doTestResult(checkGroup, group);

    delete checkGroup;
}


QTEST_GUILESS_MAIN(TestCheckGroup);
