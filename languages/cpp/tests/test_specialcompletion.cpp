/*
 * This file is part of KDevelop
 *
 * Copyright 2011 Milian Wolff <mail@milianw.de>
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

#include "test_specialcompletion.h"

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testfile.h>
#include <tests/testproject.h>

#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/codecompletion/codecompletiontesthelper.h>

#include "codecompletion/missingincludeitem.h"
#include "codecompletion/context.h"
#include "codecompletion/helpers.h"

#include <QTest>
#include <qtest_kde.h>
#include <language/duchain/duchainutils.h>

using namespace KDevelop;
using namespace Cpp;

QTEST_KDEMAIN_WITH_COMPONENTNAME(TestSpecialCompletion, GUI, "test_specialcompletion")

typedef CodeCompletionItemTester<Cpp::CodeCompletionContext> CompletionItemTester;

void TestSpecialCompletion::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    TestCore* core = dynamic_cast<TestCore*>(TestCore::self());
    QVERIFY(core);

    DUChain::self()->disablePersistentStorage();
    CodeRepresentation::setDiskChangesForbidden(true);

    m_projects = new TestProjectController(core);
    core->setProjectController(m_projects);
}

void TestSpecialCompletion::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestSpecialCompletion::cleanup()
{
    m_projects->clearProjects();
}

void TestSpecialCompletion::testMissingInclude()
{
    TestProject* project = new TestProject(this);
    m_projects->addProject(project);

    TestFile include("class A {};", "h", project);
    include.parse(TopDUContext::AllDeclarationsAndContexts);

    TestFile workingFile("int main() {}", "cpp", project);
    workingFile.parse(TopDUContext::AllDeclarationsAndContexts);

    QCOMPARE(include.url().toUrl().upUrl(), workingFile.url().toUrl().upUrl());

    QVERIFY(include.waitForParsed());
    QVERIFY(workingFile.waitForParsed());

    DUChainReadLocker lock;

    QVERIFY(include.topContext());
    TopDUContext* includeTop = DUChainUtils::contentContextFromProxyContext(include.topContext().data());
    QVERIFY(includeTop);
    QEXPECT_FAIL("", "include path resolver complains", Continue);
    QVERIFY(includeTop->problems().isEmpty());
    QCOMPARE(includeTop->localDeclarations().size(), 1);
    QCOMPARE(includeTop->childContexts().size(), 1);

    QVERIFY(workingFile.topContext());
    TopDUContext* top = DUChainUtils::contentContextFromProxyContext(workingFile.topContext());
    QVERIFY(top);
    QEXPECT_FAIL("", "include path resolver complains", Continue);
    QVERIFY(top->problems().isEmpty());
    QCOMPARE(top->childContexts().size(), 2);

    CompletionItemTester tester(top->childContexts().last(), "A::");
    QVERIFY(tester.completionContext->isValid());
    QCOMPARE(tester.items.size(), 1);
    MissingIncludeCompletionItem* item = dynamic_cast<MissingIncludeCompletionItem*>(tester.items.first().data());
    QVERIFY(item);

    QCOMPARE(item->lineToInsert(), QString("#include \"" + include.url().toUrl().fileName() + "\""));
}

#include "test_specialcompletion.moc"
