/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
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

#include "test_duchainshutdown.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QtTest>

using namespace KDevelop;

void TestDUChainShutdown::initTestCase()
{
  AutoTestShell::init();
  m_core = TestCore::initialize(Core::NoUi);
}

void TestDUChainShutdown::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestDUChainShutdown::runTest()
{
  const QString ctxId = "foo::bar::asdf";
  const QString path = "/foo/myurl";
  const QString myLang = "fooLang";

  // step 1, store a bunch of data in the repository
  IndexedTopDUContext idxTop;
  IndexedDUContext idxCtx;
  {
    DUChainWriteLocker lock;

    auto file = new ParsingEnvironmentFile(IndexedString(path));
    file->setLanguage(IndexedString(myLang));

    ReferencedTopDUContext top(new TopDUContext(IndexedString(path), RangeInRevision(1, 2, 3, 4), file));
    DUChain::self()->addDocumentChain(top);
    idxTop = IndexedTopDUContext(top);
    QVERIFY(idxTop.isValid());
    QVERIFY(idxTop.isLoaded());

    auto ctx = new DUContext(RangeInRevision(1, 2, 2, 3), top);
    ctx->setLocalScopeIdentifier(QualifiedIdentifier(ctxId));
    QCOMPARE(top->childContexts().size(), 1);

    idxCtx = IndexedDUContext(ctx);
    QVERIFY(idxCtx.isValid());
  }

  // shutdown and reinitialize - this should not crash :)
  m_core->setShuttingDown(true);
  DUChain::self()->shutdown();
  m_core->setShuttingDown(false);
  DUChain::self()->initialize();

  {
    DUChainReadLocker lock;
    // now verify that the data was properly restored
    QVERIFY(!idxTop.isLoaded());
    QVERIFY(idxTop.isValid());

    ReferencedTopDUContext top(idxTop.data());
    QVERIFY(top);
    QVERIFY(idxTop.isLoaded());
    QCOMPARE(top->childContexts().size(), 1);
    QCOMPARE(top->childContexts().first()->localScopeIdentifier().toString(), QString("foo::bar::asdf"));
    QCOMPARE(idxCtx.data(), top->childContexts().first());
  }
  {
    DUChainWriteLocker lock;
    DUChain::self()->removeDocumentChain(idxTop.data());
  }
}

QTEST_MAIN(TestDUChainShutdown)
