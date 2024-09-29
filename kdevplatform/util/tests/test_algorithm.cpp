/*
 *  SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "test_algorithm.h"

#include "../../tests/testhelpermacros.h"
#include "../algorithm.h"

#include <QSet>
#include <QStandardPaths>
#include <QString>
#include <QTest>

#include <utility>
#include <vector>

QTEST_MAIN(TestAlgorithm)

namespace {
template<typename T>
void verifyInsert(QSet<T>& set, const T& value, bool existing)
{
    const auto result = Algorithm::insert(set, value);
    QCOMPARE(*result.iterator, value);
    QCOMPARE(result.inserted, !existing);
}

#define VERIFY_INSERT_NEW(set, value)                                                                                  \
    do {                                                                                                               \
        verifyInsert(set, value, false);                                                                               \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

#define VERIFY_INSERT_EXISTING(set, value)                                                                             \
    do {                                                                                                               \
        verifyInsert(set, value, true);                                                                                \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

} // unnamed namespace

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

void TestAlgorithm::testInsert()
{
    QSet<int> set{5};
    VERIFY_INSERT_NEW(set, 4);
    VERIFY_INSERT_EXISTING(set, 5);
    VERIFY_INSERT_NEW(set, 7);
    VERIFY_INSERT_EXISTING(set, 7);
    VERIFY_INSERT_EXISTING(set, 4);
    set.remove(5);
    VERIFY_INSERT_NEW(set, 5);
    set.remove(4);
    VERIFY_INSERT_NEW(set, 4);
    VERIFY_INSERT_EXISTING(set, 4);
    VERIFY_INSERT_NEW(set, 0);

    const QSet expected{5, 7, 4, 0};
    QCOMPARE(set, expected);
}

#include "moc_test_algorithm.cpp"
