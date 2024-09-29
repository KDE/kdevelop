/*
 *  SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "test_algorithm.h"

#include "../algorithm.h"

#include <QSet>
#include <QStandardPaths>
#include <QString>
#include <QTest>

#include <utility>
#include <vector>

QTEST_MAIN(TestAlgorithm)

TestAlgorithm::TestAlgorithm(QObject* parent)
    : QObject(parent)
{
}

void TestAlgorithm::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestAlgorithm::testUnite2String()
{
    const QSet<QString> a{"a", "bc", "12", "an"};
    const QSet<QString> b{"-1", "24", "bc", "foo", "bar", "0"};
    const auto sets = {a, b};
    const auto setUnion = Algorithm::unite(sets.begin(), sets.end());
    QCOMPARE(setUnion, a + b);
}

void TestAlgorithm::testUnite5Int()
{
    std::vector<QSet<int>> sets{{9, 18, 4, -5}, {1, 2, 19}, {-2, 0}, {5, 2, 8, -4, 18, 8}, {5, 2, 12}};

    QSet<int> expected;
    for (const auto& set : sets) {
        expected += set;
    }

    const auto setUnion = Algorithm::unite(std::move(sets));
    QCOMPARE(setUnion, expected);
}

#include "moc_test_algorithm.cpp"
