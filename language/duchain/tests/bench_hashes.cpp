/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "bench_hashes.h"

#include <serialization/indexedstring.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <QDateTime>
#include <QTextCodec>
#include <QVector>
#include <QtTest/QTest>

#include <unordered_map>

// similar to e.g. modificationrevision.cpp
struct DataT {
  timeval a;
  QDateTime b;
};

typedef QPair<KDevelop::IndexedString, DataT> DataPair;
typedef QVector<DataPair> InputData;


struct IndexedStringHash
{
  inline uint operator() (const KDevelop::IndexedString& str) const
  {
    return str.hash();
  }
};

typedef std::unordered_map<KDevelop::IndexedString, DataT, IndexedStringHash> StlHash;

inline void insertData(StlHash& hash, const InputData& data)
{
  foreach(const DataPair& pair, data) {
    hash.insert(std::make_pair(pair.first, pair.second));
  }
}

typedef QHash<KDevelop::IndexedString, DataT> QStringHash;
inline void insertData(QStringHash& hash, const InputData& data)
{
  foreach(const DataPair& pair, data) {
    hash.insert(pair.first, pair.second);
  }
}

QTEST_GUILESS_MAIN(BenchHashes);

using namespace KDevelop;

Q_DECLARE_METATYPE(InputData)

void BenchHashes::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);

  qRegisterMetaType<InputData>();
}

void BenchHashes::cleanupTestCase()
{
  TestCore::shutdown();
}

void BenchHashes::feedData()
{
  QTest::addColumn<bool>("useStl");
  QTest::addColumn<InputData>("data");

  InputData data;
  QVector<int> sizes = QVector<int>() << 100 << 1000 << 10000 << 100000;
  foreach(int size, sizes) {
    for(int i = data.size(); i < size; ++i) {
      data << qMakePair(IndexedString(QString::number(i)), DataT());
    }
    QCOMPARE(data.size(), size);
    QTest::newRow(qPrintable(QString("unordered_map-%1").arg(size)))
      << true << data;
    QTest::newRow(qPrintable(QString("qhash-%1").arg(size)))
      << false << data;
  }
}

void BenchHashes::insert()
{
  QFETCH(bool, useStl);
  QFETCH(InputData, data);

  if (useStl) {
    QBENCHMARK {
      StlHash hash;
      insertData(hash, data);
    }
  } else {
    QBENCHMARK {
      QStringHash hash;
      insertData(hash, data);
    }
  }
}

void BenchHashes::insert_data()
{
  feedData();
}

void BenchHashes::find()
{
  QFETCH(bool, useStl);
  QFETCH(InputData, data);

  if(useStl) {
    StlHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        (void) hash.find(pair.first);
      }
    }
  } else {
    QStringHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        (void) hash.find(pair.first);
      }
    }
  }
}

void BenchHashes::find_data()
{
  feedData();
}

void BenchHashes::constFind()
{
  QFETCH(bool, useStl);
  QFETCH(InputData, data);

  if(useStl) {
    StlHash hash;
    insertData(hash, data);
    const StlHash& constHash = hash;
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        (void) constHash.find(pair.first);
      }
    }
  } else {
    QStringHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        (void) hash.constFind(pair.first);
      }
    }
  }
}

void BenchHashes::constFind_data()
{
  feedData();
}

void BenchHashes::remove()
{
  QFETCH(bool, useStl);
  QFETCH(InputData, data);

  if(useStl) {
    StlHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        hash.erase(pair.first);
      }
    }
  } else {
    QStringHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        hash.remove(pair.first);
      }
    }
  }
}

void BenchHashes::remove_data()
{
  feedData();
}

struct TypeRepoTestData
{
  size_t size;
  void* ptr;
};

/**
 * somewhat artificial benchmark to test speed impact if we'd ever change
 * the underlying data type of the TypeSystem / TypeRegister.
 */
void BenchHashes::typeRepo()
{
  QFETCH(int, type);
  if (type == 1 || type == 2) {
    QVector<TypeRepoTestData*> v;
    for(int i = 0; i < 100; ++i) {
      v.append(new TypeRepoTestData);
    }
    if (type == 1) {
      QBENCHMARK {
        for(int i = 0; i < 100; ++i) {
          v.at(i)->size++;
        }
      }
    } else if (type == 2) {
      TypeRepoTestData** a = v.data();
      QBENCHMARK {
        for(int i = 0; i < 100; ++i) {
          a[i]->size++;
        }
      }
    }
  } else if (type == 3) {
    QHash<int, TypeRepoTestData*> v;
    for(int i = 0; i < 100; ++i) {
      v[i] = new TypeRepoTestData;
    }
    QBENCHMARK {
      for(int i = 0; i < 100; ++i) {
        v.value(i)->size++;
      }
    }
  } else if (type == 4) {
    QMap<int, TypeRepoTestData*> v;
    for(int i = 0; i < 100; ++i) {
      v[i] = new TypeRepoTestData;
    }
    QBENCHMARK {
      for(int i = 0; i < 100; ++i) {
        v.value(i)->size++;
      }
    }
  } else if (type == 5) {
    std::unordered_map<int, TypeRepoTestData*> v;
    for(int i = 0; i < 100; ++i) {
      v[i] = new TypeRepoTestData;
    }
    QBENCHMARK {
      for(int i = 0; i < 100; ++i) {
        v.at(i)->size++;
      }
    }
  } else if (type == 6) {
    // for the idea, look at c++'s lexer.cpp
    const int vectors = 5;
    typedef QPair<int, TypeRepoTestData*> Pair;
    typedef QVarLengthArray<Pair, vectors> InnerVector;
    QVarLengthArray <InnerVector, 10> v;
    v.resize(vectors);
    for(int i = 0; i < 100; ++i) {
      v[i % vectors] << qMakePair(i, new TypeRepoTestData);
    }
    QBENCHMARK {
      for(int i = 0; i < 100; ++i) {
        foreach(const Pair& p, v.at(i % vectors)) {
          if (p.first == i) {
            p.second->size++;
            break;
          }
        }
      }
    }
  } else if (type == 0) {
    QBENCHMARK {}
  }
}

void BenchHashes::typeRepo_data()
{
  QTest::addColumn<int>("type");

  QTest::newRow("noop") << 0;
  QTest::newRow("vector") << 1;
  QTest::newRow("vector-raw") << 2;
  QTest::newRow("qhash") << 3;
  QTest::newRow("qmap") << 4;
  QTest::newRow("unordered_map") << 5;
  QTest::newRow("nested-vector") << 6;
}
