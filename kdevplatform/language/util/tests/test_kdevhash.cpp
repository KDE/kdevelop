/*
    This file is part of KDevelop

    Copyright 2015 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include <QObject>
#include <QTest>

#include <vector>
#include <algorithm>

#include "../kdevhash.h"

namespace {
template <typename T>
std::vector<T> generateData(std::size_t size)
{
    auto ret = std::vector<T>(size);
    if constexpr (std::is_same_v<T, bool>) {
        std::generate(ret.begin(), ret.end(), [i = 0]() mutable { return (i++ % 2) == 0; });
    } else {
        std::iota(ret.begin(), ret.end(), T(0));
    }
    return ret;
}

template <typename T>
void runBench()
{
    const auto data = generateData<T>(10000);
    KDevHash hash;
    QBENCHMARK {
        for (T i : data) {
            hash << i;
        }
    }
    QVERIFY(static_cast<uint>(hash));
}
}

class TestKDevHash
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void benchHash_int()
    {
        runBench<int>();
    }

    void benchHash_uint()
    {
        runBench<uint>();
    }

    void benchHash_quint64()
    {
        runBench<quint64>();
    }

    void benchHash_bool()
    {
        runBench<bool>();
    }
};

QTEST_MAIN(TestKDevHash)

#include "test_kdevhash.moc"
