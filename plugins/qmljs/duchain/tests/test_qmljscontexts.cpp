/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_qmljscontexts.h"

#include "../helper.h"
#include "../parsesession.h"
#include "../declarationbuilder.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

#include <QTest>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif

QTEST_GUILESS_MAIN(TestContexts)

using namespace KDevelop;

void TestContexts::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    QmlJS::registerDUChainItems();
}

void TestContexts::cleanupTestCase()
{
    QmlJS::unregisterDUChainItems();

    TestCore::shutdown();
}

void TestContexts::testFunctionContext()
{
    QFETCH(QString, code);
    QFETCH(RangeInRevision, argCtxRange);
    QFETCH(RangeInRevision, bodyCtxRange);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const auto random = QRandomGenerator::global()->generate();
#else
    const auto random = qrand();
#endif
    const IndexedString file(QUrl(QStringLiteral("file:///internal/%1-functionContext.js").arg(random)));
    ParseSession session(file, code, 0);
    QVERIFY(session.ast());
    QCOMPARE(session.language().dialect(), QmlJS::Dialect::JavaScript);

    DeclarationBuilder builder(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->type(), DUContext::Global);

    // the function arguments (containing the prototype context and the function body)
    QCOMPARE(top->childContexts().count(), 3);        // module, exports, the function

    DUContext* argCtx = top->childContexts().at(2);
    QCOMPARE(argCtx->type(), DUContext::Function);
    QCOMPARE(argCtx->range(), argCtxRange);
    QCOMPARE(argCtx->childContexts().size(), 2);    // The prototype context then the body context

    DUContext* bodyCtx = argCtx->childContexts().at(1);
    QVERIFY(bodyCtx);
    QCOMPARE(bodyCtx->type(), DUContext::Other);
    QCOMPARE(bodyCtx->range(), bodyCtxRange);
}

void TestContexts::testFunctionContext_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<RangeInRevision>("argCtxRange");
    QTest::addColumn<RangeInRevision>("bodyCtxRange");
    //                         0         1
    //                         012345678901234567890
    QTest::newRow("empty") << "function foo() {;}"
                           << RangeInRevision(0, 12, 0, 18)
                           << RangeInRevision(0, 15, 0, 18);

    //                        0         1         2         3
    //                        01234567890123456789012345678901234567890
    QTest::newRow("args") << "function foo(arg1, arg2, arg3) {;}"
                           << RangeInRevision(0, 12, 0, 34)
                           << RangeInRevision(0, 31, 0, 34);

    //                           0         1         2
    //                           0123456789012345678901234567890
    QTest::newRow("newline") << "function foo() {;\n}"
                           << RangeInRevision(0, 12, 1, 1)
                           << RangeInRevision(0, 15, 1, 1);
}

void TestContexts::testQMLContext()
{
    const IndexedString file(QUrl(QStringLiteral("file:///internal/testQMLContext.qml")));
    ParseSession session(file, "Text {\n"
                               "  id: main\n"
                               "  Text {\n"
                               "    id: child1\n"
                               "  }\n"
                               "  Text {\n"
                               "    id: child2\n"
                               "  }\n"
                               "}\n", 0);
    QVERIFY(session.ast());
    QCOMPARE(session.language().dialect(), QmlJS::Dialect::Qml);

    DeclarationBuilder builder(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->type(), DUContext::Global);

    QCOMPARE(top->childContexts().count(), 3);        // module, exports, Text
    DUContext* mainCtx = top->childContexts().at(2);
    QCOMPARE(mainCtx->type(), DUContext::Class);
    QCOMPARE(mainCtx->range(), RangeInRevision(0, 6, 8, 0));
    QCOMPARE(mainCtx->childContexts().size(), 2);

    DUContext* child1Ctx = mainCtx->childContexts().first();
    QCOMPARE(child1Ctx->type(), DUContext::Class);
    QCOMPARE(child1Ctx->range(), RangeInRevision(2, 8, 4, 2));
    QCOMPARE(child1Ctx->childContexts().size(), 0);

    DUContext* child2Ctx = mainCtx->childContexts().last();
    QCOMPARE(child2Ctx->type(), DUContext::Class);
    QCOMPARE(child2Ctx->range(), RangeInRevision(5, 8, 7, 2));
    QCOMPARE(child2Ctx->childContexts().size(), 0);
}
