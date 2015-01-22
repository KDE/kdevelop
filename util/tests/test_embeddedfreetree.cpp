/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

unsigned int extractor_div_with = 0;

#include <QObject>
#include <QtTest/QtTest>
#include <QVector>
#include <QDebug>

#include <stdlib.h>
#include <set>
#include <util/embeddedfreetree.h>
#include <util/convenientfreelist.h>
#include <language/util/setrepository.h>
#include <time.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

struct TestItem {
    explicit TestItem(uint _value = 0) : value(_value), leftChild(-1), rightChild(-1) {
    }
    uint value;
    int leftChild;
    int rightChild;
    bool operator==(const TestItem& rhs) const {
        return value == rhs.value;
    }

    bool operator<(const TestItem& item) const {
        return value < item.value;
    }
};

struct TestItemConversion {
    static uint toIndex(const TestItem& item) {
        return item.value;
    }
    static TestItem toItem(uint index) {
        return TestItem(index);
    }
};

struct Extractor{
    static TestItem extract(const TestItem& item) {
        return TestItem(item.value/extractor_div_with);
    }
};


clock_t std_insertion = 0, std_removal = 0, std_contains = 0, std_iteration = 0, emb_insertion = 0, emb_removal = 0, emb_contains = 0, emb_iteration = 0;

QString toString(std::set<uint> set) {
    QString ret;
    for(std::set<uint>::const_iterator it = set.begin(); it != set.end(); ++it)
        ret += QStringLiteral("%1 ").arg(*it);
    return ret;
}

bool operator==(const std::set<uint> a, const std::set<uint> b) {
    if(a.size() != b.size()) {
        qDebug() << "size mismatch" << toString(a) << ": " << toString(b);
        return false;
    }
    std::set<uint>::const_iterator aIt = a.begin();
    std::set<uint>::const_iterator bIt = b.begin();
    for(; aIt != a.end(); ++aIt, ++bIt)
        if(*aIt != *bIt) {
            qDebug() << "mismatch" << toString(a) << ": " << toString(b);
            return false;
        }
    return true;
}

    struct TestItemHandler {
        public:
        static int rightChild(const TestItem& m_data) {
            return m_data.rightChild;
        }
        static int leftChild(const TestItem& m_data) {
            return m_data.leftChild;
        }
        static void setLeftChild(TestItem& m_data, int child) {
            m_data.leftChild = child;
        }
        static void setRightChild(TestItem& m_data, int child) {
            m_data.rightChild = child;
        }
        //Copies this item into the given one
        static void copyTo(const TestItem& m_data, TestItem& data) {
            data = m_data;
        }
        static void createFreeItem(TestItem& data) {
            data = TestItem();
        }
        static inline bool isFree(const TestItem& m_data) {
            return !m_data.value;
        }

        static const TestItem& data(const TestItem& m_data) {
            return m_data;
        }

        inline static bool equals(const TestItem& m_data, const TestItem& rhs) {
            return m_data.value == rhs.value;
        }
    };

    class TestItemBasedSet {
        public:
            TestItemBasedSet() : m_centralFree(-1) {
            }

            void insert(uint i) {
                TestItem item(i);
                KDevelop::EmbeddedTreeAddItem<TestItem, TestItemHandler> add(data.data(), data.size(), m_centralFree, item);

                if((int)add.newItemCount() != (int)data.size()) {
                    QVector<TestItem> newData;
                    newData.resize(add.newItemCount());
                    add.transferData(newData.data(), newData.size());
                    data = newData;
                }
            }

            bool contains(uint item) {
                KDevelop::EmbeddedTreeAlgorithms<TestItem, TestItemHandler> alg(data.data(), data.size(), m_centralFree);
                return alg.indexOf(TestItem(item)) != -1;
            }

            void remove(uint i) {
                TestItem item(i);
                KDevelop::EmbeddedTreeRemoveItem<TestItem, TestItemHandler> remove(data.data(), data.size(), m_centralFree, item);

                if((int)remove.newItemCount() != (int)data.size()) {
                    QVector<TestItem> newData;
                    newData.resize(remove.newItemCount());
                    remove.transferData(newData.data(), newData.size());
                    data = newData;
                }
            }

            std::set<uint> toSet() const {
                std::set<uint> ret;

                for(int a = 0; a < data.size(); ++a)
                    if(data[a].value)
                        ret.insert(data[a].value);

                return ret;
            }

            void verify() {
                //1. verify order
                uint last = 0;
                uint freeCount = 0;
                for(int a = 0; a < data.size(); ++a) {
                    if(data[a].value) {
                        QVERIFY(last < data[a].value);
                        last = data[a].value;
                    }else{
                        ++freeCount;
                    }
                }
                KDevelop::EmbeddedTreeAlgorithms<TestItem, TestItemHandler> algorithms(data.data(), data.size(), m_centralFree);
                uint countFree = algorithms.countFreeItems();
                QCOMPARE(freeCount, countFree);
                algorithms.verifyTreeConsistent();
            }

            uint getItem(uint number) const {
                Q_ASSERT(number < (uint)data.size());
                uint ret = 0;
                uint size = (uint)data.size();
                uint current = 0;
                for(uint a = 0; a < size; ++a) {
                    if(data[a].value) {
                        //Only count the non-free items
                        if(current == number)
                            ret = data[a].value;
                        ++current;
                    }else{
                        //This is a free item
                    }
                }
                return ret;
            }

        private:
            int m_centralFree;
            QVector<TestItem> data;
    };


class TestSet {
    public:
    void add(uint i) {
        if(realSet.find(i) != realSet.end()) {
            QVERIFY(set.contains(i));
            return;
        }else{
            QVERIFY(!set.contains(i));
        }
        clock_t start = clock();
        realSet.insert(i);
        std_insertion += clock() - start;

        start = clock();
        set.insert(i);
        emb_insertion += clock() - start;

        start = clock();
        bool contained = realSet.find(i) != realSet.end();
        std_contains += clock() - start;

        start = clock();
        set.contains(i);
        emb_contains += clock() - start;

        QVERIFY(set.contains(i));
        QVERIFY(contained);
        set.verify();
    }

    void remove(uint i) {
        if(realSet.find(i) != realSet.end()) {
            QVERIFY(set.contains(i));
        }else{
            QVERIFY(!set.contains(i));
            return;
        }
        clock_t start = clock();
        set.remove(i);
        emb_removal += clock() - start;

        start = clock();
        realSet.erase(i);
        std_removal += clock() - start;


        QVERIFY(!set.contains(i));
    }

    uint size() const {
        return realSet.size();
    }

    uint getItem(uint number) const {
        Q_ASSERT(number < size());
        uint current = 0;
        uint ret = 0;

        clock_t start = clock();
        for(std::set<uint>::const_iterator it = realSet.begin(); it != realSet.end(); ++it) {
            if(current == number) {
                ret = *it;
            }
            ++current;
        }
        std_iteration += clock() - start;

        start = clock();
        set.getItem(number);
        emb_iteration += clock() - start;

        Q_ASSERT(ret);
        return ret;
    }

    void verify() {
        QVERIFY(realSet == set.toSet());
        set.verify();
    }
    private:
    std::set<uint> realSet;
    TestItemBasedSet set;
};

float toSeconds(clock_t time) {
    return ((float)time) / CLOCKS_PER_SEC;
}

struct StaticRepository {
    static Utils::BasicSetRepository* repository() {
        static Utils::BasicSetRepository repository("test repository");
        return &repository;
    }
};

struct UintSetVisitor {
    std::set<uint>& s;
    UintSetVisitor(std::set<uint>& _s) : s(_s) {
    }

    inline bool operator() (const TestItem& item) {
        s.insert(item.value);
        return true;
    }
};

struct NothingDoVisitor {
    inline bool operator() (const TestItem& item) {
        Q_UNUSED(item);
        return true;
    }
};

class TestEmbeddedFreeTree : public QObject {
  Q_OBJECT
  private slots:
    void initTestCase() {
        KDevelop::AutoTestShell::init();
        KDevelop::TestCore::initialize(KDevelop::Core::NoUi);
    }
    void cleanupTestCase() {
        KDevelop::TestCore::shutdown();
    }
    void randomizedTest() {
        const int cycles = 10000;
        const int valueRange = 1000;
        const int removeProbability = 40; //Percent

        TestSet set;
        srand(time(NULL));
        for(int a = 0; a < cycles; ++a) {
            if(a % (cycles / 10) == 0) {
                qDebug() << "cycle" << a;
            }

            bool remove = (rand() % 100) < removeProbability;
            if(remove && set.size()) {
                set.remove(set.getItem(rand() % set.size()));
            }else{
                int value = (rand() % valueRange) + 1;
                set.add(value);
            }
            set.verify();
        }
        qDebug() << "Performance embedded list: insertion:" << toSeconds(emb_insertion) << "removal:" << toSeconds(emb_removal) << "contains:" << toSeconds(emb_contains) << "iteration:" << toSeconds(emb_iteration);
        qDebug() << "Performance std::set: insertion:" << toSeconds(std_insertion) << "removal:" << toSeconds(std_removal) << "contains:" << toSeconds(std_contains) << "iteration:" << toSeconds(std_iteration);
    }
    void sequentialTest() {
        TestSet set;
        set.add(5);
        set.verify();

        set.remove(5);
        set.verify();

        set.add(3);
        set.verify();

        set.add(4);
        set.verify();

        set.add(7);
        set.verify();

        set.remove(3);
        set.verify();

        set.remove(7);
        set.verify();

        set.add(6);
        set.verify();

        set.add(1);
        set.verify();

        set.add(9);
        set.verify();

        set.remove(4);
        set.verify();

        set.remove(9);
        set.verify();

        set.add(1);
        set.verify();
        set.add(2);
        set.verify();
        set.add(3);
        set.verify();
        set.add(4);
        set.verify();
        set.add(5);
        set.verify();
        set.remove(1);
        set.verify();
        set.remove(3);
        set.verify();
        set.add(15);
        set.verify();
        set.add(16);
        set.verify();
        set.add(17);
        set.verify();
        set.add(18);
        set.verify();
        set.remove(18);
        set.verify();
        set.remove(17);
        set.verify();
        set.add(9);
        set.verify();
    }

    void testFiltering() {
        clock_t stdTime = 0;
        clock_t algoTime = 0;
        clock_t treeAlgoTime = 0;
        clock_t treeAlgoVisitorTime = 0;

        clock_t insertionStdTime = 0;
        clock_t insertionAlgoTime = 0;
        clock_t insertionTreeAlgoTime = 0;

        typedef Utils::StorableSet<TestItem, TestItemConversion, StaticRepository> RepositorySet;

        const uint cycles = 3000;
        const uint setSize = 1500;

        uint totalItems = 0, totalFilteredItems = 0;

        srand(time(NULL));
        for(uint a = 0; a < cycles; ++a) {
            KDevelop::ConvenientFreeListSet<TestItem, TestItemHandler> set1;
            std::set<uint> testSet1;

            KDevelop::ConvenientFreeListSet<TestItem, TestItemHandler> set2;
            std::set<uint> testSet2;
            RepositorySet repSet2;

            if(a % (cycles / 10) == 0) {
                qDebug() << "cycle" << a;
            }

            //Build the sets
            extractor_div_with = (rand() % 10) + 1;
            for(uint a = 0; a < setSize; ++a) {
                uint value = rand() % 3000;
                uint divValue = value/extractor_div_with;
                if(!divValue)
                    continue;
//                 qDebug() << "inserting" << value;

                std::set<uint>::const_iterator it = testSet1.lower_bound(value);
                int pos = set1.iterator().lowerBound(TestItem(value));
                //This tests the upperBound functionality
                if (pos != -1) {
                    QVERIFY(it != testSet1.end());
                    QVERIFY(set1.data()[pos].value == *it);
                } else {
                    QVERIFY(it == testSet1.end());
                }

                if((rand() % 10) == 0) {

                    set1.insert(TestItem(value));
                    testSet1.insert(value);
                }

                //This is tuned so in the end, about 99% of all declarations are filtered out, like in the symbol table.
                if((rand() % (extractor_div_with*100)) == 0) {

                    clock_t start = clock();

                    set2.insert(TestItem(divValue));

                    insertionStdTime += clock() - start;
                    start = clock();

                    testSet2.insert(divValue);

                    insertionAlgoTime += clock() - start;
                    start = clock();

                    repSet2.insert(TestItem(divValue));

                    insertionTreeAlgoTime += clock() - start;
                    start = clock();
                }
            }

            std::set<uint> verifySet1;
            for(KDevelop::ConvenientFreeListSet<TestItem, TestItemHandler>::Iterator it = set1.iterator(); it; ++it)
                verifySet1.insert(it->value);

            std::set<uint> verifySet2;
            for(KDevelop::ConvenientFreeListSet<TestItem, TestItemHandler>::Iterator it = set2.iterator(); it; ++it)
                verifySet2.insert(it->value);

            std::set<uint> verifyRepSet2;
            for(RepositorySet::Iterator it = repSet2.iterator(); it; ++it)
                verifyRepSet2.insert((*it).value);

            QCOMPARE(verifySet1, testSet1);
            QCOMPARE(verifySet2, testSet2);
            QCOMPARE(verifyRepSet2, testSet2);

            std::set<uint> algoFiltered;
            std::set<uint> treeAlgoFiltered;
            std::set<uint> treeAlgoVisitorFiltered;

            {
                //Do the filtering once without actions on the filtered items, just for calculating the time
                clock_t start = clock();

                {
                    KDevelop::ConvenientEmbeddedSetFilterIterator<TestItem, TestItemHandler, TestItem, TestItemHandler, Extractor> filterIterator(set1.iterator(), set2.iterator());
                    while(filterIterator)
                        ++filterIterator;

                    algoTime += clock() - start;
                }

                start = clock();

                {
                    KDevelop::ConvenientEmbeddedSetTreeFilterIterator<TestItem, TestItemHandler, TestItem, RepositorySet, Extractor> filterIterator(set1.iterator(), repSet2);
                    while(filterIterator)
                        ++filterIterator;

                    treeAlgoTime += clock() - start;
                }

                {
                    start = clock();

                    NothingDoVisitor v;
                    KDevelop::ConvenientEmbeddedSetTreeFilterVisitor<TestItem, TestItemHandler, TestItem, RepositorySet, Extractor, NothingDoVisitor> visit(v, set1.iterator(), repSet2);

                    treeAlgoVisitorTime += clock() - start;
                }


                start = clock();

                for(std::set<uint>::const_iterator it = testSet1.begin(); it != testSet1.end(); ++it) {
                    if(testSet2.count((*it) / extractor_div_with) == 1) {
                    }
                }

                stdTime += clock() - start;
            }

            {
                KDevelop::ConvenientEmbeddedSetFilterIterator<TestItem, TestItemHandler, TestItem, TestItemHandler, Extractor> filterIterator(set1.iterator(), set2.iterator());
                while(filterIterator) {
                    algoFiltered.insert(filterIterator->value);
                    ++filterIterator;
                }
            }
            {
                KDevelop::ConvenientEmbeddedSetTreeFilterIterator<TestItem, TestItemHandler, TestItem, RepositorySet, Extractor> filterIterator(set1.iterator(), repSet2);
                while(filterIterator) {
                    treeAlgoFiltered.insert((*filterIterator).value);
                    ++filterIterator;
                }
            }
            {
                UintSetVisitor v(treeAlgoVisitorFiltered);
                KDevelop::ConvenientEmbeddedSetTreeFilterVisitor<TestItem, TestItemHandler, TestItem, RepositorySet, Extractor, UintSetVisitor> visit(v, set1.iterator(), repSet2);
            }


            totalItems += testSet1.size();
            totalFilteredItems += algoFiltered.size();


            std::set<uint> stdFiltered;

            for(std::set<uint>::const_iterator it = testSet1.begin(); it != testSet1.end(); ++it) {
                if(testSet2.count((*it) / extractor_div_with) == 1) {
                    stdFiltered.insert(*it);
                }
            }

            QCOMPARE(algoFiltered, stdFiltered);
            QCOMPARE(treeAlgoFiltered, stdFiltered);
            QCOMPARE(treeAlgoVisitorFiltered, stdFiltered);

        }
        qDebug() << "Filtering performance: embedded-list filtering:" << toSeconds(algoTime) << "set-repository filtering:" << toSeconds(treeAlgoTime) << "set-repository visitor filtering:" << toSeconds(treeAlgoVisitorTime) << "std::set filtering:" << toSeconds(stdTime) << "Normal -> Embedded speedup ratio:" << (toSeconds(stdTime) / toSeconds(algoTime)) << "Normal -> Repository speedup ratio:" << (toSeconds(stdTime) / toSeconds(treeAlgoVisitorTime)) << "total processed items:" << totalItems << "total items after filtering:" << totalFilteredItems;
        qDebug() << "Insertion: embedded-list:" << toSeconds(insertionAlgoTime) << "set-repository:" << toSeconds(insertionTreeAlgoTime) << "std::set:" << toSeconds(insertionStdTime);

    }
};

#include "test_embeddedfreetree.moc"

QTEST_MAIN(TestEmbeddedFreeTree)
