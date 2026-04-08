/*
 *  SPDX-FileCopyrightText: 2026 Jarmo Tiitto <jarmo.tiitto@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "test_hashvalue.h"

#include "../../tests/testhelpermacros.h"
#include "../hash.h"

#include <QStandardPaths>
#include <QTest>

#include <utility>
#include <vector>

QTEST_MAIN(TestHashValue)

TestHashValue::TestHashValue(QObject* parent)
    : QObject(parent)
{
}

void TestHashValue::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

struct test0
{
    int x = 1, y = 2;
};

struct test1
{
    test0 s;
    int z = 3, w = 4;
};

struct test2
{
    int x = 1;
    int y = 2;
    char pad[4] = {'3', '4', '5', '6'};
};

struct badtype0
{
    int x = 1;
    int y = 2;
    char pad = '3';
};

struct badtype1
{
    virtual ~badtype1()
    {
    }
};

void TestHashValue::testHashValues()
{
    // This test is primarily a compile test and a example.
    using namespace KDevelop;
    // Legal operations:
    auto h = HashValue("");
    auto h1 = HashValue("a");
    auto h2 = HashValue("ab");
    auto h3 = HashValue("abcd");
    test0 v0[2];
    test1 v1[2];
    // Runtime sized array
    h = HashValue(v0, 1);
    h = HashValue(v1, 2);
    // Fixed sized array ctor
    h = HashValue(v1);
    // Hash and combine.
    h << v1[0];
    h2 << int(1);
    // HashValue combining. This does only N-1 mixes preventing a lot of unnecessary mixing.
    HashValue h4{h, h1, h2, h3};
    // copy assignable
    h = h3;
    // move assignable (same as copy..)
    h = std::move(h2);
    std::array<test1*, 2> ptrs;
    // Illegal, compiler errors:
    //HashValue nodefault;
    // Cannot hash:
    //h3 << float(3.0); // it's a *float* convert to a integer somehow first.
    //badtype0 err0;    // contains padding bytes sizeof(badtype0) != <sum of member type sizes>
    //h = HashValue(&err0, 1);
    //badtype1 err1;    // contains non hashable cruft
    //h = HashValue(&err1, 1);
    // HashValue cannot be hashed again.
    //h = HashValue(&h, 1);
    // Pointer types cannot be hashed, dereference the pointer first
    //h = HashValue(ptrs.data(), ptrs.size());
    // Everything can be hashed by force... not recommended.
    h << HashValue((std::byte*)ptrs.data(), ptrs.size() * sizeof(test1*));
}

#include "moc_test_hashvalue.cpp"
