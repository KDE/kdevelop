/*
 * This file is part of KDevelop
 *
 * Copyright 2011-2013 Milian Wolff <mail@milianw.de>
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
#include <QElapsedTimer>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/codemodel.h>
#include <language/duchain/types/typesystemdata.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/typeregister.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/duchainregister.h>
#include <language/duchain/problem.h>
#include <language/duchain/parsingenvironment.h>

#include <language/codegen/coderepresentation.h>

#include <language/util/setrepository.h>
#include <language/util/basicsetrepository.h>

// #include <typeinfo>
#include <set>
#include <algorithm>
#include <iterator> // needed for std::insert_iterator on windows
#include <QThread>

//Extremely slow
// #define TEST_NORMAL_IMPORTS

QTEST_MAIN(TestDUChain)

using namespace KDevelop;
using namespace Utils;

typedef BasicSetRepository::Index Index;

struct Timer
{
  Timer()
  {
    m_timer.start();
  }
  qint64 elapsed()
  {
    return m_timer.nsecsElapsed();
  }
  QElapsedTimer m_timer;
};

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

//  qDebug() << "Start repository-layout: \n" << rep.dumpDotGraph();

  qint64 repositoryTime = 0; //Time spent on repository-operations
  qint64 genericTime = 0; //Time spend on equivalent operations with generic sets

  qint64 repositoryIntersectionTime = 0; //Time spent on repository-operations
  qint64 genericIntersectionTime = 0; //Time spend on equivalent operations with generic sets
  qint64 qsetIntersectionTime = 0; //Time spend on equivalent operations with generic sets

  qint64 repositoryUnionTime = 0; //Time spent on repository-operations
  qint64 genericUnionTime = 0; //Time spend on equivalent operations with generic sets

  qint64 repositoryDifferenceTime = 0; //Time spent on repository-operations
  qint64 genericDifferenceTime = 0; //Time spend on equivalent operations with generic sets

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

      Timer t;
      chosenIndices.insert(chosenIndices.end(), choose);
      genericTime += t.elapsed();
    }

    {
      Timer t;
      sets[a] = rep.createSet(chosenIndices);
      repositoryTime += t.elapsed();
    }

    realSets[a] = chosenIndices;

    std::set<Index> tempSet = sets[a].stdSet();

    if(tempSet != realSets[a]) {
      QString dbg = "created set: ";
      for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
        dbg += QStringLiteral("%1 ").arg(*it);
      qDebug() << dbg;

      dbg = "repo.   set: ";
      for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
        dbg += QStringLiteral("%1 ").arg(*it);
      qDebug() << dbg;

      qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
      QFAIL("sets are not the same!");
    }
  }

  for(int cycle = 0; cycle < 100; ++cycle) {
      if(cycle % 10 == 0)
         qDebug() << "cycle" << cycle;

    for(unsigned int a = 0; a < setCount; a++) {
      for(unsigned int b = 0; b < setCount; b++) {
        /// ----- SUBTRACTION/DIFFERENCE
        std::set<Index> _realDifference;
        {
          Timer t;
          std::set_difference(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realDifference, _realDifference.begin()));
          genericDifferenceTime += t.elapsed();
        }

        Set _difference;
        {
          Timer t;
          _difference = sets[a] - sets[b];
          repositoryDifferenceTime += t.elapsed();
        }

        if(_difference.stdSet() != _realDifference)
        {
          {
            qDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            qDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _difference.stdSet();

            qDebug() << "SET difference:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realDifference.begin(); it != _realDifference.end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << _difference.dumpDotGraph() << "\n\n";
          }
          QFAIL("difference sets are not the same!");
        }


        /// ------ UNION

        std::set<Index> _realUnion;
        {
          Timer t;
          std::set_union(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realUnion, _realUnion.begin()));
          genericUnionTime += t.elapsed();
        }

        Set _union;
        {
          Timer t;
          _union = sets[a] + sets[b];
          repositoryUnionTime += t.elapsed();
        }

        if(_union.stdSet() != _realUnion)
        {
          {
            qDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            qDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _union.stdSet();

            qDebug() << "SET union:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realUnion.begin(); it != _realUnion.end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << _union.dumpDotGraph() << "\n\n";
          }

          QFAIL("union sets are not the same");
        }

        std::set<Index> _realIntersection;

        /// -------- INTERSECTION
        {
          Timer t;
          std::set_intersection(realSets[a].begin(), realSets[a].end(), realSets[b].begin(), realSets[b].end(), std::insert_iterator<std::set<Index> >(_realIntersection, _realIntersection.begin()));
          genericIntersectionTime += t.elapsed();
        }

        //Just for fun: Test how fast QSet intersections are
        QSet<Index> first, second;
        for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it) {
          first.insert(*it);
        }
        for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it) {
          second.insert(*it);
        }
        {
          Timer t;
          QSet<Index> i = first.intersect(second);
          qsetIntersectionTime += t.elapsed();
        }

        Set _intersection;
        {
          Timer t;
          _intersection = sets[a] & sets[b];
          repositoryIntersectionTime += t.elapsed();
        }

        if(_intersection.stdSet() != _realIntersection)
        {
          {
            qDebug() << "SET a:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[a].begin(); it != realSets[a].end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
          }
          {
            qDebug() << "SET b:";
            QString dbg = "";
            for(std::set<Index>::const_iterator it = realSets[b].begin(); it != realSets[b].end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
          }

          {
            std::set<Index> tempSet = _intersection.stdSet();

            qDebug() << "SET intersection:";
            QString dbg = "real    set: ";
            for(std::set<Index>::const_iterator it = _realIntersection.begin(); it != _realIntersection.end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            dbg = "repo.   set: ";
            for(std::set<Index>::const_iterator it = tempSet.begin(); it != tempSet.end(); ++it)
              dbg += QStringLiteral("%1 ").arg(*it);
            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << _intersection.dumpDotGraph() << "\n\n";
          }
          QFAIL("intersection sets are not the same");
        }
      }
    }

    qDebug() << "cycle " << cycle;
    qDebug() << "ns needed for set-building: repository-set: " << float(repositoryTime)
             << " generic-set: " << float(genericTime);
    qDebug() << "ns needed for intersection: repository-sets: " << float(repositoryIntersectionTime)
             << " generic-set: " << float(genericIntersectionTime) << " QSet: " << float(qsetIntersectionTime);
    qDebug() << "ns needed for union: repository-sets: " << float(repositoryUnionTime)
             << " generic-set: " << float(genericUnionTime);
    qDebug() << "ns needed for difference: repository-sets: " << float(repositoryDifferenceTime)
             << " generic-set: " << float(genericDifferenceTime);
  }
}

void TestDUChain::testSymbolTableValid() {
  DUChainReadLocker lock(DUChain::lock());
  PersistentSymbolTable::self().dump(QTextStream(stdout));
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
    //qDebug() << "checking with" << testString;
    //qDebug() << "checking" << a;
    IndexedString indexed(array.constData(), array.size(), IndexedString::hashString(array.constData(), array.size()));

    QCOMPARE(indexed.str(), testString);
    if(knownIndices.contains(testString)) {
      QCOMPARE(indexed.index(), knownIndices[testString].index());
    } else {
      knownIndices[testString] = indexed;
    }

    if(a % (testCount/10) == 0)
      qDebug() << a << "of" << testCount;
  }
  qDebug() << a << "successful tests";
}

struct TestContext {
  TestContext() {
    static int number = 0;
    ++number;
    DUChainWriteLocker lock(DUChain::lock());
    m_context = new TopDUContext(IndexedString(QStringLiteral("/test1/%1").arg(number)), RangeInRevision());
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
  Timer total;
  qDebug() << "before: " << KDevelop::RecursiveImportRepository::repository()->getDataRepository().statistics().print();

  ///Maintains a naive import-structure along with a real top-context import structure, and allows comparing both.
  int cycles = 5;
  //int cycles = 100;
  //srand(time(NULL));
  for(int t = 0; t < cycles; ++t) {
    QList<TestContext*> allContexts;
    //Create a random structure
    int contextCount = 50;
    int verifyOnceIn = contextCount/*((contextCount*contextCount)/20)+1*/; //Verify once in every chances(not in all cases, because else the import-structure isn't built on-demand!)
    int clearOnceIn = contextCount;
    for(int a = 0; a < contextCount; a++)
      allContexts << new TestContext();
    for(int c = 0; c < cycles; ++c) {
      //qDebug() << "main-cycle" << t  << "sub-cycle" << c;
      //Add random imports and compare
      for(int a = 0; a < contextCount; a++) {
        //Import up to 5 random other contexts into each context
        int importCount = rand() % 5;
        //qDebug()   << "cnt> " << importCount;
        for(int i = 0; i < importCount; ++i)
        {
          //int importNr = rand() % contextCount;
          //qDebug() << "nmr > " << importNr;
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

    qDebug() << "after: " << KDevelop::RecursiveImportRepository::repository()->getDataRepository().statistics().print();

    for(int a = 0; a < contextCount; ++a)
      delete allContexts[a];
    allContexts.clear();
    qDebug() << "after cleanup: " << KDevelop::RecursiveImportRepository::repository()->getDataRepository().statistics().print();
  }
  qDebug() << "total ns needed for import-structure test:" << float(total.elapsed());
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
  }
  void lockForRead()
  {
    for(int i = 0; i < 10000; ++i) {
      DUChainReadLocker lock;
    }
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
  }
  static QSharedPointer<QThread> createWorkerThread(const char* workerSlot)
  {
    QThread* thread = new QThread;
    TestWorker* worker = new TestWorker;
    connect(thread, SIGNAL(started()), worker, workerSlot);
    connect(thread, &QThread::finished, worker, &TestWorker::deleteLater);
    worker->moveToThread(thread);
    return QSharedPointer<QThread>(thread);
  }
};

class ThreadList : public QVector< QSharedPointer<QThread> >
{
public:
  bool join(int timeout)
  {
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
  QBENCHMARK {
    {
      DUChainWriteLocker lock;
    }
    {
      DUChainReadLocker lock;
    }
  }
  QVERIFY(threads.join(1000));
}

void TestDUChain::testLockForRead()
{
  ThreadList threads;
  for(int i = 0; i < 10; ++i) {
    threads << TestWorker::createWorkerThread(SLOT(lockForRead()));
  }
  threads.start();
  QBENCHMARK {
    DUChainReadLocker lock;
  }
  QVERIFY(threads.join(1000));
}

void TestDUChain::testLockForReadWrite()
{
  ThreadList threads;
  for(int i = 0; i < 10; ++i) {
    threads << TestWorker::createWorkerThread(SLOT(lockForReadWrite()));
  }
  threads.start();
  QBENCHMARK {
    DUChainWriteLocker lock;
  }
  QVERIFY(threads.join(1000));
}

void TestDUChain::testProblemSerialization()
{
  DUChain::self()->disablePersistentStorage(false);

  auto parent = ProblemPointer{new Problem};
  parent->setDescription("parent");

  auto child = ProblemPointer{new Problem};
  child->setDescription("child");
  parent->addDiagnostic(child);

  const IndexedString url("/my/test/file");

  TopDUContextPointer smartTop;

  { // serialize
    DUChainWriteLocker lock;
    auto file = new ParsingEnvironmentFile(url);
    auto top = new TopDUContext(url, {}, file);

    top->addProblem(parent);
    QCOMPARE(top->problems().size(), 1);
    auto p = top->problems().first();
    QCOMPARE(p->description(), QStringLiteral("parent"));
    QCOMPARE(p->diagnostics().size(), 1);
    auto c = p->diagnostics().first();
    QCOMPARE(c->description(), QStringLiteral("child"));

    DUChain::self()->addDocumentChain(top);
    QVERIFY(DUChain::self()->chainForDocument(url));
    smartTop = top;
  }

  DUChain::self()->storeToDisk();

  ProblemPointer parent_deserialized;
  ProblemPointer child_deserialized;

  { // deserialize
    DUChainWriteLocker lock;
    QVERIFY(!smartTop);
    auto top = DUChain::self()->chainForDocument(url);
    QVERIFY(top);
    smartTop = top;
    QCOMPARE(top->problems().size(), 1);
    parent_deserialized = top->problems().first();
    QCOMPARE(parent_deserialized->diagnostics().size(), 1);
    child_deserialized = parent_deserialized->diagnostics().first();

    QCOMPARE(parent_deserialized->description(), QStringLiteral("parent"));
    QCOMPARE(child_deserialized->description(), QStringLiteral("child"));

    top->clearProblems();
    QVERIFY(top->problems().isEmpty());

    QCOMPARE(parent_deserialized->description(), QStringLiteral("parent"));
    QCOMPARE(child_deserialized->description(), QStringLiteral("child"));

    DUChain::self()->removeDocumentChain(top);

    QCOMPARE(parent_deserialized->description(), QStringLiteral("parent"));
    QCOMPARE(child_deserialized->description(), QStringLiteral("child"));

    QVERIFY(!smartTop);
  }

  DUChain::self()->disablePersistentStorage(true);

  QCOMPARE(parent->description(), QStringLiteral("parent"));
  QCOMPARE(child->description(), QStringLiteral("child"));
  QCOMPARE(parent_deserialized->description(), QStringLiteral("parent"));
  QCOMPARE(child_deserialized->description(), QStringLiteral("child"));

  parent->clearDiagnostics();
  QVERIFY(parent->diagnostics().isEmpty());
}

void TestDUChain::testIdentifiers()
{
  QualifiedIdentifier aj("::Area::jump");
  QCOMPARE(aj.count(), 2);
  QCOMPARE(aj.explicitlyGlobal(), true);
  QCOMPARE(aj.at(0), Identifier("Area"));
  QCOMPARE(aj.at(1), Identifier("jump"));

  QualifiedIdentifier aj2 = QualifiedIdentifier("Area::jump");
  QCOMPARE(aj2.count(), 2);
  QCOMPARE(aj2.explicitlyGlobal(), false);
  QCOMPARE(aj2.at(0), Identifier("Area"));
  QCOMPARE(aj2.at(1), Identifier("jump"));
  QVERIFY(aj != aj2);

  QVERIFY(QualifiedIdentifier("") == QualifiedIdentifier());
  QVERIFY(QualifiedIdentifier("").index() == QualifiedIdentifier().index());

  QualifiedIdentifier ajt("Area::jump::test");
  QualifiedIdentifier jt("jump::test");
  QualifiedIdentifier ajt2("Area::jump::tes");

  QualifiedIdentifier t(" Area<A,B>::jump <F> ::tes<C>");
  QCOMPARE(t.count(), 3);
  QCOMPARE(t.at(0).templateIdentifiersCount(), 2u);
  QCOMPARE(t.at(1).templateIdentifiersCount(), 1u);
  QCOMPARE(t.at(2).templateIdentifiersCount(), 1u);
  QCOMPARE(t.at(0).identifier().str(), QStringLiteral("Area"));
  QCOMPARE(t.at(1).identifier().str(), QStringLiteral("jump"));
  QCOMPARE(t.at(2).identifier().str(), QStringLiteral("tes"));

  QualifiedIdentifier op1("operator<");
  QualifiedIdentifier op2("operator<=");
  QualifiedIdentifier op3("operator>");
  QualifiedIdentifier op4("operator>=");
  QualifiedIdentifier op5("operator()");
  QualifiedIdentifier op6("operator( )");
  QCOMPARE(op1.count(), 1);
  QCOMPARE(op2.count(), 1);
  QCOMPARE(op3.count(), 1);
  QCOMPARE(op4.count(), 1);
  QCOMPARE(op5.count(), 1);
  QCOMPARE(op6.count(), 1);
  QCOMPARE(op4.toString(), QStringLiteral("operator>="));
  QCOMPARE(op3.toString(), QStringLiteral("operator>"));
  QCOMPARE(op1.toString(), QStringLiteral("operator<"));
  QCOMPARE(op2.toString(), QStringLiteral("operator<="));
  QCOMPARE(op5.toString(), QStringLiteral("operator()"));
  QCOMPARE(op6.toString(), QStringLiteral("operator( )"));
  QCOMPARE(QualifiedIdentifier("Area<A,B>::jump <F> ::tes<C>").index(), t.index());
  QCOMPARE(op4.index(), QualifiedIdentifier("operator>=").index());

  QualifiedIdentifier pushTest("foo");
  QCOMPARE(pushTest.count(), 1);
  QCOMPARE(pushTest.toString(), QStringLiteral("foo"));
  pushTest.push(Identifier("bar"));
  QCOMPARE(pushTest.count(), 2);
  QCOMPARE(pushTest.toString(), QStringLiteral("foo::bar"));
  pushTest.push(QualifiedIdentifier("baz::asdf"));
  QCOMPARE(pushTest.count(), 4);
  QCOMPARE(pushTest.toString(), QStringLiteral("foo::bar::baz::asdf"));
  QualifiedIdentifier mergeTest = pushTest.merge(QualifiedIdentifier("meh::muh"));
  QCOMPARE(mergeTest.count(), 6);
  QCOMPARE(mergeTest.toString(), QStringLiteral("meh::muh::foo::bar::baz::asdf"));
  QualifiedIdentifier plusTest = QualifiedIdentifier("la::lu") + QualifiedIdentifier("ba::bu");
  QCOMPARE(plusTest.count(), 4);
  QCOMPARE(plusTest.toString(), QStringLiteral("la::lu::ba::bu"));
  ///@todo create a big randomized test for the identifier repository(check that indices are the same)
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
      qDebug() << "progress:" << (a * 100) / topContextCount;
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
  qDebug() << "average total count of imports:" << totalImportCount / analyzedCount;
  qDebug() << "count of reachable nodes:" << reachableNodes.size();
  qDebug() << "naive node-count:" << naiveNodeCount << "sharing compression factor:" << ((float)reachableNodes.size()) / ((float)naiveNodeCount);
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

void TestDUChain::benchTypeRegistry()
{
  IntegralTypeData data;
  data.m_dataType = IntegralType::TypeInt;
  data.typeClassId = IntegralType::Identity;
  data.inRepository = false;
  data.m_modifiers = 42;
  data.m_dynamic = false;
  data.refCount = 1;

  IntegralTypeData to;

  QFETCH(int, func);

  QBENCHMARK {
    switch(func) {
      case 0:
        TypeSystem::self().dataClassSize(data);
        break;
      case 1:
        TypeSystem::self().dynamicSize(data);
        break;
      case 2:
        TypeSystem::self().create(&data);
        break;
      case 3:
        TypeSystem::self().isFactoryLoaded(data);
        break;
      case 4:
        TypeSystem::self().copy(data, to, !data.m_dynamic);
        break;
      case 5:
        TypeSystem::self().copy(data, to, data.m_dynamic);
        break;
      case 6:
        TypeSystem::self().callDestructor(&data);
        break;
    }
  }
}

void TestDUChain::benchTypeRegistry_data()
{
  QTest::addColumn<int>("func");
  QTest::newRow("dataClassSize") << 0;
  QTest::newRow("dynamicSize") << 1;
  QTest::newRow("create") << 2;
  QTest::newRow("isFactoryLoaded") << 3;
  QTest::newRow("copy") << 4;
  QTest::newRow("copyNonDynamic") << 5;
  QTest::newRow("callDestructor") << 6;
}

void TestDUChain::benchDuchainReadLocker()
{
  QBENCHMARK {
    DUChainReadLocker lock;
  }
}

void TestDUChain::benchDuchainWriteLocker()
{
  QBENCHMARK {
    DUChainWriteLocker lock;
  }
}

void TestDUChain::benchDUChainItemFactory_copy()
{
  DUChainItemFactory<Declaration, DeclarationData> factory;
  DeclarationData from, to;
  from.classId = Declaration::Identity;

  QFETCH(int, constant);

  bool c = constant;

  QBENCHMARK {
    factory.copy(from, to, c);
    if (constant == 2) {
      c = !c;
    }
  }
}

void TestDUChain::benchDUChainItemFactory_copy_data()
{
  QTest::addColumn<int>("constant");
  QTest::newRow("non-const") << 0;
  QTest::newRow("const") << 1;
  QTest::newRow("flip") << 2;
}

#include "test_duchain.moc"
#include "moc_test_duchain.cpp"
