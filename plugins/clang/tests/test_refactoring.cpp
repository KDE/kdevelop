/*
 * Copyright 2017 Kevin Funk <kfunk@kde.org>
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

