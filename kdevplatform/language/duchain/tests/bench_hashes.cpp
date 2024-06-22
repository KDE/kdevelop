/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bench_hashes.h"

#include <serialization/indexedstring.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <QDateTime>
#include <QVector>
#include <QTest>

#include <unordered_map>

// similar to e.g. modificationrevision.cpp
struct DataT
{
    QDateTime a;
    QDateTime b;
};

using DataPair = QPair<KDevelop::IndexedString, DataT>;
using InputData = QVector<DataPair>;

struct IndexedStringHash
{
    inline uint operator()(const KDevelop::IndexedString& str) const
    {
        return str.hash();
    }
};

using StlHash = std::unordered_map<KDevelop::IndexedString, DataT, IndexedStringHash>;

inline void insertData(StlHash& hash, const InputData& data)
{
    for (const DataPair& pair : data) {
        hash.insert(std::make_pair(pair.first, pair.second));
    }
}

using QStringHash = QHash<KDevelop::IndexedString, DataT>;
inline void insertData(QStringHash& hash, const InputData& data)
{
    for (const DataPair& pair : data) {
        hash.insert(pair.first, pair.second);
    }
}

QTEST_GUILESS_MAIN(BenchHashes)

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
    const QVector<int> sizes{100, 1000, 10000, 100000};
    for (int size : sizes) {
        for (int i = data.size(); i < size; ++i) {
            data << qMakePair(IndexedString(QString::number(i)), DataT());
        }

        QCOMPARE(data.size(), size);
        QTest::newRow(qPrintable(QStringLiteral("unordered_map-%1").arg(size)))
            << true << data;
        QTest::newRow(qPrintable(QStringLiteral("qhash-%1").arg(size)))
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
    QFETCH(const InputData, data);

    if (useStl) {
        StlHash hash;
        insertData(hash, data);
        QBENCHMARK {
            for (const DataPair& pair : data) {
                ( void ) hash.find(pair.first);
            }
        }
    } else {
        QStringHash hash;
        insertData(hash, data);
        QBENCHMARK {
            for (const DataPair& pair : data) {
                ( void ) hash.find(pair.first);
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
    QFETCH(const InputData, data);

    if (useStl) {
        StlHash hash;
        insertData(hash, data);
        const StlHash& constHash = hash;
        QBENCHMARK {
            for (const DataPair& pair : data) {
                ( void ) constHash.find(pair.first);
            }
        }
    } else {
        QStringHash hash;
        insertData(hash, data);
        QBENCHMARK {
            for (const DataPair& pair : data) {
                ( void ) hash.constFind(pair.first);
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
    QFETCH(const InputData, data);

    if (useStl) {
        StlHash hash;
        insertData(hash, data);
        QBENCHMARK {
            for (const DataPair& pair : data) {
                hash.erase(pair.first);
            }
        }
    } else {
        QStringHash hash;
        insertData(hash, data);
        QBENCHMARK {
            for (const DataPair& pair : data) {
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
    size_t size = 0;
    void* ptr = nullptr;
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
        for (int i = 0; i < 100; ++i) {
            v.append(new TypeRepoTestData);
        }

        if (type == 1) {
            QBENCHMARK {
                for (int i = 0; i < 100; ++i) {
                    v.at(i)->size++;
                }
            }
        } else if (type == 2) {
            TypeRepoTestData** a = v.data();
            QBENCHMARK {
                for (int i = 0; i < 100; ++i) {
                    a[i]->size++;
                }
            }
        }
        qDeleteAll(v);
    } else if (type == 3) {
        QHash<int, TypeRepoTestData*> v;
        for (int i = 0; i < 100; ++i) {
            v[i] = new TypeRepoTestData;
        }

        QBENCHMARK {
            for (int i = 0; i < 100; ++i) {
                v.value(i)->size++;
            }
        }
        qDeleteAll(v);
    } else if (type == 4) {
        QMap<int, TypeRepoTestData*> v;
        for (int i = 0; i < 100; ++i) {
            v[i] = new TypeRepoTestData;
        }

        QBENCHMARK {
            for (int i = 0; i < 100; ++i) {
                v.value(i)->size++;
            }
        }
        qDeleteAll(v);
    } else if (type == 5) {
        std::unordered_map<int, TypeRepoTestData*> v;
        for (int i = 0; i < 100; ++i) {
            v[i] = new TypeRepoTestData;
        }

        QBENCHMARK {
            for (int i = 0; i < 100; ++i) {
                v.at(i)->size++;
            }
        }
        for (const auto& [key, value] : v) {
            delete value;
        }
    } else if (type == 6) {
        // for the idea, look at c++'s lexer.cpp
        const int vectors = 5;
        using Pair = QPair<int, TypeRepoTestData*>;
        using InnerVector = QVarLengthArray<Pair, vectors>;
        QVarLengthArray <InnerVector, 10> v;
        v.resize(vectors);
        for (int i = 0; i < 100; ++i) {
            v[i % vectors] << qMakePair(i, new TypeRepoTestData);
        }

        QBENCHMARK {
            for (int i = 0; i < 100; ++i) {
                for (const Pair& p : std::as_const(v.at(i % vectors))) {
                    if (p.first == i) {
                        p.second->size++;
                        break;
                    }
                }
            }
        }
        for (const auto& inner : v) {
            for (const auto& [key, value] : inner) {
                delete value;
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

#include "moc_bench_hashes.cpp"
