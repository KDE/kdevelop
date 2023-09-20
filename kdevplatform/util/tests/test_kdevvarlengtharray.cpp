/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QObject>
#include <QTest>
#include <QStandardPaths>
#include <QDebug>

#include "../kdevvarlengtharray.h"

struct TestValue
{
    TestValue()
    {}
    TestValue(const TestValue& other)
    {
        if (other.m_index) {
            int mustDo = 1;
            ++mustDo;
        }
        m_index = other.m_index;
    }
    uint m_index = 0;
};

class TestKDevVarLengthArray : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { QStandardPaths::setTestModeEnabled(true); }

    /**
     * Make sure that valgrind does not report any warnings here
     * about uninitialized member variables.
     */
    void appendReallocIntegrity()
    {
        KDevVarLengthArray<TestValue, 2> array;
        QCOMPARE(array.size(), 0);
        QCOMPARE(array.capacity(), 2);

        qDebug() << "append item 1";
        array << TestValue();
        qDebug() << "appended index is:" << array[0].m_index;
        QCOMPARE(array.size(), 1);
        QCOMPARE(array.capacity(), 2);
        qDebug() << "append item 2";
        array << TestValue(); // should trigger the realloc
        qDebug() << "appended index is:" << array[1].m_index;
        QCOMPARE(array.size(), 2);
        QCOMPARE(array.capacity(), 2);
        qDebug() << "append item 3";
        array << TestValue();
        qDebug() << "appended index is:" << array[2].m_index;
        QCOMPARE(array.size(), 3);
        QCOMPARE(array.capacity(), 4);

        array.clear();
    }

    void mixed()
    {
        KDevVarLengthArray<int, 10> array;
        array.append(1);
        array << 2;
        array.prepend(0);
        QCOMPARE(array.back(), 2);
        array.pop_back();
        QCOMPARE(array.back(), 1);
        array.append(1);
        QVERIFY(array.removeOne(1));
        QCOMPARE(array.toList(), QList<int>() << 0 << 1);
        QCOMPARE(array.toVector(), QVector<int>() << 0 << 1);
        array.prepend(42);
        QCOMPARE(array.toVector(), QVector<int>() << 42 << 0 << 1);
        array.remove(0);
        QCOMPARE(array.toVector(), QVector<int>() << 0 << 1);
        QVERIFY(array.contains(1));
        QVERIFY(!array.contains(42));
        QCOMPARE(array.back(), 1);
        QCOMPARE(array.indexOf(1), 1);
        QCOMPARE(array.indexOf(42), -1);
    }
};

QTEST_MAIN(TestKDevVarLengthArray)

#include "test_kdevvarlengtharray.moc"
