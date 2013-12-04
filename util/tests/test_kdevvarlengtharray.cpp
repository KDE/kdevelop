/* This file is part of KDevelop
    Copyright 2010 Milian Wolff <mail@milianw.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

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
#include <QtTest/QtTest>

#include <QDebug>

#include "../kdevvarlengtharray.h"

struct TestValue {
    TestValue() : m_index(0)
    {}
    TestValue(const TestValue& other)
    {
        if (other.m_index) {
            int mustDo = 1;
            ++mustDo;
        }
        m_index = other.m_index;
    }
    uint m_index;
};

class TestKDevVarLengthArray : public QObject {
    Q_OBJECT

private slots:
    /**
     * Make sure that valgrind does not report any warnings here
     * about uninitialized member variables.
     */
    void appendReallocIntegrity() {
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
        array.insert(0, 0);
        QCOMPARE(array.back(), 2);
        array.pop_back();
        QCOMPARE(array.back(), 1);
        array.append(1);
        QVERIFY(array.removeOne(1));
        QCOMPARE(array.toList(), QList<int>() << 0 << 1);
        QCOMPARE(array.toVector(), QVector<int>() << 0 << 1);
        array.insert(0, 42);
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
