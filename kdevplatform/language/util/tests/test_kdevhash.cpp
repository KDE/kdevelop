/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QObject>
#include <QTest>
#include <QStandardPaths>

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
    QVERIFY(static_cast<size_t>(hash));
}
}

class TestKDevHash
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { QStandardPaths::setTestModeEnabled(true); }

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
