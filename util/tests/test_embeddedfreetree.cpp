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
#include <QObject>
#include <QtTest/QtTest>
#include <QVector>
#include <stdlib.h>
#include <set>
#include <util/embeddedfreetree.h>
#include <kdebug.h>

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

clock_t std_insertion = 0, std_removal = 0, std_contains = 0, std_iteration = 0, emb_insertion = 0, emb_removal = 0, emb_contains = 0, emb_iteration = 0;

QString toString(std::set<uint> set) {
    QString ret;
    for(std::set<uint>::const_iterator it = set.begin(); it != set.end(); ++it)
        ret += QString("%1 ").arg(*it);
    return ret;
}

bool operator==(const std::set<uint> a, const std::set<uint> b) {
    if(a.size() != b.size()) {
        kDebug() << "size mismatch" << toString(a) << ": " << toString(b);
        return false;
    }
    std::set<uint>::const_iterator aIt = a.begin();
    std::set<uint>::const_iterator bIt = b.begin();
    for(; aIt != a.end(); ++aIt, ++bIt)
        if(*aIt != *bIt) {
            kDebug() << "mismatch" << toString(a) << ": " << toString(b);
            return false;
        }
    return true;
}

    struct TestItemHandler {
        public:
        TestItemHandler (const TestItem& data) : m_data(const_cast<TestItem&>(data)) {
        }
        int rightChild() const {
            return m_data.rightChild;
        }
        int leftChild() const {
            return m_data.leftChild;
        }
        void setLeftChild(int child) {
            m_data.leftChild = child;
        }
        void setRightChild(int child) {
            m_data.rightChild = child;
        }
        bool operator<(const TestItemHandler & rhs) const {
            Q_ASSERT(m_data.value && rhs.m_data.value);
            return m_data.value < rhs.m_data.value;
        }
        //Copies this item into the given one
        void copyTo(TestItem& data) const {
            data = m_data;
        }
        static void createFreeItem(TestItem& data) {
            data = TestItem();
        }
        bool isFree() const {
            return !m_data.value;
        }
        
        const TestItem& item() const {
            return m_data;
        }

        TestItem& item() {
            return m_data;
        }

        private:
            TestItem& m_data;
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
                        Q_ASSERT(last < data[a].value);
                        last = data[a].value;
                    }else{
                        ++freeCount;
                    }
                }
                KDevelop::EmbeddedTreeAlgorithms<TestItem, TestItemHandler> algorithms(data.data(), data.size(), m_centralFree);
                uint countFree = algorithms.countFreeItems();
                Q_ASSERT(freeCount == countFree);
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
            Q_ASSERT(set.contains(i));
            return;
        }else{
            Q_ASSERT(!set.contains(i));
        }
        clock_t start = clock();
        realSet.insert(i);
        std_insertion += clock() - start;
        
        start = clock();
        set.insert(i);
        emb_insertion += clock() - start;
        
        start = clock();
        realSet.find(i) == realSet.end();
        std_contains += clock() - start;
        
        start = clock();
        set.contains(i);
        emb_contains += clock() - start;
        
        Q_ASSERT(set.contains(i));
        set.verify();
    }
    
    void remove(uint i) {
        if(realSet.find(i) != realSet.end()) {
            Q_ASSERT(set.contains(i));
        }else{
            Q_ASSERT(!set.contains(i));
            return;
        }
        clock_t start = clock();
        set.remove(i);
        emb_removal += clock() - start;
        
        start = clock();
        realSet.erase(i);
        std_removal += clock() - start;
        
        
        Q_ASSERT(!set.contains(i));
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
        Q_ASSERT(realSet == set.toSet());
        set.verify();
    }
    private:
    std::set<uint> realSet;
    TestItemBasedSet set;
};

const int cycles = 100000;
const int valueRange = 1000;
const int removeProbability = 40; //Percent

float toSeconds(clock_t time) {
    return ((float)time) / CLOCKS_PER_SEC;
}

class TestEmbeddedFreeTree : public QObject {
  Q_OBJECT
  private slots:
    
    void randomizedTest() {
        TestSet set;
        srand(time(NULL));
        for(int a = 0; a < cycles; ++a) {
            if(a % (cycles / 10) == 0)
                kDebug() << "cycle" << a;
            
            bool remove = (rand() % 100) < removeProbability;
            if(remove && set.size()) {
                set.remove(set.getItem(rand() % set.size()));
            }else{
                int value = (rand() % valueRange) + 1;
                set.add(value);
            }
            set.verify();
        }
        kDebug() << "Performance embedded list: insertion:" << toSeconds(emb_insertion) << "removal:" << toSeconds(emb_removal) << "contains:" << toSeconds(emb_contains) << "iteration:" << toSeconds(emb_iteration);
        kDebug() << "Performance std::set: insertion:" << toSeconds(std_insertion) << "removal:" << toSeconds(std_removal) << "contains:" << toSeconds(std_contains) << "iteration:" << toSeconds(std_iteration);
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
};

#include "test_itemrepository.moc"

QTEST_MAIN(TestEmbeddedFreeTree);
