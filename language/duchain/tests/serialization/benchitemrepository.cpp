/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "benchitemrepository.h"


#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <language/duchain/repositories/itemrepository.h>
#include <language/duchain/indexedstring.h>

#include <algorithm>

QTEST_KDEMAIN(BenchItemRepository, NoGUI);

using namespace KDevelop;

struct TestData
{
  uint length;
  uint itemSize() const
  {
    return sizeof(TestData) + length;
  }
  uint hash() const
  {
    const char* str = ((const char*)this) + sizeof(TestData);
    return IndexedString::hashString(str, length);
  }
};

struct TestDataRepositoryItemRequest
{
  //The text is supposed to be utf8 encoded
  TestDataRepositoryItemRequest(const char* text, uint length)
  : m_length(length)
  , m_text(text)
  , m_hash(IndexedString::hashString(text, length))
  {
  }

  enum {
    AverageSize = 10 //This should be the approximate average size of an Item
  };

  typedef uint HashType;

  //Should return the hash-value associated with this request(For example the hash of a string)
  HashType hash() const
  {
    return m_hash;
  }

  //Should return the size of an item created with createItem
  uint itemSize() const
  {
    return sizeof(TestData) + m_length;
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(TestData* item) const
  {
    item->length = m_length;
    ++item;
    memcpy(item, m_text, m_length);
  }

  static void destroy(TestData* item, KDevelop::AbstractItemRepository&)
  {
    Q_UNUSED(item);
    //Nothing to do here (The object is not intelligent)
  }

  static bool persistent(const TestData* item)
  {
    Q_UNUSED(item);
    return true;
  }

  //Should return whether the here requested item equals the given item
  bool equals(const TestData* item) const
  {
    return item->length == m_length && (memcmp(++item, m_text, m_length) == 0);
  }
  unsigned short m_length;
  const char* m_text;
  unsigned int m_hash;
};

typedef ItemRepository<TestData, TestDataRepositoryItemRequest, false, true> TestDataRepository;

void BenchItemRepository::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);
}

void BenchItemRepository::cleanupTestCase()
{
  TestCore::shutdown();
}

QVector<QString> generateData()
{
  QVector<QString> data;
  static const int NUM_ITEMS = 100000;
  data.resize(NUM_ITEMS);
  for(int i = 0; i < NUM_ITEMS; ++i) {
    data[i] = QString("/foo/%1").arg(i);
  }
  return data;
}

QVector<uint> insertData(const QVector<QString>& data, TestDataRepository& repo)
{
  QVector<uint> indices;
  indices.reserve(data.size());
  foreach(const QString& item, data) {
    const QByteArray byteArray = item.toUtf8();
    indices << repo.index(TestDataRepositoryItemRequest(byteArray.constData(), byteArray.length()));
  }
  return indices;
}

void BenchItemRepository::insert()
{
  TestDataRepository repo("TestDataRepositoryInsert");
  QVector<QString> data = generateData();
  QVector<uint> indices;
  QBENCHMARK_ONCE {
    indices = insertData(data, repo);
    repo.store();
  }
  Q_ASSERT(indices.size() == data.size());
  QCOMPARE(repo.statistics().totalItems, uint(data.size()));
}

void BenchItemRepository::remove()
{
  TestDataRepository repo("TestDataRepositoryRemove");
  QVector<QString> data = generateData();
  QVector<uint> indices = insertData(data, repo);
  repo.store();
  QVERIFY(indices.size() == indices.toList().toSet().size());
  QVERIFY(indices.size() == data.size());
  QBENCHMARK_ONCE {
    foreach(uint index, indices) {
      repo.deleteItem(index);
    }
    repo.store();
  }
  QCOMPARE(repo.statistics().totalItems, 0u);
}

void BenchItemRepository::removeDisk()
{
  QVector<QString> data = generateData();
  QVector<uint> indices;
  {
    TestDataRepository repo("TestDataRepositoryRemoveDisk");
    indices = insertData(data, repo);
    repo.store();
  }
  TestDataRepository repo("TestDataRepositoryRemoveDisk");
  QVERIFY(repo.statistics().totalItems == static_cast<uint>(data.size()));
  QBENCHMARK_ONCE {
    foreach(uint index, indices) {
      repo.deleteItem(index);
    }
    repo.store();
  }
  QCOMPARE(repo.statistics().totalItems, 0u);
}

void BenchItemRepository::lookupKey()
{
  TestDataRepository repo("TestDataRepositoryLookupKey");
  QVector<QString> data = generateData();
  QVector<uint> indices = insertData(data, repo);
  srand(0);
  std::random_shuffle(indices.begin(), indices.end());
  QBENCHMARK {
    foreach(uint index, indices) {
      repo.itemFromIndex(index);
    }
  }
}

void BenchItemRepository::lookupValue()
{
  TestDataRepository repo("TestDataRepositoryLookupValue");
  QVector<QString> data = generateData();
  QVector<uint> indices = insertData(data, repo);
  srand(0);
  std::random_shuffle(indices.begin(), indices.end());
  QBENCHMARK {
    foreach(const QString& item, data) {
      const QByteArray byteArray = item.toUtf8();
      repo.findIndex(TestDataRepositoryItemRequest(byteArray.constData(), byteArray.length()));
    }
  }
}

#include "benchitemrepository.moc"
