/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
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

#include "testcontexts.h"

#include "../parsesession.h"
#include "../declarationbuilder.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

QTEST_KDEMAIN(TestContexts, NoGUI);

using namespace KDevelop;

void TestContexts::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestContexts::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestContexts::testFunctionContext()
{
    QFETCH(QString, code);
    QFETCH(RangeInRevision, argCtxRange);
    QFETCH(RangeInRevision, bodyCtxRange);

    const IndexedString file(QString("%1-functionContext.js").arg(qrand()));
    ParseSession session(file, code, 0);
    QVERIFY(session.ast());
    QCOMPARE(session.language(), QmlJS::Language::JavaScript);

    DeclarationBuilder builder(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->type(), DUContext::Global);

    // one context for (), one for {}, and one for the prototype context
    QCOMPARE(top->childContexts().count(), 3);

    DUContext* argCtx = top->childContexts().at(1);     // The prototype context is at position 0
    QCOMPARE(argCtx->type(), DUContext::Function);
    QCOMPARE(argCtx->range(), argCtxRange);

    DUContext* bodyCtx = top->childContexts().at(2);
    QCOMPARE(bodyCtx->type(), DUContext::Other);
    QCOMPARE(bodyCtx->range(), bodyCtxRange);

    QVERIFY(bodyCtx->imports(argCtx));
}

void TestContexts::testFunctionContext_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<RangeInRevision>("argCtxRange");
    QTest::addColumn<RangeInRevision>("bodyCtxRange");
    //                         0         1
    //                         012345678901234567890
    QTest::newRow("empty") << "function foo() {;}"
                           << RangeInRevision(0, 12, 0, 14)
                           << RangeInRevision(0, 15, 0, 18);

    //                        0         1         2         3
    //                        01234567890123456789012345678901234567890
    QTest::newRow("args") << "function foo(arg1, arg2, arg3) {;}"
                           << RangeInRevision(0, 12, 0, 30)
                           << RangeInRevision(0, 31, 0, 34);

    //                           0         1         2
    //                           0123456789012345678901234567890
    QTest::newRow("newline") << "function foo() {;\n}"
                           << RangeInRevision(0, 12, 0, 14)
                           << RangeInRevision(0, 15, 1, 1);
}

void TestContexts::testQMLContext()
{
    const IndexedString file("testQMLContext.qml");
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
    QCOMPARE(session.language(), QmlJS::Language::Qml);

    DeclarationBuilder builder(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->type(), DUContext::Global);

    QCOMPARE(top->childContexts().count(), 1);
    DUContext* mainCtx = top->childContexts().first();
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

#include "testcontexts.moc"
