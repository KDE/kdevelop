/*
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_refactoring.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <tests/testproject.h>
#include <interfaces/ilanguagecontroller.h>

#include "codegen/clangrefactoring.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/interfaces/ilanguagesupport.h>

#include <QTemporaryDir>
#include <QLoggingCategory>
#include <QTest>

QTEST_MAIN(TestRefactoring)

using namespace KDevelop;

TestRefactoring::~TestRefactoring() = default;

void TestRefactoring::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));

    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));

    AutoTestShell::init({QStringLiteral("kdevclangsupport")});

    auto core = TestCore::initialize();
    delete core->projectController();
    m_projectController = new TestProjectController(core);
    core->setProjectController(m_projectController);
}

void TestRefactoring::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestRefactoring::testClassRename()
{
    const QString codeBefore(QStringLiteral(R"(
class Foo {
public:
    Foo();
    ~Foo();
};
Foo::Foo() {
}
Foo::~Foo() {
}
    )"));

    const QString codeAfter(QStringLiteral(R"(
class FooNew {
public:
    FooNew();
    ~FooNew();
};
FooNew::FooNew() {
}
FooNew::~FooNew() {
}
    )"));

    QTemporaryDir dir;
    auto project = new TestProject(Path(dir.path()), this);
    m_projectController->addProject(project);

    TestFile file(codeBefore, QStringLiteral("cpp"), project, dir.path());
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses));

    DUChainReadLocker lock;

    auto top = file.topContext();
    QVERIFY(top);

    auto declaration = top->localDeclarations().first();
    QVERIFY(declaration);

    const QString originalName = declaration->identifier().identifier().str();
    const QString newName = QStringLiteral("FooNew");

    QSharedPointer<BasicRefactoringCollector> collector(new BasicRefactoringCollector(declaration));

    // TODO: Do this without GUI?
    UsesWidget uses(declaration, collector);
    lock.unlock();

    for (int i = 0; i < 30000; i += 1000) {
        if (collector->isReady()) {
            break;
        }
        QTest::qWait(1000);
    }
    QVERIFY(collector->isReady());

    BasicRefactoring::NameAndCollector nameAndCollector{newName, collector};

    auto languages = ICore::self()->languageController()->languagesForUrl(file.url().toUrl());
    QVERIFY(!languages.isEmpty());
    auto clangLanguageSupport = languages.first();
    QVERIFY(clangLanguageSupport);
    auto clangRefactoring = qobject_cast<ClangRefactoring*>(clangLanguageSupport->refactoring());
    QVERIFY(clangRefactoring);

    clangRefactoring->renameCollectedDeclarations(nameAndCollector.collector.data(), newName, originalName);
    QCOMPARE(file.fileContents(), codeAfter);

    m_projectController->closeAllProjects();
}

