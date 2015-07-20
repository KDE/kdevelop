/*
 * Copyright 2014  David Stevens <dgedstevens@gmail.com>
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
 * Copyright 2015 Milian Wolff <mail@milianw.de>
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include "duchain/parsesession.h"
#include "util/clangtypes.h"

#include <interfaces/idocumentcontroller.h>

#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/duchain/types/functiontype.h>

#include "codecompletion/completionhelper.h"
#include "codecompletion/context.h"
#include "codecompletion/includepathcompletioncontext.h"
#include "../clangsettings/clangsettingsmanager.h"

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <ktexteditor_version.h>
#if KTEXTEDITOR_VERSION < QT_VERSION_CHECK(5, 10, 0)
Q_DECLARE_METATYPE(KTextEditor::Cursor);
#endif

QTEST_MAIN(TestCodeCompletion);

using namespace KDevelop;

using ClangCodeCompletionItemTester = CodeCompletionItemTester<ClangCodeCompletionContext>;

struct CompletionItems {
    CompletionItems(){}
    CompletionItems(const KTextEditor::Cursor& position, const QStringList& completions, const QStringList& declarationItems = {})
        : position(position)
        , completions(completions)
        , declarationItems(declarationItems)
    {};
    KTextEditor::Cursor position;
    QStringList completions;
    QStringList declarationItems; ///< completion items that have associated declarations. Declarations with higher match quality at the top. @sa KTextEditor::CodeCompletionModel::MatchQuality
};
Q_DECLARE_TYPEINFO(CompletionItems, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CompletionItems);


struct CompletionPriorityItem
{
    CompletionPriorityItem(const QString name, int matchQuality = 0, int inheritanceDepth = 0, const QString failMessage = {})
        : name(name)
        , failMessage(failMessage)
        , matchQuality(matchQuality)
        , inheritanceDepth(inheritanceDepth)
    {}

    QString name;
    QString failMessage;
    int matchQuality;
    int inheritanceDepth;
};

struct CompletionPriorityItems : public CompletionItems
{
    CompletionPriorityItems(){}
    CompletionPriorityItems(const KTextEditor::Cursor& position, const QList<CompletionPriorityItem>& completions)
        : CompletionItems(position, {})
        , completions(completions)
    {};

    QList<CompletionPriorityItem> completions;
};

Q_DECLARE_TYPEINFO(CompletionPriorityItems, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CompletionPriorityItems);

void TestCodeCompletion::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init({QStringLiteral("kdevclangsupport")});
    TestCore::initialize();

    ClangSettingsManager::self()->m_enableTesting = true;
}

void TestCodeCompletion::cleanupTestCase()
{
    TestCore::shutdown();
}

namespace {

void executeCompletionTest(const QString& code, const CompletionItems& expectedCompletionItems,
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

    // TODO: We should not need to pass 'session' to the context, should just use the base class ctor
    auto context = new ClangCodeCompletionContext(topPtr, sessionData, file.url().toUrl(), expectedCompletionItems.position, QString());
    context->setFilters(filters);

    lock.lock();
    auto tester = ClangCodeCompletionItemTester(QExplicitlySharedDataPointer<ClangCodeCompletionContext>(context));

    int previousMatchQuality = 10;
    for(const auto& declarationName : expectedCompletionItems.declarationItems){
        const auto declarationItem = tester.findItem(declarationName);
        QVERIFY(declarationItem);
        QVERIFY(declarationItem->declaration());

        auto matchQuality = tester.itemData(declarationItem, KTextEditor::CodeCompletionModel::Name, KTextEditor::CodeCompletionModel::MatchQuality).toInt();
        QVERIFY(matchQuality <= previousMatchQuality);
        previousMatchQuality = matchQuality;
    }

    tester.names.sort();
    QEXPECT_FAIL("look-ahead function primary type argument", "No API in LibClang to determine expected code completion type", Continue);
    QEXPECT_FAIL("look-ahead template parameter substitution", "No parameters substitution so far", Continue);
    QEXPECT_FAIL("look-ahead auto item", "Auto type, like many other types, is not exposed through LibClang. We assign DelayedType to it instead of IdentifiedType", Continue);
    QCOMPARE(tester.names, expectedCompletionItems.completions);
}

void executeCompletionPriorityTest(const QString& code, const CompletionPriorityItems& expectedCompletionItems,
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

    auto context = new ClangCodeCompletionContext(topPtr, sessionData, file.url().toUrl(), expectedCompletionItems.position, QString());
    context->setFilters(filters);

    lock.lock();
    auto tester = ClangCodeCompletionItemTester(QExplicitlySharedDataPointer<ClangCodeCompletionContext>(context));

    for(const auto& declaration : expectedCompletionItems.completions){
        const auto declarationItem = tester.findItem(declaration.name);
        QVERIFY(declarationItem);
        QVERIFY(declarationItem->declaration());

        auto matchQuality = tester.itemData(declarationItem, KTextEditor::CodeCompletionModel::Name, KTextEditor::CodeCompletionModel::MatchQuality).toInt();
        auto inheritanceDepth = declarationItem->inheritanceDepth();

        if(!declaration.failMessage.isEmpty()){
            QEXPECT_FAIL("", declaration.failMessage.toUtf8().constData(), Continue);
        }
        QVERIFY(matchQuality == declaration.matchQuality && inheritanceDepth == declaration.inheritanceDepth);
    }
}

void executeMemberAccessReplacerTest(const QString& code, const CompletionItems& expectedCompletionItems,
                                     const ClangCodeCompletionContext::ContextFilters& filters = ClangCodeCompletionContext::ContextFilters(
                                            ClangCodeCompletionContext::NoBuiltins |
                                            ClangCodeCompletionContext::NoMacros))
{
    TestFile file(code, "cpp");

    auto document = ICore::self()->documentController()->openDocument(file.url().toUrl());
    QVERIFY(document);
    ICore::self()->documentController()->activateDocument(document);
    auto view = ICore::self()->documentController()->activeTextDocumentView();
    Q_ASSERT(view);
    view->setCursorPosition(expectedCompletionItems.position);

    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    DUContextPointer topPtr(top);

    lock.unlock();

    auto context = new ClangCodeCompletionContext(topPtr, sessionData, file.url().toUrl(), expectedCompletionItems.position, QString());

    QApplication::processEvents();
    document->close(KDevelop::IDocument::Silent);
    // The previous ClangCodeCompletionContext call should replace member access.
    context = new ClangCodeCompletionContext(topPtr, sessionData, file.url().toUrl(), expectedCompletionItems.position, QString());
    context->setFilters(filters);
    lock.lock();
    auto tester = ClangCodeCompletionItemTester(QExplicitlySharedDataPointer<ClangCodeCompletionContext>(context));

    tester.names.sort();
    QEXPECT_FAIL("replace dot to arrow", "Clang doesn't provide the diagnostic when '.' used instead of '->'", Continue);
    QCOMPARE(tester.names, expectedCompletionItems.completions);
}

using IncludeTester = CodeCompletionItemTester<IncludePathCompletionContext>;

QExplicitlySharedDataPointer<IncludePathCompletionContext> executeIncludePathCompletion(TestFile* file, const KTextEditor::Cursor& position)
{
    if (!file->parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST)) {
        QTest::qFail("Failed to parse source file.", __FILE__, __LINE__);
        return {};
    }

    DUChainReadLocker lock;
    auto top = file->topContext();
    if (!top) {
        QTest::qFail("Failed to parse source file.", __FILE__, __LINE__);
        return {};
    }
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    if (!sessionData) {
        QTest::qFail("Failed to acquire parse session data.", __FILE__, __LINE__);
        return {};
    }

    DUContextPointer topPtr(top);

    lock.unlock();

    auto text = file->fileContents();
    int textLength = -1;
    if (position.isValid()) {
        textLength = 0;
        for (int i = 0; i < position.line(); ++i) {
            textLength = text.indexOf('\n', textLength) + 1;
        }
        textLength += position.column();
    }
    auto context = new IncludePathCompletionContext(topPtr, sessionData, file->url().toUrl(), position, text.mid(0, textLength));
    return QExplicitlySharedDataPointer<IncludePathCompletionContext>{context};
}

}

void TestCodeCompletion::testClangCodeCompletion()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);

    executeCompletionTest(code, expectedItems);
}

void TestCodeCompletion::testClangCodeCompletion_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");

    QTest::newRow("assignment")
        << "int foo = 5; \nint bar = "
        << CompletionItems{{1,9}, {
            "bar",
            "foo",
        }, {"bar","foo"}};
    QTest::newRow("dotmemberaccess")
        << "class Foo { public: void foo() {} bool operator=(Foo &&) }; int main() { Foo f; \nf. "
        << CompletionItems{{1, 2}, {
            "foo",
            "operator="
        }, {"foo",  "operator="}};
    QTest::newRow("arrowmemberaccess")
        << "class Foo { public: void foo() {} }; int main() { Foo* f = new Foo; \nf-> }"
        << CompletionItems{{1, 3}, {
            "foo"
        }, {"foo"}};
    QTest::newRow("enum-case")
        << "enum Foo { foo, bar }; int main() { Foo f; switch (f) {\ncase "
        << CompletionItems{{1,4}, {
            "bar",
            "foo",
        }, {"foo", "bar"}};
    QTest::newRow("only-private")
        << "class SomeStruct { private: void priv() {} };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItems{{2, 2}, {
        }};
    QTest::newRow("private-friend")
        << "class SomeStruct { private: void priv() {} friend int main(); };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItems{{2, 2}, {
            "priv",
        }, {"priv"}};
    QTest::newRow("private-public")
        << "class SomeStruct { public: void pub() {} private: void priv() {} };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItems{{2, 2}, {
            "pub",
        }, {"pub"}};
    QTest::newRow("protected-public")
        << "class SomeStruct { public: void pub() {} protected: void prot() {} };\n"
           "int main() { SomeStruct s;\ns. "
        << CompletionItems{{2, 2}, {
            "pub",
        }, {"pub"}};
    QTest::newRow("localVariable")
        << "int main() { int localVariable;\nloc "
        << CompletionItems{{1, 3},
            {"localVariable","main"},
            {"localVariable", "main"}
        };
    QTest::newRow("globalVariable")
        << "int globalVariable;\nint main() { \ngl "
        << CompletionItems{{2, 2},
            {"globalVariable","main"},
            {"globalVariable", "main"}
        };
    QTest::newRow("namespaceVariable")
        << "namespace NameSpace{int variable};\nint main() { \nNameSpace:: "
        << CompletionItems{{2, 11},
            {"variable"},
            {"variable"}
        };
    QTest::newRow("parentVariable")
        << "class A{public: int m_variable;};class B : public A{};\nint main() { B b;\nb. "
        << CompletionItems{{2, 2},
            {"m_variable"},
            {"m_variable"}
        };
    QTest::newRow("itemsPriority")
        << "class A; class B; void f(A); int main(){ A c; B b;f(\n} "
        << CompletionItems{{1, 0},
            {"A", "B", "b", "c", "f", "main"},
            {"c", "A", "b", "B"}
    };
    QTest::newRow("function-arguments")
        << "class Abc; int f(Abc){\n "
        << CompletionItems{{1, 0}, {
            "Abc",
            "f",
        }};
    QTest::newRow("look-ahead int")
        << "struct LookAhead { int intItem;}; int main() {LookAhead* pInstance; LookAhead instance; int i =\n }"
        << CompletionItems{{1, 0}, {
            "LookAhead", "i", "instance",
            "instance.intItem", "main",
            "pInstance", "pInstance->intItem",
        }};
    QTest::newRow("look-ahead class")
        << "class Class{}; struct LookAhead {Class classItem;}; int main() {LookAhead* pInstance; LookAhead instance; Class cl =\n }"
        << CompletionItems{{1, 0}, {
            "Class", "LookAhead", "cl",
            "instance", "instance.classItem",
            "main", "pInstance", "pInstance->classItem",
        }};
    QTest::newRow("look-ahead function argument")
        << "class Class{}; struct LookAhead {Class classItem;}; void function(Class cl);"
           "int main() {LookAhead* pInstance; LookAhead instance; function(\n }"
        << CompletionItems{{1, 0}, {
            "Class", "LookAhead", "function",
            "instance", "instance.classItem",
            "main", "pInstance", "pInstance->classItem",
        }};
    QTest::newRow("look-ahead function primary type argument")
        << "struct LookAhead {double doubleItem;}; void function(double double);"
           "int main() {LookAhead* pInstance; LookAhead instance; function(\n }"
        << CompletionItems{{1, 0}, {
            "LookAhead", "function", "instance",
            "instance.doubleItem", "main",
            "pInstance", "pInstance->doubleItem",
        }};
    QTest::newRow("look-ahead typedef")
        << "typedef double DOUBLE; struct LookAhead {DOUBLE doubleItem;};"
           "int main() {LookAhead* pInstance; LookAhead instance; double i =\n "
        << CompletionItems{{1, 0}, {
            "DOUBLE", "LookAhead", "i",
            "instance", "instance.doubleItem",
            "main", "pInstance", "pInstance->doubleItem",
        }};
    QTest::newRow("look-ahead pointer")
        << "struct LookAhead {int* pInt;};"
           "int main() {LookAhead* pInstance; LookAhead instance; int* i =\n "
        << CompletionItems{{1, 0}, {
            "LookAhead", "i", "instance",
            "instance.pInt", "main",
            "pInstance", "pInstance->pInt",
        }};
    QTest::newRow("look-ahead template")
        << "template <typename T> struct LookAhead {int intItem;};"
           "int main() {LookAhead<int>* pInstance; LookAhead<int> instance; int i =\n "
        << CompletionItems{{1, 0}, {
            "LookAhead", "i", "instance",
            "instance.intItem", "main",
            "pInstance", "pInstance->intItem",
        }};
    QTest::newRow("look-ahead template parameter substitution")
        << "template <typename T> struct LookAhead {T itemT;};"
           "int main() {LookAhead<int>* pInstance; LookAhead<int> instance; int i =\n "
        << CompletionItems{{1, 0}, {
            "LookAhead", "i", "instance",
            "instance.itemT", "main",
            "pInstance", "pInstance->itemT",
        }};
        QTest::newRow("look-ahead item access")
        << "class Class { public: int publicInt; protected: int protectedInt; private: int privateInt;};"
           "int main() {Class cl; int i =\n "
        << CompletionItems{{1, 0}, {
            "Class", "cl",
            "cl.publicInt",
            "i", "main",
        }};

        QTest::newRow("look-ahead auto item")
        << "struct LookAhead { int intItem; };"
           "int main() {auto instance = LookAhead(); int i = \n "
        << CompletionItems{{1, 0}, {
            "LookAhead",
            "i",
            "instance",
            "instance.intItem"
        }};
}

void TestCodeCompletion::testReplaceMemberAccess()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);

    executeMemberAccessReplacerTest(code, expectedItems);
}

void TestCodeCompletion::testReplaceMemberAccess_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");

    QTest::newRow("replace arrow to dot")
    <<  "struct Struct { void function(); };"
        "int main() { Struct s; \ns-> "
    << CompletionItems{{1, 3}, {
        "function"
    }};

    QTest::newRow("replace dot to arrow")
    <<  "struct Struct { void function(); };"
        "int main() { Struct* s; \ns. "
    << CompletionItems{{1, 2}, {
        "function"
    }};
}

void TestCodeCompletion::testVirtualOverride()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);

    executeCompletionTest(code, expectedItems, ClangCodeCompletionContext::NoClangCompletion);
}

void TestCodeCompletion::testVirtualOverride_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");

    QTest::newRow("basic")
        <<  "class Foo { virtual void foo(); virtual void foo(char c); virtual char foo(char c, int i, double d); };\n"
            "class Bar : Foo \n{void foo(char c) override;\n}"
        << CompletionItems{{3, 1}, {"foo()", "foo(char c, int i, double d)"}};

    QTest::newRow("template")
        << "template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); virtual T2 overridden(T1 a); } ;\n"
           "class Bar : Foo<char, double> \n{double overridden(char a) override;\n}"
        << CompletionItems{{3, 1}, {"foo(char a, double b, int i)"}};

    QTest::newRow("nested-template")
        << "template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); virtual T2 overridden(T1 a, T2 b, int i); } ;\n"
           "template<class T1, class T2> class Baz { };\n"
           "class Bar : Foo<char, Baz<char, double>> \n{Baz<char, double> overridden(char a, Baz<char, double> b, int i) override;\n}"
        << CompletionItems{{4, 1}, {"foo(char a, Baz<char, double> b, int i)"}};

    QTest::newRow("multi")
        << "class Foo { virtual int foo(int i); virtual int overridden(int i); };\n"
           "class Baz { virtual char baz(char c); };\n"
           "class Bar : Foo, Baz \n{int overridden(int i) override;\n}"
        << CompletionItems{{4, 1}, {"baz(char c)", "foo(int i)"}};

    QTest::newRow("deep")
        << "class Foo { virtual int foo(int i); virtual int overridden(int i); };\n"
           "class Baz : Foo { };\n"
           "class Bar : Baz \n{int overridden(int i) overriden;\n}"
        << CompletionItems{{4, 1}, {"foo(int i)"}};

    QTest::newRow("pure")
        << "class Foo { virtual void foo() = 0; virtual void overridden() = 0;};\n"
           "class Bar : Foo \n{void overridden() override;\n};"
        << CompletionItems{{3, 0}, {"foo() = 0"}};

    QTest::newRow("const")
        << "class Foo { virtual void foo(const int b) const; virtual void overridden(const int b) const; }\n;"
           "class Bar : Foo \n{void overridden(const int b) const override;\n}"
        << CompletionItems{{3, 1}, {"foo(const int b) const"}};
}

void TestCodeCompletion::testImplement()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);

    executeCompletionTest(code, expectedItems, ClangCodeCompletionContext::NoClangCompletion);
}

void TestCodeCompletion::testImplement_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");

    QTest::newRow("basic")
        << "int foo(char c, int i); \n"
        << CompletionItems{{1, 0}, {"foo(char c, int i)"}};

    QTest::newRow("class")
        << "class Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItems{{2, 0}, {}};

    QTest::newRow("class2")
        << "class Foo { \n"
            "int bar(char c, int i); \n\n"
            "}; \n"
        << CompletionItems{{4, 0}, {"Foo::bar(char c, int i)"}};

    QTest::newRow("namespace")
        << "namespace Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItems{{2, 0}, {"bar(char c, int i)"}};

    QTest::newRow("namespace2")
        << "namespace Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItems{{4, 0}, {"Foo::bar(char c, int i)"}};

    QTest::newRow("two-namespace")
        << "namespace Foo { int bar(char c, int i); };\n"
           "namespace Foo {\n"
           "};\n"
        << CompletionItems{{2, 0}, {"bar(char c, int i)"}};

    QTest::newRow("destructor")
        << "class Foo { ~Foo(); }\n"
        << CompletionItems{{1, 0}, {"Foo::~Foo()"}};

    QTest::newRow("constructor")
        << "class Foo { \n"
                 "Foo(int i); \n"
                 "}; \n"
        << CompletionItems{{3, 1}, {"Foo::Foo(int i)"}};

    QTest::newRow("template")
        << "template<typename T> class Foo { T bar(T t); };\n"
        << CompletionItems{{1, 1}, {"Foo<T>::bar(T t)"}};

    QTest::newRow("specialized-template")
        << "template<typename T> class Foo { \n"
           "T bar(T t); \n"
           "}; \n"
           "template<typename T> T Foo<T>::bar(T t){} \n"
           "template<> class Foo<int> { \n"
           "int bar(int t); \n"
           "}\n"
        << CompletionItems{{6, 1}, {"Foo<int>::bar(int t)"}};

    QTest::newRow("nested-class")
        << "class Foo { \n"
           "class Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItems {{3, 1}, {}};

    QTest::newRow("nested-class2")
        << "class Foo { \n"
           "class Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItems {{5, 1}, {}};

    QTest::newRow("nested-class3")
        << "class Foo { \n"
           "class Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItems {{7, 1}, {"Foo::Bar::baz(char c, int i)"}};

    QTest::newRow("nested-namespace")
        << "namespace Foo { \n"
           "namespace Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItems {{3, 1}, {"baz(char c, int i)"}};

    QTest::newRow("nested-namespace2")
        << "namespace Foo { \n"
           "namespace Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItems {{5, 1}, {"Bar::baz(char c, int i)"}};

    QTest::newRow("nested-namespace3")
        << "namespace Foo { \n"
           "namespace Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItems {{7, 1}, {"Foo::Bar::baz(char c, int i)"}};

    QTest::newRow("partial-template")
        << "template<typename T> class Foo { \n"
           "template<typename U> class Bar;\n"
           "template<typename U> class Bar<U*> { void baz(T t, U u); }\n"
           "}\n"
        << CompletionItems{{5,1}, {"Foo<T>::Bar<U *>::baz(T t, U u)"}};

    QTest::newRow("variadic")
        << "int foo(...); int bar(int i, ...); \n"
        << CompletionItems{{1, 1}, {"bar(int i, ...)", "foo(...)"}};

    QTest::newRow("const")
        << "class Foo { int bar() const; };"
        << CompletionItems{{3, 1}, {"Foo::bar() const"}};

    QTest::newRow("multiple-methods")
        << "class Foo { int bar(); void foo(); char asdf() const; };"
        << CompletionItems{{3, 1}, {"Foo::asdf() const", "Foo::bar()", "Foo::foo()"}};
}

void TestCodeCompletion::testInvalidCompletions()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);

    executeCompletionTest(code, expectedItems);
}

void TestCodeCompletion::testInvalidCompletions_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");

    QTest::newRow("invalid-context-incomment")
        << "class Foo { int bar() const; };\n/*\n*/"
        << CompletionItems{{2, 0}, {}};
}

void TestCodeCompletion::testIncludePathCompletion_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<KTextEditor::Cursor>("cursor");
    QTest::addColumn<QString>("itemId");
    QTest::addColumn<QString>("result");

    QTest::newRow("global-1") << QString("#include ") << KTextEditor::Cursor(0, 9)
                              << QString("iostream") << QString("#include <iostream>");
    QTest::newRow("global-2") << QString("#include <") << KTextEditor::Cursor(0, 9)
                              << QString("iostream") << QString("#include <iostream>");
    QTest::newRow("global-3") << QString("#include <") << KTextEditor::Cursor(0, 10)
                              << QString("iostream") << QString("#include <iostream>");
    QTest::newRow("global-4") << QString("#  include <") << KTextEditor::Cursor(0, 12)
                              << QString("iostream") << QString("#  include <iostream>");
    QTest::newRow("global-5") << QString("#  include   <") << KTextEditor::Cursor(0, 14)
                              << QString("iostream") << QString("#  include   <iostream>");
    QTest::newRow("global-6") << QString("#  include   <> /* 1 */") << KTextEditor::Cursor(0, 14)
                              << QString("iostream") << QString("#  include   <iostream> /* 1 */");
    QTest::newRow("global-7") << QString("#  include /* 1 */ <> /* 1 */") << KTextEditor::Cursor(0, 21)
                              << QString("iostream") << QString("#  include /* 1 */ <iostream> /* 1 */");
    QTest::newRow("global-8") << QString("# /* 1 */ include /* 1 */ <> /* 1 */") << KTextEditor::Cursor(0, 28)
                              << QString("iostream") << QString("# /* 1 */ include /* 1 */ <iostream> /* 1 */");
    QTest::newRow("global-9") << QString("#include <cstdint>") << KTextEditor::Cursor(0, 10)
                              << QString("iostream") << QString("#include <iostream>");
    QTest::newRow("global-10") << QString("#include <cstdint>") << KTextEditor::Cursor(0, 14)
                              << QString("cstdint") << QString("#include <cstdint>");
    QTest::newRow("global-11") << QString("#include <cstdint>") << KTextEditor::Cursor(0, 17)
                              << QString("cstdint") << QString("#include <cstdint>");
    QTest::newRow("local-0") << QString("#include \"") << KTextEditor::Cursor(0, 10)
                              << QString("foo/") << QString("#include \"foo/\"");
    QTest::newRow("local-1") << QString("#include \"foo/\"") << KTextEditor::Cursor(0, 14)
                              << QString("bar/") << QString("#include \"foo/bar/\"");
    QTest::newRow("local-2") << QString("#include \"foo/") << KTextEditor::Cursor(0, 14)
                              << QString("bar/") << QString("#include \"foo/bar/\"");
    QTest::newRow("local-3") << QString("# /* 1 */ include /* 1 */ \"\" /* 1 */") << KTextEditor::Cursor(0, 28)
                              << QString("foo/") << QString("# /* 1 */ include /* 1 */ \"foo/\" /* 1 */");
    QTest::newRow("local-4") << QString("# /* 1 */ include /* 1 */ \"foo/\" /* 1 */") << KTextEditor::Cursor(0, 31)
                              << QString("bar/") << QString("# /* 1 */ include /* 1 */ \"foo/bar/\" /* 1 */");
    QTest::newRow("local-5") << QString("#include \"foo/\"") << KTextEditor::Cursor(0, 10)
                              << QString("foo/") << QString("#include \"foo/\"");
    QTest::newRow("local-6") << QString("#include \"foo/asdf\"") << KTextEditor::Cursor(0, 10)
                              << QString("foo/") << QString("#include \"foo/\"");
    QTest::newRow("local-7") << QString("#include \"foo/asdf\"") << KTextEditor::Cursor(0, 14)
                              << QString("bar/") << QString("#include \"foo/bar/\"");
}

void TestCodeCompletion::testIncludePathCompletion()
{
    QFETCH(QString, code);
    QFETCH(KTextEditor::Cursor, cursor);
    QFETCH(QString, itemId);
    QFETCH(QString, result);

    QTemporaryDir tempDir;
    QDir dir(tempDir.path());
    QVERIFY(dir.mkpath("foo/bar/asdf"));
    TestFile file(code, "cpp", 0, tempDir.path());
    IncludeTester tester(executeIncludePathCompletion(&file, cursor));
    QVERIFY(tester.completionContext);
    QVERIFY(tester.completionContext->isValid());

    auto item = tester.findItem(itemId);
    QVERIFY(item);

    KTextEditor::Editor* editor = KTextEditor::Editor::instance();
    QVERIFY(editor);

    auto doc = std::unique_ptr<KTextEditor::Document>(editor->createDocument(this));
    QVERIFY(doc.get());
    QVERIFY(doc->openUrl(file.url().toUrl()));

    QWidget parent;
    auto view = doc->createView(&parent);
    item->execute(view, KTextEditor::Range(cursor, cursor));
    QCOMPARE(doc->text(), result);

    const auto newCursor = view->cursorPosition();
    QCOMPARE(newCursor.line(), cursor.line());
    if (!itemId.endsWith('/')) {
        // file inserted, cursor should be at end of line
        QCOMPARE(newCursor.column(), doc->lineLength(cursor.line()));
    } else {
        // directory inserted, cursor should be before the " or >
        const auto cursorChar = doc->characterAt(newCursor);
        QVERIFY(cursorChar == '"' || cursorChar == '>');
    }
}

void TestCodeCompletion::testIncludePathCompletionLocal()
{
    TestFile header("int foo() { return 42; }\n", "h");
    TestFile impl("#include \"", "cpp", &header);

    IncludeTester tester(executeIncludePathCompletion(&impl, {0, 10}));
    QVERIFY(tester.names.contains(header.url().toUrl().fileName()));
    QVERIFY(!tester.names.contains("iostream"));
}

void TestCodeCompletion::testOverloadedFunctions()
{
    TestFile file("void f(); int f(int); void f(int, double){\n ", "cpp");
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    DUContextPointer topPtr(top);
    lock.unlock();

    const auto context = new ClangCodeCompletionContext(topPtr, sessionData, file.url().toUrl(), {1, 0}, QString());
    context->setFilters(ClangCodeCompletionContext::ContextFilters(
                            ClangCodeCompletionContext::NoBuiltins |
                            ClangCodeCompletionContext::NoMacros));
    lock.lock();
    const auto tester = ClangCodeCompletionItemTester(QExplicitlySharedDataPointer<ClangCodeCompletionContext>(context));
    QCOMPARE(tester.items.size(), 3);
    for (const auto& item : tester.items) {
        auto function = item->declaration()->type<FunctionType>();
        const QString display = item->declaration()->identifier().toString() + function->partToString(FunctionType::SignatureArguments);
        const QString itemDisplay = tester.itemData(item).toString() + tester.itemData(item, KTextEditor:: CodeCompletionModel::Arguments).toString();
        QCOMPARE(display, itemDisplay);
    }

    QVERIFY(tester.items[0]->declaration().data() != tester.items[1]->declaration().data());
    QVERIFY(tester.items[0]->declaration().data() != tester.items[2]->declaration().data());
    QVERIFY(tester.items[1]->declaration().data() != tester.items[2]->declaration().data());
}

void TestCodeCompletion::testCompletionPriority()
{
    QFETCH(QString, code);
    QFETCH(CompletionPriorityItems, expectedItems);

    executeCompletionPriorityTest(code, expectedItems);
}

void TestCodeCompletion::testCompletionPriority_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionPriorityItems>("expectedItems");

    QTest::newRow("pointer")
        << "class A{}; class B{}; class C : public B{}; int main(){A* a; B* b; C* c; b =\n "
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21}, {"b", 9, 0},
        {"c", 8, 0, QStringLiteral("Pointer to derived class is not added to the Best Matches group")}}};

    QTest::newRow("class")
        << "class A{}; class B{}; class C : public B{}; int main(){A a; B b; C c; b =\n "
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21}, {"b", 9, 0},
        {"c", 8, 0, QStringLiteral("Derived class is not added to the Best Matches group")}}};

    QTest::newRow("primary-types")
        << "class A{}; int main(){A a; int b; bool c = \n "
        << CompletionPriorityItems{{1,0}, {{"a", 0, 34}, {"b", 8, 0}, {"c", 9, 0}}};

    QTest::newRow("reference")
        << "class A{}; class B{}; class C : public B{};"
           "int main(){A tmp; A& a = tmp; C tmp2; C& c = tmp2; B& b =\n ;}"
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21}, {"b", 9, 0},
        {"c", 8, 0, QStringLiteral("Reference to derived class is not added to the Best Matches group")}}};

    QTest::newRow("typedef")
        << "struct A{}; struct B{}; typedef A AA; typedef B BB; void f(A p);"
           "int main(){ BB b; AA a; f(\n }"
        << CompletionPriorityItems{{1,0}, {{"a", 9, 0}, {"b", 0, 21}}};

    QTest::newRow("returnType")
        << "struct A{}; struct B{}; struct Test{A f();B g(); Test() { A a =\n }};"
        << CompletionPriorityItems{{1,0}, {{"f", 9, 0}, {"g", 0, 21}}};

    QTest::newRow("template")
        << "template <typename T> class Class{}; template <typename T> class Class2{};"
           "int main(){ Class<int> a; Class2<int> b =\n }"
        << CompletionPriorityItems{{1,0}, {{"b", 9, 0}, {"a", 0, 21}}};

    QTest::newRow("protected-access")
        << "class Base { protected: int m_protected; };"
           "class Derived: public Base {public: void g(){\n }};"
        << CompletionPriorityItems{{1,0}, {{"m_protected", 0, 37}}};

    QTest::newRow("protected-access2")
        << "class Base { protected: int m_protected; };"
           "class Derived: public Base {public: void f();};"
           "void Derived::f(){\n }"
        << CompletionPriorityItems{{1,0}, {{"m_protected", 0, 37}}};
}

void TestCodeCompletion::testVariableScope()
{
    TestFile file("int var; \nvoid test(int var) {int tmp =\n }", "cpp");
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    DUContextPointer topPtr(top);
    lock.unlock();

    const auto context = new ClangCodeCompletionContext(topPtr, sessionData, file.url().toUrl(), {2, 0}, QString());
    context->setFilters(ClangCodeCompletionContext::ContextFilters(
                            ClangCodeCompletionContext::NoBuiltins |
                            ClangCodeCompletionContext::NoMacros));
    lock.lock();
    const auto tester = ClangCodeCompletionItemTester(QExplicitlySharedDataPointer<ClangCodeCompletionContext>(context));

    QCOMPARE(tester.items.size(), 4);
    auto item = tester.findItem(QStringLiteral("var"));
    VERIFY(item);
    QCOMPARE(item->declaration()->range().start, CursorInRevision(1, 14));
}
