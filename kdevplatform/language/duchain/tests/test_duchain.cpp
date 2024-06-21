/*
    SPDX-FileCopyrightText: 2011-2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <type_traits>
#include <QThread>

//Extremely slow
// #define TEST_NORMAL_IMPORTS

QTEST_MAIN(TestDUChain)

using namespace KDevelop;
using namespace Utils;

static_assert(std::is_nothrow_move_assignable<TypePtr<AbstractType>>(), "Why would a move assignment operator throw?");
static_assert(std::is_nothrow_move_constructible<TypePtr<AbstractType>>(), "Why would a move constructor throw?");

static_assert(std::is_nothrow_move_assignable<DUChainPointer<DUContext>>(), "Why would a move assignment operator throw?");
static_assert(std::is_nothrow_move_constructible<DUChainPointer<DUContext>>(), "Why would a move constructor throw?");

using Index = BasicSetRepository::Index;

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

#ifndef Q_OS_WIN
void TestDUChain::testStringSets()
{
    const unsigned int setCount = 8;
    const unsigned int choiceCount = 40;
    const unsigned int itemCount = 120;

    QRecursiveMutex mutex;
    BasicSetRepository rep(QStringLiteral("test repository"), &mutex);

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
    for (unsigned int a = 0; a < setCount; a++) {
        std::set<Index> chosenIndices;
        unsigned int thisCount = rand() % choiceCount;
        if (thisCount == 0)
            thisCount = 1;

        for (unsigned int b = 0; b < thisCount; b++) {
            Index choose = (rand() % itemCount) + 1;
            while (chosenIndices.find(choose) != chosenIndices.end()) {
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

        const std::set<Index> tempSet = sets[a].stdSet();

        if (tempSet != realSets[a]) {
            QString dbg = QStringLiteral("created set: ");
            for (unsigned int i : qAsConst(realSets[a]))
                dbg += QStringLiteral("%1 ").arg(i);

            qDebug() << dbg;

            dbg = QStringLiteral("repo.   set: ");
            for (unsigned int i : tempSet)
                dbg += QStringLiteral("%1 ").arg(i);

            qDebug() << dbg;

            qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
            QFAIL("sets are not the same!");
        }
    }

    for (int cycle = 0; cycle < 100; ++cycle) {
        if (cycle % 10 == 0)
            qDebug() << "cycle" << cycle;

        for (unsigned int a = 0; a < setCount; a++) {
            for (unsigned int b = 0; b < setCount; b++) {
                /// ----- SUBTRACTION/DIFFERENCE
                std::set<Index> _realDifference;
                {
                    Timer t;
                    std::set_difference(realSets[a].begin(), realSets[a].end(), realSets[b].begin(),
                                        realSets[b].end(),
                                        std::insert_iterator<std::set<Index>>(_realDifference,
                                                                              _realDifference.begin()));
                    genericDifferenceTime += t.elapsed();
                }

                Set _difference;
                {
                    Timer t;
                    _difference = sets[a] - sets[b];
                    repositoryDifferenceTime += t.elapsed();
                }

                if (_difference.stdSet() != _realDifference) {
                    {
                        qDebug() << "SET a:";
                        QString dbg;
                        for (unsigned int i : qAsConst(realSets[a]))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
                    }
                    {
                        qDebug() << "SET b:";
                        QString dbg;
                        for (unsigned int i : qAsConst(realSets[b]))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
                    }

                    {
                        const std::set<Index> tempSet = _difference.stdSet();

                        qDebug() << "SET difference:";
                        QString dbg = QStringLiteral("real    set: ");
                        for (unsigned int i : qAsConst(_realDifference))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        dbg = QStringLiteral("repo.   set: ");
                        for (unsigned int i : tempSet)
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << _difference.dumpDotGraph() << "\n\n";
                    }
                    QFAIL("difference sets are not the same!");
                }

                /// ------ UNION

                std::set<Index> _realUnion;
                {
                    Timer t;
                    std::set_union(realSets[a].begin(), realSets[a].end(), realSets[b].begin(),
                                   realSets[b].end(),
                                   std::insert_iterator<std::set<Index>>(_realUnion, _realUnion.begin()));
                    genericUnionTime += t.elapsed();
                }

                Set _union;
                {
                    Timer t;
                    _union = sets[a] + sets[b];
                    repositoryUnionTime += t.elapsed();
                }

                if (_union.stdSet() != _realUnion) {
                    {
                        qDebug() << "SET a:";
                        QString dbg;
                        for (unsigned int i : qAsConst(realSets[a]))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
                    }
                    {
                        qDebug() << "SET b:";
                        QString dbg;
                        for (unsigned int i : qAsConst(realSets[b]))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
                    }

                    {
                        const std::set<Index> tempSet = _union.stdSet();

                        qDebug() << "SET union:";
                        QString dbg = QStringLiteral("real    set: ");
                        for (unsigned int i : qAsConst(_realUnion))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        dbg = QStringLiteral("repo.   set: ");
                        for (unsigned int i : tempSet)
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << _union.dumpDotGraph() << "\n\n";
                    }

                    QFAIL("union sets are not the same");
                }

                std::set<Index> _realIntersection;

                /// -------- INTERSECTION
                {
                    Timer t;
                    std::set_intersection(realSets[a].begin(), realSets[a].end(), realSets[b].begin(),
                                          realSets[b].end(),
                                          std::insert_iterator<std::set<Index>>(_realIntersection,
                                                                                _realIntersection.begin()));
                    genericIntersectionTime += t.elapsed();
                }

                //Just for fun: Test how fast QSet intersections are
                QSet<Index> first, second;
                for (unsigned int i : qAsConst(realSets[a])) {
                    first.insert(i);
                }

                for (unsigned int i : qAsConst(realSets[b])) {
                    second.insert(i);
                }

                {
                    Timer t;
                    QSet<Index> i = first.intersect(second); // clazy:exclude=unused-non-trivial-variable
                    qsetIntersectionTime += t.elapsed();
                }

                Set _intersection;
                {
                    Timer t;
                    _intersection = sets[a] & sets[b];
                    repositoryIntersectionTime += t.elapsed();
                }

                if (_intersection.stdSet() != _realIntersection) {
                    {
                        qDebug() << "SET a:";
                        QString dbg;
                        for (unsigned int i : qAsConst(realSets[a]))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << sets[a].dumpDotGraph() << "\n\n";
                    }
                    {
                        qDebug() << "SET b:";
                        QString dbg;
                        for (unsigned int i : qAsConst(realSets[b]))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << sets[b].dumpDotGraph() << "\n\n";
                    }

                    {
                        const std::set<Index> tempSet = _intersection.stdSet();

                        qDebug() << "SET intersection:";
                        QString dbg = QStringLiteral("real    set: ");
                        for (unsigned int i : qAsConst(_realIntersection))
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        dbg = QStringLiteral("repo.   set: ");
                        for (unsigned int i : tempSet)
                            dbg += QStringLiteral("%1 ").arg(i);

                        qDebug() << dbg;

                        qDebug() << "DOT-Graph:\n\n" << _intersection.dumpDotGraph() << "\n\n";
                    }
                    QFAIL("intersection sets are not the same");
                }
            }
        }

        qDebug() << "cycle " << cycle;
        qDebug() << "ns needed for set-building: repository-set: " << float( repositoryTime )
                 << " generic-set: " << float( genericTime );
        qDebug() << "ns needed for intersection: repository-sets: " << float( repositoryIntersectionTime )
                 << " generic-set: " << float( genericIntersectionTime ) << " QSet: " << float( qsetIntersectionTime );
        qDebug() << "ns needed for union: repository-sets: " << float( repositoryUnionTime )
                 << " generic-set: " << float( genericUnionTime );
        qDebug() << "ns needed for difference: repository-sets: " << float( repositoryDifferenceTime )
                 << " generic-set: " << float( genericDifferenceTime );
    }
}
#endif

void TestDUChain::testSymbolTableValid()
{
    DUChainReadLocker lock(DUChain::lock());
    PersistentSymbolTable::self().dump(QTextStream(stdout));
}

void TestDUChain::testIndexedStrings()
{
    int testCount  = 600000;

    QHash<QString, IndexedString> knownIndices;
    int a = 0;
    for (a = 0; a < testCount; ++a) {
        QString testString;
        int length = rand() % 10;
        for (int b = 0; b < length; ++b)
            testString.append(QLatin1Char(rand() % 6 + 'a'));

        QByteArray array = testString.toUtf8();
        //qDebug() << "checking with" << testString;
        //qDebug() << "checking" << a;
        IndexedString indexed(array.constData(), array.size(), IndexedString::hashString(array.constData(),
                                                                                         array.size()));

        QCOMPARE(indexed.str(), testString);
        if (knownIndices.contains(testString)) {
            QCOMPARE(indexed.index(), knownIndices[testString].index());
        } else {
            knownIndices[testString] = indexed;
        }

        if (a % (testCount / 10) == 0)
            qDebug() << a << "of" << testCount;
    }

    qDebug() << a << "successful tests";
}

struct TestContext
{
    TestContext()
    {
        static int number = 0;
        ++number;
        DUChainWriteLocker lock(DUChain::lock());
        m_context = new TopDUContext(IndexedString(QStringLiteral("/test1/%1").arg(number)), RangeInRevision());
        m_normalContext = new DUContext(RangeInRevision(), m_context);
        DUChain::self()->addDocumentChain(m_context);
        Q_ASSERT(IndexedDUContext(m_context).context() == m_context);
    }

    ~TestContext()
    {
        const auto currentImporters = importers;
        for (TestContext* importer : currentImporters) {
            importer->unImport(QList<TestContext*>() << this);
        }

        unImport(imports);

        DUChainWriteLocker lock(DUChain::lock());
        TopDUContextPointer tp(m_context);
        DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(m_context));
        Q_ASSERT(!tp);
    }

    void verify(QList<TestContext*> allContexts)
    {
        {
            DUChainReadLocker lock(DUChain::lock());
            QCOMPARE(m_context->importedParentContexts().count(), imports.count());
        }
        //Compute a closure of all children, and verify that they are imported.
        QSet<TestContext*> collected;
        collectImports(collected);
        collected.remove(this);

        DUChainReadLocker lock(DUChain::lock());
        for (TestContext* context : qAsConst(collected)) {
            QVERIFY(m_context->imports(context->m_context, CursorInRevision::invalid()));
#ifdef TEST_NORMAL_IMPORTS
            QVERIFY(m_normalContext->imports(context->m_normalContext));
#endif
        }

        //Verify that no other contexts are imported

        for (TestContext* context : qAsConst(allContexts)) {
            if (context != this) {
                QVERIFY(collected.contains(context) ||
                        !m_context->imports(context->m_context, CursorInRevision::invalid()));
#ifdef TEST_NORMAL_IMPORTS
                QVERIFY(collected.contains(context) ||
                        !m_normalContext->imports(context->m_normalContext, CursorInRevision::invalid()));
#endif
            }
        }
    }

    void collectImports(QSet<TestContext*>& collected)
    {
        if (collected.contains(this))
            return;
        collected.insert(this);
        for (TestContext* context : qAsConst(imports)) {
            context->collectImports(collected);
        }
    }
    void import(TestContext* ctx)
    {
        if (imports.contains(ctx) || ctx == this)
            return;
        imports << ctx;
        ctx->importers << this;
        DUChainWriteLocker lock(DUChain::lock());
        m_context->addImportedParentContext(ctx->m_context);
#ifdef TEST_NORMAL_IMPORTS
        m_normalContext->addImportedParentContext(ctx->m_normalContext);
#endif
    }

    void unImport(QList<TestContext*> ctxList)
    {
        QList<TopDUContext*> list;
        QList<DUContext*> normalList;

        for (TestContext* ctx : qAsConst(ctxList)) {
            if (!imports.contains(ctx))
                continue;
            list << ctx->m_context;
            normalList << ctx->m_normalContext;

            imports.removeAll(ctx);
            ctx->importers.removeAll(this);
        }

        DUChainWriteLocker lock(DUChain::lock());
        m_context->removeImportedParentContexts(list);

#ifdef TEST_NORMAL_IMPORTS
        for (DUContext* ctx : qAsConst(normalList)) {
            m_normalContext->removeImportedParentContext(ctx);
        }

#endif
    }

    void clearImports()
    {
        {
            DUChainWriteLocker lock(DUChain::lock());

            m_context->clearImportedParentContexts();
            m_normalContext->clearImportedParentContexts();
        }
        const auto currentImports = imports;
        for (TestContext* ctx : currentImports) {
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

void collectReachableNodes(QSet<uint>& reachableNodes, uint currentNode)
{
    if (!currentNode)
        return;
    reachableNodes.insert(currentNode);
    const Utils::SetNodeData* node = KDevelop::RecursiveImportRepository::repository()->nodeFromIndex(currentNode);
    Q_ASSERT(node);
    collectReachableNodes(reachableNodes, node->leftNode());
    collectReachableNodes(reachableNodes, node->rightNode());
}

uint collectNaiveNodeCount(uint currentNode)
{
    if (!currentNode)
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
    qDebug() << "before: " << KDevelop::RecursiveImportRepository::repository()->statistics().print();

    ///Maintains a naive import-structure along with a real top-context import structure, and allows comparing both.
    int cycles = 5;
    //int cycles = 100;
    //srand(time(NULL));
    for (int t = 0; t < cycles; ++t) {
        QList<TestContext*> allContexts;
        //Create a random structure
        int contextCount = 50;
        int verifyOnceIn = contextCount /*((contextCount*contextCount)/20)+1*/; //Verify once in every chances(not in all cases, because else the import-structure isn't built on-demand!)
        int clearOnceIn = contextCount;
        for (int a = 0; a < contextCount; a++)
            allContexts << new TestContext();

        for (int c = 0; c < cycles; ++c) {
            //qDebug() << "main-cycle" << t  << "sub-cycle" << c;
            //Add random imports and compare
            for (int a = 0; a < contextCount; a++) {
                //Import up to 5 random other contexts into each context
                int importCount = rand() % 5;
                //qDebug()   << "cnt> " << importCount;
                for (int i = 0; i < importCount; ++i) {
                    //int importNr = rand() % contextCount;
                    //qDebug() << "nmr > " << importNr;
                    //allContexts[a]->import(allContexts[importNr]);
                    allContexts[a]->import(allContexts[rand() % contextCount]);
                }

                for (int b = 0; b < contextCount; b++)
                    if (rand() % verifyOnceIn == 0)
                        allContexts[b]->verify(allContexts);
            }

            //Remove random imports and compare
            for (int a = 0; a < contextCount; a++) {
                //Import up to 5 random other contexts into each context
                int removeCount = rand() % 3;
                QSet<TestContext*> removeImports;
                for (int i = 0; i < removeCount; ++i)
                    if (!allContexts[a]->imports.isEmpty())
                        removeImports.insert(allContexts[a]->imports[rand() % allContexts[a]->imports.count()]);

                allContexts[a]->unImport(removeImports.values());

                for (int b = 0; b < contextCount; b++)
                    if (rand() % verifyOnceIn == 0)
                        allContexts[b]->verify(allContexts);
            }

            for (int a = 0; a < contextCount; a++) {
                if (rand() % clearOnceIn == 0) {
                    allContexts[a]->clearImports();
                    allContexts[a]->verify(allContexts);
                }
            }
        }

        qDebug() << "after: " <<
            KDevelop::RecursiveImportRepository::repository()->statistics().print();

        for (int a = 0; a < contextCount; ++a)
            delete allContexts[a];

        allContexts.clear();
        qDebug() << "after cleanup: " <<
            KDevelop::RecursiveImportRepository::repository()->statistics().print();
    }

    qDebug() << "total ns needed for import-structure test:" << float( total.elapsed());
}

class TestWorker
    : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void lockForWrite()
    {
        for (int i = 0; i < 10000; ++i) {
            DUChainWriteLocker lock;
        }
    }
    void lockForRead()
    {
        for (int i = 0; i < 10000; ++i) {
            DUChainReadLocker lock;
        }
    }
    void lockForReadWrite()
    {
        for (int i = 0; i < 10000; ++i) {
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
        auto* thread = new QThread;
        auto* worker = new TestWorker;
        connect(thread, SIGNAL(started()), worker, workerSlot);
        connect(thread, &QThread::finished, worker, &TestWorker::deleteLater);
        worker->moveToThread(thread);
        return QSharedPointer<QThread>(thread);
    }
};

class ThreadList
    : public QVector<QSharedPointer<QThread>>
{
public:
    bool join(int timeout)
    {
        for (const QSharedPointer<QThread>& thread : qAsConst(*this)) {
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
        for (const QSharedPointer<QThread>& thread : qAsConst(*this)) {
            thread->start();
        }
    }
};

void TestDUChain::testLockForWrite()
{
    ThreadList threads;
    for (int i = 0; i < 10; ++i) {
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
    for (int i = 0; i < 10; ++i) {
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
    for (int i = 0; i < 10; ++i) {
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
    parent->setDescription(QStringLiteral("parent"));

    auto child = ProblemPointer{new Problem};
    child->setDescription(QStringLiteral("child"));
    parent->addDiagnostic(child);

    const IndexedString url("/my/test/file");

    TopDUContextPointer smartTop;

    { // serialize
        DUChainWriteLocker lock;
        auto file = new ParsingEnvironmentFile(url);
        auto top = new TopDUContext(url, {}, file);

        top->addProblem(parent);
        QCOMPARE(top->problems().size(), 1);
        auto p = top->problems().at(0);
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
    IProblem::Ptr child_deserialized;

    { // deserialize
        DUChainWriteLocker lock;
        QVERIFY(!smartTop);
        auto top = DUChain::self()->chainForDocument(url);
        QVERIFY(top);
        smartTop = top;
        QCOMPARE(top->problems().size(), 1);
        parent_deserialized = top->problems().at(0);
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
    QualifiedIdentifier aj(QStringLiteral("::Area::jump"));
    QCOMPARE(aj.count(), 2);
    QCOMPARE(aj.explicitlyGlobal(), true);
    QCOMPARE(aj.at(0), Identifier(QStringLiteral("Area")));
    QCOMPARE(aj.at(1), Identifier(QStringLiteral("jump")));

    QualifiedIdentifier aj2 = QualifiedIdentifier(QStringLiteral("Area::jump"));
    QCOMPARE(aj2.count(), 2);
    QCOMPARE(aj2.explicitlyGlobal(), false);
    QCOMPARE(aj2.at(0), Identifier(QStringLiteral("Area")));
    QCOMPARE(aj2.at(1), Identifier(QStringLiteral("jump")));
    QVERIFY(aj != aj2);

    QVERIFY(QualifiedIdentifier(QString()) == QualifiedIdentifier());
    QVERIFY(QualifiedIdentifier(QString()).index() == QualifiedIdentifier().index());

    QualifiedIdentifier ajt(QStringLiteral("Area::jump::test"));
    QualifiedIdentifier jt(QStringLiteral("jump::test"));
    QualifiedIdentifier ajt2(QStringLiteral("Area::jump::tes"));

    QualifiedIdentifier t(QStringLiteral(" Area<A,B>::jump <F> ::tes<C>"));
    QCOMPARE(t.count(), 3);
    QCOMPARE(t.at(0).templateIdentifiersCount(), 2u);
    QCOMPARE(t.at(1).templateIdentifiersCount(), 1u);
    QCOMPARE(t.at(2).templateIdentifiersCount(), 1u);
    QCOMPARE(t.at(0).identifier().str(), QStringLiteral("Area"));
    QCOMPARE(t.at(1).identifier().str(), QStringLiteral("jump"));
    QCOMPARE(t.at(2).identifier().str(), QStringLiteral("tes"));

    QualifiedIdentifier op1(QStringLiteral("operator<"));
    QualifiedIdentifier op2(QStringLiteral("operator<="));
    QualifiedIdentifier op3(QStringLiteral("operator>"));
    QualifiedIdentifier op4(QStringLiteral("operator>="));
    QualifiedIdentifier op5(QStringLiteral("operator()"));
    QualifiedIdentifier op6(QStringLiteral("operator( )"));
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
    QCOMPARE(QualifiedIdentifier(QStringLiteral("Area<A,B>::jump <F> ::tes<C>")).index(), t.index());
    QCOMPARE(op4.index(), QualifiedIdentifier(QStringLiteral("operator>=")).index());

    QualifiedIdentifier pushTest(QStringLiteral("foo"));
    QCOMPARE(pushTest.count(), 1);
    QCOMPARE(pushTest.toString(), QStringLiteral("foo"));
    pushTest.push(Identifier(QStringLiteral("bar")));
    QCOMPARE(pushTest.count(), 2);
    QCOMPARE(pushTest.toString(), QStringLiteral("foo::bar"));
    pushTest.push(QualifiedIdentifier(QStringLiteral("baz::asdf")));
    QCOMPARE(pushTest.count(), 4);
    QCOMPARE(pushTest.toString(), QStringLiteral("foo::bar::baz::asdf"));
    QualifiedIdentifier mergeTest = pushTest.merge(QualifiedIdentifier(QStringLiteral("meh::muh")));
    QCOMPARE(mergeTest.count(), 6);
    QCOMPARE(mergeTest.toString(), QStringLiteral("meh::muh::foo::bar::baz::asdf"));
    QualifiedIdentifier plusTest = QualifiedIdentifier(QStringLiteral("la::lu")) +
                                   QualifiedIdentifier(QStringLiteral("ba::bu"));
    QCOMPARE(plusTest.count(), 4);
    QCOMPARE(plusTest.toString(), QStringLiteral("la::lu::ba::bu"));
    ///@todo create a big randomized test for the identifier repository(check that indices are the same)
}

void TestDUChain::testTypePtr()
{
    AbstractType::Ptr abstractT;
    QVERIFY(!abstractT);

    IntegralType::Ptr integralT(new IntegralType(IntegralType::TypeDouble));

    abstractT = integralT;
    QCOMPARE(abstractT.get(), integralT.get());

    DelayedType::Ptr delayedT(new DelayedType);
    QVERIFY(abstractT.get() != delayedT.get());
    QVERIFY(!abstractT->equals(delayedT.get()));

    auto abstractT2 = abstractT;
    QCOMPARE(abstractT2.get(), integralT.get());

    auto abstractT3 = AbstractType::Ptr(integralT);
    QCOMPARE(abstractT2.get(), integralT.get());
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
    for (uint a = 0; a < topContextCount; ++a) {
        if (a % qMax(1u, topContextCount / 100) == 0) {
            qDebug() << "progress:" << (a * 100) / topContextCount;
        }
        TopDUContext* context = DUChain::self()->chainForIndex(a);
        if (context) {
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
    qDebug() << "naive node-count:" << naiveNodeCount << "sharing compression factor:" <<
    (( float )reachableNodes.size()) / (( float )naiveNodeCount);
}

#endif

void TestDUChain::benchCodeModel()
{
    const IndexedString file("testFile");

    QVERIFY(QTypeInfo<KDevelop::CodeModelItem>::isRelocatable);

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
    data.inRepository = true;
    data.m_modifiers = 42;
    data.m_dynamic = false;
    data.refCount = 1;

    IntegralTypeData to;

    QFETCH(int, func);

    QBENCHMARK {
        switch (func) {
        case 0:
            TypeSystem::self().dataClassSize(data);
            break;
        case 1:
            TypeSystem::self().dynamicSize(data);
            break;
        case 2: {
            AbstractType::Ptr t(TypeSystem::self().create(&data));
            break;
        }
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

void TestDUChain::benchDeclarationQualifiedIdentifier()
{
    QVector<DUContext*> contexts;
    contexts.reserve(10);
    DUChainWriteLocker lock;
    auto topDUContext = new TopDUContext(IndexedString("/tmp/something"), {0, 0, INT_MAX, INT_MAX});
    DUChain::self()->addDocumentChain(topDUContext);
    contexts << topDUContext;
    for (int i = 1; i < contexts.capacity(); ++i) {
        contexts << new DUContext({0, 0, INT_MAX, INT_MAX}, contexts.at(i - 1));
        contexts.last()->setLocalScopeIdentifier(QualifiedIdentifier(QString::number(i)));
    }

    auto dec = new Declaration({0, 0, 0, 1}, contexts.last());
    dec->setIdentifier(Identifier(QStringLiteral("myDecl")));

    qDebug() << "start benchmark!";
    qint64 count = 0;
    QBENCHMARK {
        count += dec->qualifiedIdentifier().count();
    }
    QVERIFY(count > 0);

    // manually delete as QScopedPointer does not work well with QBENCHMARK
    delete dec;
    DUChain::self()->removeDocumentChain(topDUContext);
}

#include "test_duchain.moc"
#include "moc_test_duchain.cpp"
