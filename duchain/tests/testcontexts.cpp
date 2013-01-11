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
#include "../contextbuilder.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

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
    const IndexedString file("functionContext.js");
    //                          0         1
    //                          012345678901234567890
    ParseSession session(file, "function foo() {}");
    QVERIFY(session.ast());
    qDebug() << session.language();
    QCOMPARE(session.language(), QmlJS::Document::JavaScriptLanguage);

    ContextBuilder builder;
    builder.setParseSession(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->type(), DUContext::Global);

    // one context for (), one for {}
    QCOMPARE(top->childContexts().count(), 2);

    DUContext* argCtx = top->childContexts().first();
    QCOMPARE(argCtx->type(), DUContext::Function);
    QCOMPARE(argCtx->range(), RangeInRevision(0, 12, 0, 13));

    DUContext* bodyCtx = top->childContexts().last();
    QCOMPARE(bodyCtx->type(), DUContext::Other);
    QCOMPARE(bodyCtx->range(), RangeInRevision(0, 15, 0, 16));

    QVERIFY(bodyCtx->imports(argCtx));
}

#include "testcontexts.moc"
