/*
 * This file is part of KDevelop
 *
 * Copyright 2011 Milian Wolff <mail@milianw.de>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "test_duchain.h"

#include <QTest>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/codemodel.h>

#include <language/codegen/coderepresentation.h>

#include <language/util/setrepository.h>
#include <language/util/basicsetrepository.h>

// #include <typeinfo>
#include <time.h>
#include <set>
#include <algorithm>
#include <iterator> // needed for std::insert_iterator on windows
#include <QThread>

//Extremely slow
// #define TEST_NORMAL_IMPORTS

//Uncomment the following line to get additional output from the string-repository test
//#define DEBUG_STRINGREPOSITORY

QTEST_MAIN(TestDUChain)

using namespace KDevelop;
using namespace Utils;

typedef BasicSetRepository::Index Index;

void TestDUChain::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);

  DUChain::self()->disablePersistentStorage();
  CodeRepresentation::setDiskChangesForbidden(true);
}

void TestDUChain::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestDUChain::testStringSets() {

  const unsigned int setCount = 8;
  const unsigned int choiceCount = 40;
  const unsigned int itemCount = 120;

  BasicSetRepository rep("test repository");

//  kDebug() << "Start repository-layout: \n" << rep.dumpDotGraph();

  clock_t repositoryClockTime = 0; //Time spent on repository-operations
  clock_t genericClockTime = 0; //Time spend on equivalent operations with generic sets

  clock_t repositoryIntersectionClockTime = 0; //Time spent on repository-operations
  clock_t genericIntersectionClockTime = 0; //Time spend on equivalent operations with generic sets
  clock_t qsetIntersectionClockTime = 0; //Time spend on equivalent operations with generic sets

  clock_t repositoryUnionClockTime = 0; //Time spent on repository-operations
  clock_t genericUnionClockTime = 0; //Time spend on equivalent operations with generic sets

  clock_t repositoryDifferenceClockTime = 0; //Time spent on repository-operations
  clock_t genericDifferenceClockTime = 0; //Time spend on equivalent operations with generic sets

  Set sets[setCount];
  std::set<Index> realSets[setCount];
  for(unsigned int a = 0; a < setCount; a++)
  {
    std::set<Index> chosenIndices;
    unsigned int thisCount = rand() % choiceCount;
    if(thisCount == 0)
      thisCount = 1;

    for(unsigned int b = 0; b < thisCount; b++)
    {
      Index choose = (rand() % itemCount) + 1;
      while(chosenIndices.find(choose) != chosenIndices.end()) {
        choose = (rand() % itemCount) + 1;
      }

      clock_t c = clock();
      chosenIndices.insert(chosenIndices.end(), choose);
      genericClockTime += clock() - c;
    }

    clock_t c = clock();
    sets[a] = rep.createSet(chosenIndices);
    repositoryClockTime += clock() - c;

    realSets[a] = chosenIndices;

    std::set<Index> tempSet = sets[a].stdSet();

    if(tempSet != realSets[a]) {
      QString dbg = "created set: ";
      for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
        dbg += QString("%1 ").arg(*it);
      kDebug() << dbg;

      dbg = "repo.   set: ";
      for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
        dbg += QString("%1 ").arg(*it);
      kDebug() << dbg;

      kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
    }
    QVERIFY(tempSet == realSets[a]);
  }

  for(int cycle = 0; cycle < 100; ++cycle) {
      if(cycle % 10 == 0)
         kDebug() << "cycle" << cycle;

    for(unsigned int a = 0; a < setCount; a++) {
      for(unsigned int b = 0; b < setCount; b++) {
        /// ----- SUBTRACTION/DIFFERENCE
        std::set<Index> _realDifference;
        clock_t c = clock();
        std::set_difference(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realDifference, _realDifference.begin()));
        genericDifferenceClockTime += clock() - c;

        c = clock();
        Set _difference = sets[a] - sets[b];
        repositoryDifferenceClockTime += clock() - c;

        if(_difference.stdSet() != _realDifference)
        {
          {
            kDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            kDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _difference.stdSet();

            kDebug() << "SET difference:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realDifference.begin(); it != _realDifference.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << _difference.dumpDotGraph() << "\n\n";
          }
        }
        QVERIFY(_difference.stdSet() == _realDifference);


        /// ------ UNION

        std::set<Index> _realUnion;
        c = clock();
        std::set_union(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realUnion, _realUnion.begin()));
        genericUnionClockTime += clock() - c;

        c = clock();
        Set _union = sets[a] + sets[b];
        repositoryUnionClockTime += clock() - c;

        if(_union.stdSet() != _realUnion)
        {
          {
            kDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            kDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _union.stdSet();

            kDebug() << "SET union:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realUnion.begin(); it != _realUnion.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << _union.dumpDotGraph() << "\n\n";
          }
        }
        QVERIFY(_union.stdSet() == _realUnion);

        std::set<Index> _realIntersection;

        /// -------- INTERSECTION
        c = clock();
        std::set_intersection(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realIntersection, _realIntersection.begin()));
        genericIntersectionClockTime += clock() - c;

        //Just for fun: Test how fast QSet intersections are
        QSet<Index> first, second;
        for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
          first.insert(*it);
        for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
          second.insert(*it);
        c = clock();
        QSet<Index> i = first.intersect(second);
        qsetIntersectionClockTime += clock() - c;

        c = clock();
        Set _intersection = sets[a] & sets[b];
        repositoryIntersectionClockTime += clock() - c;


        if(_intersection.stdSet() != _realIntersection)
        {
          {
            kDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            kDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _intersection.stdSet();

            kDebug() << "SET intersection:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realIntersection.begin(); it != _realIntersection.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QString("%1 ").arg(*it);
            kDebug() << dbg;

            kDebug() << "DOT-Graph:\n\n" << _intersection.dumpDotGraph() << "\n\n";
          }
        }
        QVERIFY(_intersection.stdSet() == _realIntersection);
      }
    }
#ifdef DEBUG_STRINGREPOSITORY
    kDebug() << "cycle " << cycle;
  kDebug() << "Clock-cycles needed for set-building: repository-set: " << repositoryClockTime << " generic-set: " << genericClockTime;
  kDebug() << "Clock-cycles needed for intersection: repository-sets: " << repositoryIntersectionClockTime << " generic-set: " << genericIntersectionClockTime << " QSet: " << qsetIntersectionClockTime;
  kDebug() << "Clock-cycles needed for union: repository-sets: " << repositoryUnionClockTime << " generic-set: " << genericUnionClockTime;
  kDebug() << "Clock-cycles needed for difference: repository-sets: " << repositoryDifferenceClockTime << " generic-set: " << genericDifferenceClockTime;
#endif
  }
}

void TestDUChain::testSymbolTableValid() {
  DUChainReadLocker lock(DUChain::lock());
  PersistentSymbolTable::self().selfAnalysis();
}

void TestDUChain::testIndexedStrings() {

  int testCount  = 600000;

  QHash<QString, IndexedString> knownIndices;
  int a = 0;
  for(a = 0; a < testCount; ++a) {
    QString testString;
    int length = rand() % 10;
    for(int b = 0; b < length; ++b)
      testString.append((char)(rand() % 6) + 'a');
    QByteArray array = testString.toUtf8();
    //kDebug() << "checking with" << testString;
    //kDebug() << "checking" << a;
    IndexedString indexed(array.constData(), array.size(), IndexedString::hashString(array.constData(), array.size()));

    QCOMPARE(indexed.str(), testString);
    if(knownIndices.contains(testString)) {
      QCOMPARE(indexed.index(), knownIndices[testString].index());
    } else {
      knownIndices[testString] = indexed;
    }

    if(a % (testCount/10) == 0)
      kDebug() << a << "of" << testCount;
  }
  kDebug() << a << "successful tests";
}

struct TestContext {
  TestContext() {
    static int number = 0;
    ++number;
    DUChainWriteLocker lock(DUChain::lock());
    m_context = new TopDUContext(IndexedString(QString("/test1/%1").arg(number)), RangeInRevision());
    m_normalContext = new DUContext(RangeInRevision(), m_context);
    DUChain::self()->addDocumentChain(m_context);
    Q_ASSERT(IndexedDUContext(m_context).context() == m_context);
  }

  ~TestContext() {
    foreach(TestContext* importer, importers)
      importer->unImport(QList<TestContext*>() << this);
    unImport(imports);

    DUChainWriteLocker lock(DUChain::lock());
    TopDUContextPointer tp(m_context);
    DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(m_context));
    Q_ASSERT(!tp);
  }

  void verify(QList<TestContext*> allContexts) {

    {
      DUChainReadLocker lock(DUChain::lock());
      QCOMPARE(m_context->importedParentContexts().count(), imports.count());
    }
    //Compute a closure of all children, and verify that they are imported.
    QSet<TestContext*> collected;
    collectImports(collected);
    collected.remove(this);

    DUChainReadLocker lock(DUChain::lock());
    foreach(TestContext* context, collected) {
      QVERIFY(m_context->imports(context->m_context, CursorInRevision::invalid()));
#ifdef TEST_NORMAL_IMPORTS
      QVERIFY(m_normalContext->imports(context->m_normalContext));
#endif
    }
    //Verify that no other contexts are imported

    foreach(TestContext* context, allContexts)
      if(context != this) {
        QVERIFY(collected.contains(context) || !m_context->imports(context->m_context, CursorInRevision::invalid()));
#ifdef TEST_NORMAL_IMPORTS
        QVERIFY(collected.contains(context) || !m_normalContext->imports(context->m_normalContext, CursorInRevision::invalid()));
#endif
      }
  }

  void collectImports(QSet<TestContext*>& collected) {
    if(collected.contains(this))
      return;
    collected.insert(this);
    foreach(TestContext* context, imports)
      context->collectImports(collected);
  }
  void import(TestContext* ctx) {
    if(imports.contains(ctx) || ctx == this)
      return;
    imports << ctx;
    ctx->importers << this;
    DUChainWriteLocker lock(DUChain::lock());
    m_context->addImportedParentContext(ctx->m_context);
#ifdef TEST_NORMAL_IMPORTS
    m_normalContext->addImportedParentContext(ctx->m_normalContext);
#endif
  }

  void unImport(QList<TestContext*> ctxList) {
    QList<TopDUContext*> list;
    QList<DUContext*> normalList;

    foreach(TestContext* ctx, ctxList) {
      if(!imports.contains(ctx))
        continue;
      list << ctx->m_context;
      normalList << ctx->m_normalContext;

      imports.removeAll(ctx);
      ctx->importers.removeAll(this);
    }
    DUChainWriteLocker lock(DUChain::lock());
    m_context->removeImportedParentContexts(list);

#ifdef TEST_NORMAL_IMPORTS
    foreach(DUContext* ctx, normalList)
      m_normalContext->removeImportedParentContext(ctx);
#endif
  }

  void clearImports() {

    {
      DUChainWriteLocker lock(DUChain::lock());

      m_context->clearImportedParentContexts();
      m_normalContext->clearImportedParentContexts();
    }
    foreach(TestContext* ctx, imports) {
      imports.removeAll(ctx);
      ctx->importers.removeAll(this);
    }
  }

  QList<TestContext*> imports;

  private:

  TopDUContext* m_context;
  DUContext* m_normalContext;
  QList<TestContext*> importers;
};

void collectReachableNodes(QSet<uint>& reachableNodes, uint currentNode) {
  if(!currentNode)
    return;
  reachableNodes.insert(currentNode);
  const Utils::SetNodeData* node = KDevelop::RecursiveImportRepository::repository()->nodeFromIndex(currentNode);
  Q_ASSERT(node);
  collectReachableNodes(reachableNodes, node->leftNode());
  collectReachableNodes(reachableNodes, node->rightNode());
}

uint collectNaiveNodeCount(uint currentNode) {
  if(!currentNode)
    return 0;
  uint ret = 1;
  const Utils::SetNodeData* node = KDevelop::RecursiveImportRepository::repository()->nodeFromIndex(currentNode);
  Q_ASSERT(node);
  ret += collectNaiveNodeCount(node->leftNode());
  ret += collectNaiveNodeCount(node->rightNode());
  return ret;
}

void TestDUChain::testImportStructure()
{
  clock_t startClock = clock();
  kDebug() << "before: " << KDevelop::RecursiveImportRepository::repository()->getDataRepository().statistics().print();

  ///Maintains a naive import-structure along with a real top-context import structure, and allows comparing both.
  int cycles = 5;
  //int cycles = 100;
  //srand(time(NULL));
  for(int t = 0; t < cycles; ++t) {
    QList<TestContext*> allContexts;
    //Create a random structure
    int contextCount = 120;
    int verifyOnceIn = contextCount/*((contextCount*contextCount)/20)+1*/; //Verify once in every chances(not in all cases, becase else the import-structure isn't built on-demand!)
    int clearOnceIn = contextCount;
    for(int a = 0; a < contextCount; a++)
      allContexts << new TestContext();
    for(int c = 0; c < cycles; ++c) {
      //kDebug() << "main-cycle" << t  << "sub-cycle" << c;
      //Add random imports and compare
      for(int a = 0; a < contextCount; a++) {
        //Import up to 5 random other contexts into each context
        int importCount = rand() % 5;
        //kDebug()   << "cnt> " << importCount;
        for(int i = 0; i < importCount; ++i)
        {
          //int importNr = rand() % contextCount;
          //kDebug() << "nmr > " << importNr;
          //allContexts[a]->import(allContexts[importNr]);
          allContexts[a]->import(allContexts[rand() % contextCount]);
        }
        for(int b = 0; b < contextCount; b++)
          if(rand() % verifyOnceIn == 0)
            allContexts[b]->verify(allContexts);
      }

      //Remove random imports and compare
      for(int a = 0; a < contextCount; a++) {
        //Import up to 5 random other contexts into each context
        int removeCount = rand() % 3;
        QSet<TestContext*> removeImports;
        for(int i = 0; i < removeCount; ++i)
          if(allContexts[a]->imports.count())
            removeImports.insert(allContexts[a]->imports[rand() % allContexts[a]->imports.count()]);
        allContexts[a]->unImport(removeImports.toList());

        for(int b = 0; b < contextCount; b++)
          if(rand() % verifyOnceIn == 0)
            allContexts[b]->verify(allContexts);
      }

      for(int a = 0; a < contextCount; a++) {
        if(rand() % clearOnceIn == 0) {
          allContexts[a]->clearImports();
          allContexts[a]->verify(allContexts);
        }
      }
    }

  kDebug() << "after: " << KDevelop::RecursiveImportRepository::repository()->getDataRepository().statistics().print();

  for(int a = 0; a < contextCount; ++a)
    delete allContexts[a];
  allContexts.clear();
  kDebug() << "after cleanup: " << KDevelop::RecursiveImportRepository::repository()->getDataRepository().statistics().print();

  }
  clock_t endClock = clock();
  kDebug() << "total clock cycles needed for import-structure test:" << endClock - startClock;
}

class TestWorker : public QObject
{
  Q_OBJECT
public slots:
  void lockForWrite()
  {
    for(int i = 0; i < 10000; ++i) {
      DUChainWriteLocker lock;
    }
    qDebug() << "FINISHED lockForWrite";
  }
  void lockForRead()
  {
    for(int i = 0; i < 10000; ++i) {
      DUChainReadLocker lock;
    }
    qDebug() << "FINISHED lockForRead";
  }
  void lockForReadWrite()
  {
    for(int i = 0; i < 10000; ++i) {
      {
        DUChainReadLocker lock;
      }
      {
        DUChainWriteLocker lock;
      }
    }
    qDebug() << "FINISHED lockForReadWrite";
  }
  static QSharedPointer<QThread> createWorkerThread(const char* workerSlot)
  {
    QThread* thread = new QThread;
    TestWorker* worker = new TestWorker;
    connect(thread, SIGNAL(started()), worker, workerSlot);
    connect(thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    worker->moveToThread(thread);
    return QSharedPointer<QThread>(thread);
  }
};

class ThreadList : public QVector< QSharedPointer<QThread> >
{
public:
  bool join(int timeout)
  {
    qDebug() << "joining" << size() << "threads" << timeout;
    foreach(const QSharedPointer<QThread>& thread, *this) {
      // quit event loop
      Q_ASSERT(thread->isRunning());
      thread->quit();
      // wait for finish
      if (!thread->wait(timeout)) {
        return false;
      }
      Q_ASSERT(thread->isFinished());
    }
    return true;
  }
  void start()
  {
    foreach(const QSharedPointer<QThread>& thread, *this) {
      thread->start();
    }
  }
};

void TestDUChain::testLockForWrite()
{
  ThreadList threads;
  for(int i = 0; i < 10; ++i) {
    threads << TestWorker::createWorkerThread(SLOT(lockForWrite()));
  }
  threads.start();
  QVERIFY(threads.join(1000));
}

void TestDUChain::testLockForRead()
{
  ThreadList threads;
  for(int i = 0; i < 10; ++i) {
    threads << TestWorker::createWorkerThread(SLOT(lockForRead()));
  }
  threads.start();
  QVERIFY(threads.join(1000));
}

void TestDUChain::testLockForReadWrite()
{
  ThreadList threads;
  for(int i = 0; i < 10; ++i) {
    threads << TestWorker::createWorkerThread(SLOT(lockForReadWrite()));
  }
  threads.start();
  QVERIFY(threads.join(1000));
}

#if 0

///NOTE: the "unit tests" below are not automated, they - so far - require
///      human interpretation which is not useful for a unit test!
///      someone should investigate what the expected output should be
///      and add proper QCOMPARE/QVERIFY checks accordingly

///FIXME: this needs to be rewritten in order to remove dependencies on formerly run unit tests
void TestDUChain::testImportCache()
{
  KDevelop::globalItemRepositoryRegistry().printAllStatistics();

  KDevelop::RecursiveImportRepository::repository()->printStatistics();

  //Analyze the whole existing import-cache
  //This is very expensive, since it involves loading all existing top-contexts
  uint topContextCount = DUChain::self()->newTopContextIndex();

  uint analyzedCount = 0;
  uint totalImportCount = 0;
  uint naiveNodeCount = 0;
  QSet<uint> reachableNodes;

  DUChainReadLocker lock(DUChain::lock());
  for(uint a = 0; a < topContextCount; ++a) {
    if(a % qMax(1u, topContextCount / 100) == 0) {
      kDebug() << "progress:" << (a * 100) / topContextCount;
    }
    TopDUContext* context = DUChain::self()->chainForIndex(a);
    if(context) {
      TopDUContext::IndexedRecursiveImports imports = context->recursiveImportIndices();
      ++analyzedCount;
      totalImportCount += imports.set().count();
      collectReachableNodes(reachableNodes, imports.setIndex());
      naiveNodeCount += collectNaiveNodeCount(imports.setIndex());
    }
  }

  QVERIFY(analyzedCount);
  kDebug() << "average total count of imports:" << totalImportCount / analyzedCount;
  kDebug() << "count of reachable nodes:" << reachableNodes.size();
  kDebug() << "naive node-count:" << naiveNodeCount << "sharing compression factor:" << ((float)reachableNodes.size()) / ((float)naiveNodeCount);
}

#endif

void TestDUChain::benchCodeModel()
{
  const IndexedString file("testFile");

  QVERIFY(!QTypeInfo< KDevelop::CodeModelItem >::isStatic);

  int i = 0;
  QBENCHMARK {
    CodeModel::self().addItem(file, QualifiedIdentifier("testQID" + QString::number(i++)),
                              KDevelop::CodeModelItem::Class);
  }
}

#include "test_duchain.moc"
#include "moc_test_duchain.cpp"
