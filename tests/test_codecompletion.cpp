/*
 * Copyright 2014  David Stevens <dgedstevens@gmail.com>
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
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

#include "test_codecompletion.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include <duchain/parsesession.h>
#include <util/clangtypes.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <codecompletion/completionhelper.h>
#include <codecompletion/context.h>

QTEST_GUILESS_MAIN(TestCodeCompletion);

using namespace KDevelop;

using ClangCodeCompletionItemTester = CodeCompletionItemTester<ClangCodeCompletionContext>;

struct CompletionItems {
    KTextEditor::Cursor position;
    QStringList completions;
};
Q_DECLARE_TYPEINFO(CompletionItems, Q_MOVABLE_TYPE);
using CompletionItemsList = QList<CompletionItems>;
Q_DECLARE_METATYPE(CompletionItemsList);

void TestCodeCompletion::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCodeCompletion::cleanupTestCase()
{
    TestCore::shutdown();
}

namespace {

void executeCompletionTest(const QString& code, const CompletionItemsList& expectedCompletionItems,
                           const ClangCodeCompletionContext::ContextFilters& filters = ClangCodeCompletionContext::ContextFilters(
                                ClangCodeCompletionContext::NoBuiltins |
                                ClangCodeCompletionContext::NoMacros))
{
    TestFile file(code, "cpp");
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    DUContextPointer topPtr(top);

    // don't hold DUChain lock when constructing ClangCodeCompletionContext
    lock.unlock();

    foreach(CompletionItems items, expectedCompletionItems) {
        // TODO: We should not need to pass 'session' to the context, should just use the base class ctor
        auto context = new ClangCodeCompletionContext(topPtr, sessionData, items.position, QString());
        context->setFilters(filters);

        DUChainReadLocker lock;
        auto tester = ClangCodeCompletionItemTester(QExplicitlySharedDataPointer<ClangCodeCompletionContext>(context));

        tester.names.sort();
        QCOMPARE(tester.names, items.completions);
    }
}

}

void TestCodeCompletion::testClangCodeCompletion()
{
    QFETCH(QString, code);
    QFETCH(CompletionItemsList, expectedItems);

    executeCompletionTest(code, expectedItems);
}

void TestCodeCompletion::testClangCodeCompletion_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItemsList>("expectedItems");

    QTest::newRow("assignment")
        << "int foo = 5; \nint bar = "
        << CompletionItemsList{{{1,9}, {
            "bar",
            "foo",
        }}};
    QTest::newRow("dotmemberaccess")
        << "class Foo { public: void foo() {} }; int main() { Foo f; \nf. "
        << CompletionItemsList{{{1, 2}, {
            "foo()",
            "operator=(Foo &&)",
            "operator=(const Foo &)",

        }}};
    QTest::newRow("arrowmemberaccess")
        << "class Foo { public: void foo() {} }; int main() { Foo* f = new Foo; \nf-> }"
        << CompletionItemsList{{{1, 3}, {
            "foo()",
            "operator=(Foo &&)",
            "operator=(const Foo &)",
        }}};
    QTest::newRow("enum-case")
        << "enum Foo { foo, bar }; int main() { Foo f; switch (f) {\ncase "
        << CompletionItemsList{{{1,4}, {
            "bar",
            "foo",
        }}};
    QTest::newRow("only-private")
        << "class SomeStruct { private: void priv() {} };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItemsList{{{2, 2}, {
            "operator=(SomeStruct &&)",
            "operator=(const SomeStruct &)",
        }}};
    QTest::newRow("private-friend")
        << "class SomeStruct { private: void priv() {} friend int main(); };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItemsList{{{2, 2}, {
            "operator=(SomeStruct &&)",
            "operator=(const SomeStruct &)",
            "priv()",
        }}};
    QTest::newRow("private-public")
        << "class SomeStruct { public: void pub() {} private: void priv() {} };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItemsList{{{2, 2}, {
            "operator=(SomeStruct &&)",
            "operator=(const SomeStruct &)",
            "pub()",
        }}};
    QTest::newRow("protected-public")
        << "class SomeStruct { public: void pub() {} protected: void prot() {} };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItemsList{{{2, 2}, {
            "operator=(SomeStruct &&)",
            "operator=(const SomeStruct &)",
            "pub()",
        }}};
}

void TestCodeCompletion::testVirtualOverride()
{
    QFETCH(QString, code);
    QFETCH(CompletionItemsList, expectedItems);

    executeCompletionTest(code, expectedItems, ClangCodeCompletionContext::NoClangCompletion);
}

void TestCodeCompletion::testVirtualOverride_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItemsList>("expectedItems");

    QTest::newRow("basic")
        <<  "class Foo { virtual void foo(); virtual char foo(char c, int i, double d); };\n"
            "class Bar : Foo \n{\n}"
        << CompletionItemsList{{{3, 1}, {"foo()", "foo(char c, int i, double d)"}}};

    QTest::newRow("template")
        << "template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
           "class Bar : Foo<char, double> \n{\n}"
        << CompletionItemsList{{{3, 1}, {"foo(char a, double b, int i)"}}};

    QTest::newRow("nested-template")
        << "template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
           "template<class T1, class T2> class Baz { };\n"
           "class Bar : Foo<char, Baz<char, double>> \n{\n}"
        << CompletionItemsList{{{4, 1}, {"foo(char a, Baz<char, double> b, int i)"}}};

    QTest::newRow("multi")
        << "class Foo { virtual int foo(int i); };\n"
           "class Baz { virtual char baz(char c); };\n"
           "class Bar : Foo, Baz \n{\n}"
        << CompletionItemsList{{{4, 1}, {"baz(char c)", "foo(int i)"}}};

    QTest::newRow("deep")
        << "class Foo { virtual int foo(int i); };\n"
           "class Baz : Foo { };\n"
           "class Bar : Baz \n{\n}"
        << CompletionItemsList{{{4, 1}, {"foo(int i)"}}};

    QTest::newRow("pure")
        << "class Foo { virtual void foo() = 0; foo() {} };\n"
           "class Bar : Foo \n{\n}"
        << CompletionItemsList{{{3, 1}, {"foo() = 0"}}};

    QTest::newRow("const")
        << "class Foo { virtual void foo(const int b) const; }\n;"
           "class Bar : Foo \n{\n}"
        << CompletionItemsList{{{3, 1}, {"foo(const int b) const"}}};
}

void TestCodeCompletion::testImplement()
{
    QFETCH(QString, code);
    QFETCH(CompletionItemsList, expectedItems);

    executeCompletionTest(code, expectedItems, ClangCodeCompletionContext::NoClangCompletion);
}

void TestCodeCompletion::testImplement_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItemsList>("expectedItems");

    QTest::newRow("basic")
        << "int foo(char c, int i); \n"
        << CompletionItemsList{{{1, 0}, {"foo(char c, int i)"}}};

    QTest::newRow("class")
        << "class Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItemsList{
                {{2, 0}, {}},
                {{4, 0}, {"Foo::bar(char c, int i)"}}
        };

    QTest::newRow("namespace")
        << "namespace Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItemsList{
                {{2, 0}, {"bar(char c, int i)"}},
                {{4, 0}, {"Foo::bar(char c, int i)"}}
        };

    QTest::newRow("two-namespace")
        << "namespace Foo { int bar(char c, int i); };\n"
           "namespace Foo {\n"
           "};\n"
        << CompletionItemsList{{{2, 0}, {"bar(char c, int i)"}}};

    QTest::newRow("destructor")
        << "class Foo { ~Foo(); }\n"
        << CompletionItemsList{{{1, 0}, {"Foo::~Foo()"}}};

    QTest::newRow("constructor")
        << "class Foo { \n"
                 "Foo(int i); \n"
                 "}; \n"
        << CompletionItemsList{{{3, 1}, {"Foo::Foo(int i)"}}};

    QTest::newRow("template")
        << "template<typename T> class Foo { T bar(T t); };\n"
        << CompletionItemsList{{{1, 1}, {"Foo<T>::bar(T t)"}}};

    QTest::newRow("specialized-template")
        << "template<typename T> class Foo { \n"
           "T bar(T t); \n"
           "}; \n"
           "template<typename T> T Foo<T>::bar(T t){} \n"
           "template<> class Foo<int> { \n"
           "int bar(int t); \n"
           "}\n"
        << CompletionItemsList{{{6, 1}, {"Foo<int>::bar(int t)"}}};

    QTest::newRow("nested-class")
        << "class Foo { \n"
           "class Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItemsList{
            {{3, 1}, {}},
            {{5, 1}, {}},
            {{7, 1}, {"Foo::Bar::baz(char c, int i)"}}
        };

    QTest::newRow("nested-namespace")
        << "namespace Foo { \n"
           "namespace Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItemsList{
            {{3, 1}, {"baz(char c, int i)"}},
            {{5, 1}, {"Bar::baz(char c, int i)"}},
            {{7, 1}, {"Foo::Bar::baz(char c, int i)"}}
        };

    QTest::newRow("partial-template")
        << "template<typename T> class Foo { \n"
           "template<typename U> class Bar;\n"
           "template<typename U> class Bar<U*> { void baz(T t, U u); }\n"
           "}\n"
        << CompletionItemsList{{{5,1}, {"Foo<T>::Bar<U *>::baz(T t, U u)"}}};

    QTest::newRow("variadic")
        << "int foo(...); int bar(int i, ...); \n"
        << CompletionItemsList{{{1, 1}, {"bar(int i, ...)", "foo(...)"}}};

    QTest::newRow("const")
        << "class Foo { int bar() const; };"
        << CompletionItemsList{{{3, 1}, {"Foo::bar() const"}}};

}

void TestCodeCompletion::testInvalidCompletions()
{
    QFETCH(QString, code);
    QFETCH(CompletionItemsList, expectedItems);

    executeCompletionTest(code, expectedItems);
}

void TestCodeCompletion::testInvalidCompletions_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItemsList>("expectedItems");

    QTest::newRow("invalid-context-incomment")
        << "class Foo { int bar() const; };\n/*\n*/"
        << CompletionItemsList{{{2, 0}, {}}};
}


#include "test_codecompletion.moc"
