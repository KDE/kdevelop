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
#include <language/codegen/coderepresentation.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/problem.h>

#include "codecompletion/missingincludeitem.h"
#include "codecompletion/context.h"
#include "codecompletion/helpers.h"

#include <QTest>
#include <qtest_kde.h>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>
#include <KTempDir>

using namespace KDevelop;
using namespace Cpp;

QTEST_KDEMAIN_WITH_COMPONENTNAME(TestSpecialCompletion, GUI, "test_specialcompletion")

typedef CodeCompletionItemTester<Cpp::CodeCompletionContext> CompletionItemTester;

void TestSpecialCompletion::initTestCase()
{
    AutoTestShell::init(QStringList() << "kdevcppsupport");
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
    TestProject* project = new TestProject;
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
    QVERIFY(includeTop->problems().isEmpty());
    QCOMPARE(includeTop->localDeclarations().size(), 1);
    QCOMPARE(includeTop->childContexts().size(), 1);

    QVERIFY(workingFile.topContext());
    TopDUContext* top = DUChainUtils::contentContextFromProxyContext(workingFile.topContext());
    QVERIFY(top);
    QVERIFY(top->problems().isEmpty());
    QCOMPARE(top->childContexts().size(), 2);

    CompletionItemTester tester(top->childContexts().last(), "A::");
    QVERIFY(tester.completionContext->isValid());
    QCOMPARE(tester.items.size(), 1);
    MissingIncludeCompletionItem* item = dynamic_cast<MissingIncludeCompletionItem*>(tester.items.first().data());
    QVERIFY(item);

    QCOMPARE(item->lineToInsert(), QString("#include \"" + include.url().toUrl().fileName() + "\""));
}

void TestSpecialCompletion::testIncludeDefine()
{
    TestProject* project = new TestProject;
    m_projects->addProject(project);

    TestFile include("class A {};", "h", project);
    include.parse(TopDUContext::AllDeclarationsAndContexts);

    TestFile active("#if 0\n"
                    "#include \"asdf.h\"\n"
                    "#endif\n"
                    "int main() {}",
                    "cpp", project);
    active.parse(TopDUContext::AllDeclarationsAndContexts);

    QCOMPARE(include.url().toUrl().upUrl(), active.url().toUrl().upUrl());

    QVERIFY(include.waitForParsed());
    QVERIFY(active.waitForParsed());

    DUChainReadLocker lock;

    QVERIFY(include.topContext());
    TopDUContext* includeTop = DUChainUtils::contentContextFromProxyContext(include.topContext().data());
    QVERIFY(includeTop);
    QVERIFY(includeTop->problems().isEmpty());
    QCOMPARE(includeTop->localDeclarations().size(), 1);
    QCOMPARE(includeTop->childContexts().size(), 1);

    QVERIFY(active.topContext());
    TopDUContext* top = DUChainUtils::contentContextFromProxyContext(active.topContext());
    QVERIFY(top);

    CompletionItemTester tester(top->childContexts().last(), "A::");
    QVERIFY(tester.completionContext->isValid());
    QCOMPARE(tester.items.size(), 1);
    MissingIncludeCompletionItem* item = dynamic_cast<MissingIncludeCompletionItem*>(tester.items.first().data());
    QVERIFY(item);

    QCOMPARE(item->lineToInsert(), QString("#include \"" + include.url().toUrl().fileName() + "\""));

    KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
    QVERIFY(editor);

    KTextEditor::Document* doc = editor->createDocument(this);
    QVERIFY(doc);
    QVERIFY(doc->openUrl(active.url().toUrl()));

    item->execute(doc, KTextEditor::Range(3, 12, 3, 12));

    QCOMPARE(doc->text(), QString(
                    "#if 0\n"
                    "#include \"asdf.h\"\n"
                    "#endif\n"
                    "#include \"" + include.url().toUrl().fileName() + "\"\n"
                    "int main() {}"));
}

void TestSpecialCompletion::testIncludeGrouping()
{
    TestProject* project = new TestProject;
    m_projects->addProject(project);

    KTempDir dir1;
    QVERIFY(dir1.exists());
    const QString dir1Name = QFileInfo(dir1.name()).dir().dirName() + "/";
    KTempDir dir2;
    QVERIFY(dir2.exists());
    const QString dir2Name = QFileInfo(dir2.name()).dir().dirName() + "/";

    TestFile includeA("class A {};", "h", project, dir1Name);
    includeA.parse(TopDUContext::AllDeclarationsAndContexts);
    TestFile includeB("class B {};", "h", project, dir2Name);
    includeB.parse(TopDUContext::AllDeclarationsAndContexts);
    TestFile includeD("class D {};", "h", project, dir2Name);
    includeD.parse(TopDUContext::AllDeclarationsAndContexts);
    TestFile includeC("class C {};", "h", project, dir1Name);
    includeC.parse(TopDUContext::AllDeclarationsAndContexts);

    QVERIFY(QFile::exists(dir1.name() + includeA.url().toUrl().fileName()));
    QVERIFY(QFile::exists(dir2.name() + includeB.url().toUrl().fileName()));
    QVERIFY(QFile::exists(dir2.name() + includeD.url().toUrl().fileName()));

    TestFile active("#include \"" + dir1Name + includeA.url().toUrl().fileName() + "\"\n"
                    "#include \"" + dir2Name + includeB.url().toUrl().fileName() + "\"\n"
                    "#include \"" + dir2Name + includeD.url().toUrl().fileName() + "\"\n"
                    "\n"
                    "int main() {\n"
                    "\n"
                    "}",
                    "cpp", project);
    active.parse(TopDUContext::AllDeclarationsAndContexts);

    QCOMPARE(includeA.url().toUrl().upUrl().upUrl(), active.url().toUrl().upUrl());
    QCOMPARE(includeB.url().toUrl().upUrl().upUrl(), active.url().toUrl().upUrl());
    QCOMPARE(includeC.url().toUrl().upUrl().upUrl(), active.url().toUrl().upUrl());
    QCOMPARE(includeD.url().toUrl().upUrl().upUrl(), active.url().toUrl().upUrl());

    QCOMPARE(includeA.url().toUrl().upUrl(), includeC.url().toUrl().upUrl());
    QCOMPARE(includeB.url().toUrl().upUrl(), includeD.url().toUrl().upUrl());
    QVERIFY(includeC.url().toUrl().upUrl() != includeB.url().toUrl().upUrl());

    QVERIFY(includeA.waitForParsed());
    QVERIFY(includeB.waitForParsed());
    QVERIFY(includeC.waitForParsed());
    QVERIFY(includeD.waitForParsed());
    QVERIFY(active.waitForParsed());

    DUChainReadLocker lock;

    QVERIFY(active.topContext());
    QVERIFY(active.topContext()->problems().isEmpty());
    TopDUContext* top = DUChainUtils::contentContextFromProxyContext(active.topContext());
    QVERIFY(top);
    QCOMPARE(top->importedParentContexts().size(), 3);

    CompletionItemTester tester(top->childContexts().last(), "C::");
    QVERIFY(tester.completionContext->isValid());
    QCOMPARE(tester.items.size(), 1);
    MissingIncludeCompletionItem* item = dynamic_cast<MissingIncludeCompletionItem*>(tester.items.first().data());
    QVERIFY(item);

    QCOMPARE(item->lineToInsert(), QString("#include \"" + dir1Name + includeC.url().toUrl().fileName() + "\""));

    KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
    QVERIFY(editor);

    KTextEditor::Document* doc = editor->createDocument(this);
    QVERIFY(doc);
    QVERIFY(doc->openUrl(active.url().toUrl()));

    item->execute(doc, KTextEditor::Range(3, 0, 3, 3));

    QCOMPARE(doc->text(), QString(
                    "#include \"" + dir1Name + includeA.url().toUrl().fileName() + "\"\n"
                    "#include \"" + dir1Name + includeC.url().toUrl().fileName() + "\"\n"
                    "#include \"" + dir2Name + includeB.url().toUrl().fileName() + "\"\n"
                    "#include \"" + dir2Name + includeD.url().toUrl().fileName() + "\"\n"
                    "\n"
                    "int main() {\n\n}"));
}

void TestSpecialCompletion::testIncludeComment()
{
    TestProject* project = new TestProject;
    m_projects->addProject(project);

    TestFile include("class A {};", "h", project);
    include.parse(TopDUContext::AllDeclarationsAndContexts);

    TestFile active("/*\n"
                    "#include \"asdf.h\"\n"
                    "*/\n"
                    "/**\n"
                    "#include \"asdf.h\"\n"
                    "*/\n"
                    "// #include \"asdf.h\"\n"
                    "int main()\n"
                    "{\n"
                    "\n"
                    "}",
                    "cpp", project);
    active.parse(TopDUContext::AllDeclarationsAndContexts);

    QCOMPARE(include.url().toUrl().upUrl(), active.url().toUrl().upUrl());

    QVERIFY(include.waitForParsed());
    QVERIFY(active.waitForParsed());

    DUChainReadLocker lock;

    QVERIFY(include.topContext());
    TopDUContext* includeTop = DUChainUtils::contentContextFromProxyContext(include.topContext().data());
    QVERIFY(includeTop);
    QVERIFY(includeTop->problems().isEmpty());
    QCOMPARE(includeTop->localDeclarations().size(), 1);
    QCOMPARE(includeTop->childContexts().size(), 1);

    QVERIFY(active.topContext());
    TopDUContext* top = DUChainUtils::contentContextFromProxyContext(active.topContext());
    QVERIFY(top);

    CompletionItemTester tester(top->childContexts().last(), "A::");
    QVERIFY(tester.completionContext->isValid());
    QCOMPARE(tester.items.size(), 1);
    MissingIncludeCompletionItem* item = dynamic_cast<MissingIncludeCompletionItem*>(tester.items.first().data());
    QVERIFY(item);

    QCOMPARE(item->lineToInsert(), QString("#include \"" + include.url().toUrl().fileName() + "\""));

    KTextEditor::Editor* editor = KTextEditor::EditorChooser::editor();
    QVERIFY(editor);

    KTextEditor::Document* doc = editor->createDocument(this);
    QVERIFY(doc);
    QVERIFY(doc->openUrl(active.url().toUrl()));

    item->execute(doc, KTextEditor::Range(9, 0, 9, 3));

    QCOMPARE(doc->text(), QString(
                    "/*\n"
                    "#include \"asdf.h\"\n"
                    "*/\n"
                    "/**\n"
                    "#include \"asdf.h\"\n"
                    "*/\n"
                    "// #include \"asdf.h\"\n"
                    "#include \"" + include.url().toUrl().fileName() + "\"\n"
                    "int main()\n"
                    "{\n"
                    "\n"
                    "}"));
}

#include "test_specialcompletion.moc"
