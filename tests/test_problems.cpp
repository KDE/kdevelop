/*************************************************************************************
 *  Copyright (C) Kevin Funk <kfunk@kde.org>                                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "test_problems.h"

#include "../duchain/clangproblem.h"
#include "../duchain/clangtypes.h"
#include "../duchain/parsesession.h"
#include "../duchain/unknowndeclarationproblem.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/codegen/coderepresentation.h>
#include <language/backgroundparser/backgroundparser.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testfile.h>
#include <tests/testproject.h>
#include <interfaces/ilanguagecontroller.h>

#include <qtest_kde.h>

using namespace KDevelop;

namespace {

const QString FileName = "/tmp/stdin.cpp";

}

QTEST_KDEMAIN(TestProblems, NoGUI)

void TestProblems::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    DUChain::self()->disablePersistentStorage();
    Core::self()->languageController()->backgroundParser()->setDelay(0);
    CodeRepresentation::setDiskChangesForbidden(true);
}

void TestProblems::cleanupTestCase()
{
    TestCore::shutdown();
}

QList<ProblemPointer> TestProblems::parse(const QByteArray& code)
{
    ClangIndex index;
    ParseSession session(ParseSessionData::Ptr(new ParseSessionData(IndexedString(FileName), code, &index)));
    return session.problemsForFile(session.file());
}

void TestProblems::testNoProblems()
{
    const QByteArray code = "int main() {}";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 0);
}

void TestProblems::testBasicProblems()
{
    // expected:
    // <stdin>:1:13: error: expected ';' after class
    // class Foo {}
    //             ^
    //             ;
    const QByteArray code = "class Foo {}";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 1);
    QCOMPARE(problems[0]->diagnostics().size(), 0);
    auto range = problems[0]->rangeInCurrentRevision();
    QCOMPARE(range.start, SimpleCursor(0, 12));
    QCOMPARE(range.end, SimpleCursor(0, 12));
}

void TestProblems::testBasicRangeSupport()
{
    // expected:
    // <stdin>:1:17: warning: expression result unused [-Wunused-value]
    // int main() { (1 + 1); }
    //               ~ ^ ~
    const QByteArray code = "int main() { (1 + 1); }";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 1);
    QCOMPARE(problems[0]->diagnostics().size(), 0);
    auto range = problems[0]->rangeInCurrentRevision();
    QCOMPARE(range.start, SimpleCursor(0, 14));
    QCOMPARE(range.end, SimpleCursor(0, 19));
}

void TestProblems::testChildDiagnostics()
{
    // expected:
    // test.cpp:3:14: error: call to 'foo' is ambiguous
    // int main() { foo(0); }
    //              ^~~
    // test.cpp:1:6: note: candidate function
    // void foo(unsigned int);
    //      ^
    // test.cpp:2:6: note: candidate function
    // void foo(const char*);
    //      ^
    const QByteArray code = "void foo(unsigned int);\n"
                            "void foo(const char*);\n"
                            "int main() { foo(0); }";
    auto problems = parse(code);
    QCOMPARE(problems.size(), 1);
    auto range = problems[0]->rangeInCurrentRevision();
    QCOMPARE(range.start, SimpleCursor(2, 13));
    QCOMPARE(range.end, SimpleCursor(2, 16));
    QCOMPARE(problems[0]->diagnostics().size(), 2);
    const ProblemPointer d1 = problems[0]->diagnostics()[0];
    QCOMPARE(d1->url().str(), FileName);
    QCOMPARE(d1->rangeInCurrentRevision().start, SimpleCursor(0, 5));
    const ProblemPointer d2 = problems[0]->diagnostics()[1];
    QCOMPARE(d2->url().str(), FileName);
    QCOMPARE(d2->rangeInCurrentRevision().start, SimpleCursor(1, 5));
}

Q_DECLARE_METATYPE(QVector<ClangFixit>);

/**
 * Provides a list of possible fixits: http://blog.llvm.org/2010/04/amazing-feats-of-clang-error-recovery.html
 */
void TestProblems::testFixits()
{
    QFETCH(QString, code);
    QFETCH(int, problemsCount);
    QFETCH(QVector<ClangFixit>, fixits);

    auto problems = parse(code.toAscii());

    qDebug() << problems.last()->description();
    QCOMPARE(problems.size(), problemsCount);

    const ClangProblem* p1 = dynamic_cast<ClangProblem*>(problems[0].data());
    QVERIFY(p1);
    ClangFixitAssistant* a1 = qobject_cast<ClangFixitAssistant*>(p1->solutionAssistant().data());
    QVERIFY(a1);

    QCOMPARE(p1->allFixits(), fixits);
}

void TestProblems::testFixits_data()
{
    QTest::addColumn<QString>("code"); // input
    QTest::addColumn<int>("problemsCount");
    QTest::addColumn<QVector<ClangFixit>>("fixits");

    // expected:
    // test -Wextra-tokens
    // /home/krf/test.cpp:2:8: warning: extra tokens at end of #endif directive [-Wextra-tokens]
    // #endif FOO
    //        ^
    //        //
    QTest::newRow("extra-tokens test")
        << "#ifdef FOO\n#endif FOO\n"
        << 1
        << QVector<ClangFixit>{ ClangFixit{"//", DocumentRange(IndexedString(FileName), SimpleRange(1, 7, 1, 7)), QString()} };

    // expected:
    // test.cpp:1:19: warning: empty parentheses interpreted as a function declaration [-Wvexing-parse]
    //     int a();
    //          ^~
    // test.cpp:1:19: note: replace parentheses with an initializer to declare a variable
    //     int a();
    //          ^~
    //           = 0
    QTest::newRow("vexing-parse test")
        << "int main() { int a(); }\n"
        << 1
        << QVector<ClangFixit>{ ClangFixit{" = 0", DocumentRange(IndexedString(FileName), SimpleRange(0, 18, 0, 20)), QString()} };

    // expected:
    // test.cpp:2:21: error: no member named 'someVariablf' in 'C'; did you mean 'someVariable'?
    // int main() { C c; c.someVariablf = 1; }
    //                     ^~~~~~~~~~~~
    //                     someVariable
    QTest::newRow("spell-check test")
        << "class C{ int someVariable; };\n"
           "int main() { C c; c.someVariablf = 1; }\n"
        << 1
        << QVector<ClangFixit>{ ClangFixit{"someVariable", DocumentRange(IndexedString(FileName), SimpleRange(1, 20, 1, 32)), QString()} };
}

void TestProblems::testMissingInclude()
{
    TestFile include("class A {};\n", "h");
    include.parse(TopDUContext::AllDeclarationsAndContexts);

    TestFile workingFile("int main() { A a; }", "cpp");
    workingFile.parse(TopDUContext::AllDeclarationsAndContexts);

    QCOMPARE(include.url().toUrl().upUrl(), workingFile.url().toUrl().upUrl());
    QVERIFY(include.waitForParsed());
    QVERIFY(workingFile.waitForParsed());

    DUChainReadLocker lock;

    QVERIFY(include.topContext());
    TopDUContext* includeTop = DUChainUtils::contentContextFromProxyContext(include.topContext().data());
    QVERIFY(includeTop);
    QVERIFY(includeTop->problems().isEmpty());

    QVERIFY(workingFile.topContext());
    TopDUContext* top = DUChainUtils::contentContextFromProxyContext(workingFile.topContext());
    QVERIFY(top);
    QCOMPARE(top->problems().size(), 1);

    auto problem = dynamic_cast<UnknownDeclarationProblem*>(top->problems().first().data());
    auto assistant = problem->solutionAssistant();
    auto clangFixitAssistant = qobject_cast<ClangFixitAssistant*>(assistant.data());
    QVERIFY(clangFixitAssistant);

    auto fixits = clangFixitAssistant->fixits();
    QCOMPARE(fixits.size(), 3);
    QCOMPARE(fixits[0].replacementText, QString("class A;\n"));
    QCOMPARE(fixits[1].replacementText, QString("struct A;\n")); // TODO: We shouldn't show this
    QCOMPARE(fixits[2].replacementText, QString("#include \"%1\"\n").arg(include.url().toUrl().fileName()));
}

struct ExpectedTodo
{
    QString description;
    SimpleCursor start;
    SimpleCursor end;
};
Q_DECLARE_METATYPE(ExpectedTodo)

void TestProblems::testTodoProblems()
{
    QFETCH(QString, code);
    QFETCH(ExpectedTodo, expectedTodo);

    TestFile file(code, "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    auto problems = top->problems();
    QCOMPARE(problems.size(), 1);
    auto p1 = problems.first();
    QCOMPARE(p1->description(), expectedTodo.description);
    QCOMPARE(p1->finalLocation().start, expectedTodo.start);
    QCOMPARE(p1->finalLocation().end, expectedTodo.end);
}

void TestProblems::testTodoProblems_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<ExpectedTodo>("expectedTodo");

    // we have two problems here:
    // - we cannot search for comments without declarations,
    //   that means: we can only search inside doxygen-style comments
    //   possible fix: -fparse-all-comments -- however: libclang API is lacking here again.
    //   Can only search through comments attached to a valid entity in the AST
    // - we cannot detect the correct location of the comment yet
    // see more comments inside TodoExtractor
    QTest::newRow("simple")
        << "/** TODO: Something */\nint foo;\n/** notodo */\nint bar;"
        << ExpectedTodo{"TODO: Something", {1, 4}, {1, 7}};
    QTest::newRow("simple")
        << "/// FIXME: Something\nint foo;"
        << ExpectedTodo{"FIXME: Something", {1, 4}, {1, 7}};
}


#include "test_problems.moc"
