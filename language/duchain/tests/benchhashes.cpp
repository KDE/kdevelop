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

#include "benchhashes.h"

#include <language/duchain/indexedstring.h>

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include "config-kdevplatform.h"

// similar to e.g. modificationrevision.cpp
struct DataT {
  timeval a;
  QDateTime b;
};

typedef QPair<KDevelop::IndexedString, DataT> DataPair;
typedef QVector<DataPair> InputData;

#ifdef HAVE_UNORDERED_MAP
#include <unordered_map>

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

#define HAVE_STL_HASH
#endif

typedef QHash<KDevelop::IndexedString, DataT> QStringHash;
inline void insertData(QStringHash& hash, const InputData& data)
{
  foreach(const DataPair& pair, data) {
    hash.insert(pair.first, pair.second);
  }
}

QTEST_KDEMAIN(BenchHashes, NoGUI);

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
#ifdef HAVE_STL_HASH
    QTest::newRow(qPrintable(QString("unordered_map-%1").arg(size)))
      << true << data;
#endif
    QTest::newRow(qPrintable(QString("qhash-%1").arg(size)))
      << false << data;
  }
}

void BenchHashes::insert()
{
  QFETCH(bool, useStl);
  QFETCH(InputData, data);

  if (useStl) {
#ifdef HAVE_STL_HASH
    QBENCHMARK {
      StlHash hash;
      insertData(hash, data);
    }
#endif
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
#ifdef HAVE_STL_HASH
    StlHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        (void) hash.find(pair.first);
      }
    }
#endif
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
#ifdef HAVE_STL_HASH
    StlHash hash;
    insertData(hash, data);
    const StlHash& constHash = hash;
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        (void) constHash.find(pair.first);
      }
    }
#endif
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
#ifdef HAVE_STL_HASH
    StlHash hash;
    insertData(hash, data);
    QBENCHMARK {
      foreach(const DataPair& pair, data) {
        hash.erase(pair.first);
      }
    }
#endif
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

#include "benchhashes.moc"
