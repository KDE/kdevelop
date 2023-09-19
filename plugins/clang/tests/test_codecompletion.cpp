/*
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_codecompletion.h"
#include <language/backgroundparser/backgroundparser.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <tests/testproject.h>

#include "duchain/parsesession.h"
#include "duchain/clanghelpers.h"
#include "util/clangtypes.h"

#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/duchain/types/functiontype.h>

#include "codecompletion/completionhelper.h"
#include "codecompletion/context.h"
#include "codecompletion/includepathcompletioncontext.h"
#include "../clangsettings/clangsettingsmanager.h"

#include "sanitizer_test_init.h"

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <KConfigGroup>

#include <QVersionNumber>
#include <QStandardPaths>

#include <optional>

// TODO: QTEST_MAIN_WRAPPER not part of public documented API
QTEST_MAIN_WRAPPER(TestCodeCompletion, KDevelop::sanitizerTestInit(argv); QApplication app(argc, argv););

static const auto NoMacroOrBuiltin = ClangCodeCompletionContext::ContextFilters(
    ClangCodeCompletionContext::NoBuiltins | ClangCodeCompletionContext::NoMacros);

using namespace KDevelop;

using ClangCodeCompletionItemTester = CodeCompletionItemTester<ClangCodeCompletionContext>;

struct CompletionItems {
    CompletionItems(){}
    CompletionItems(const KTextEditor::Cursor& position, const QStringList& completions,
                    const QStringList& declarationItems = {}, const std::optional<QString>& code = std::nullopt)
        : position(position)
        , completions(completions)
        , declarationItems(declarationItems)
        , code(code){};
    KTextEditor::Cursor position;
    QStringList completions;
    QStringList declarationItems; ///< completion items that have associated declarations. Declarations with higher match quality at the top. @sa KTextEditor::CodeCompletionModel::MatchQuality
    std::optional<QString> code; ///< If we expect the completion (not) to change the code, e.g., arrow-to-dot
};
Q_DECLARE_TYPEINFO(CompletionItems, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CompletionItems)


struct CompletionPriorityItem
{
    CompletionPriorityItem(const QString& name, int matchQuality = 0, int inheritanceDepth = 0, const QString& failMessage = {})
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
Q_DECLARE_METATYPE(CompletionPriorityItems)

namespace {

struct NoopTestFunction
{
    void operator()(const ClangCodeCompletionItemTester& /*tester*/) const
    {
    }
};

QString textForDocument(const QUrl& url, const KTextEditor::Cursor& position)
{
    bool close = false;

    auto* doc = ICore::self()->documentController()->documentForUrl(url);
    if (!doc) {
        doc = ICore::self()->documentController()->openDocument(url);
        close = true;
    }

    auto text = doc->textDocument()->text({{0, 0}, position});

    if (close) {
        doc->close(IDocument::Discard);
    }

    return text;
}

QExplicitlySharedDataPointer<ClangCodeCompletionContext> createContext(const ReferencedTopDUContext& top,
                                                                       const ParseSessionData::Ptr& sessionData,
                                                                       const KTextEditor::Cursor position,
                                                                       const QString& code = {})
{
    const auto url = top->url().toUrl();
    const auto text = code.isEmpty() ? textForDocument(url, position) : code;
    return QExplicitlySharedDataPointer<ClangCodeCompletionContext>{
        new ClangCodeCompletionContext(DUContextPointer(top), sessionData, url, position, text)};
}

template<typename CustomTestFunction = NoopTestFunction>
void executeCompletionTest(const ReferencedTopDUContext& top, const CompletionItems& expectedCompletionItems,
                           const ClangCodeCompletionContext::ContextFilters& filters = NoMacroOrBuiltin,
                           CustomTestFunction customTestFunction = {})
{
    DUChainReadLocker lock;
    QVERIFY(top);
    QVERIFY(top->ast());
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);
    lock.unlock();
    // TODO: We should not need to pass 'session' to the context, should just use the base class ctor
    auto context = createContext(top, sessionData, expectedCompletionItems.position);
    context->setFilters(filters);
    lock.lock();

    auto tester = ClangCodeCompletionItemTester(context);

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
#if CINDEX_VERSION_MINOR < 30
    QEXPECT_FAIL("look-ahead auto item", "Auto type, like many other types, is not exposed through LibClang. We assign DelayedType to it instead of IdentifiedType", Continue);
#endif
    if (QTest::currentTestFunction() == QByteArrayLiteral("testImplementAfterEdit") && expectedCompletionItems.position.line() == 3) {
        QEXPECT_FAIL("", "TU is not properly updated after edit", Continue);
    }
    if (QTest::currentTestFunction() == QByteArrayLiteral("testClangCodeCompletion")) {
        QEXPECT_FAIL("look-ahead pointer", "self-assignment isn't done anymore, so we don't find any suitable type anymore", Continue);

        if (QVersionNumber::fromString(ClangHelpers::clangVersion()) >= QVersionNumber(9, 0, 0)) {
            QEXPECT_FAIL("enum-case", "quite a lot of unrelated cruft is suggested, needs to be fixed upstream", Continue);
        }
    }
    if (tester.names.size() != expectedCompletionItems.completions.size()) {
        qDebug() << "different results:\nactual:" << tester.names << "\nexpected:" << expectedCompletionItems.completions;
    }
    QCOMPARE(tester.names, expectedCompletionItems.completions);

    lock.unlock(); // customTestFunction should lock appropriately
    customTestFunction(tester);
}

template<typename CustomTestFunction = NoopTestFunction>
void executeCompletionTest(const QString& code, const CompletionItems& expectedCompletionItems,
                           const ClangCodeCompletionContext::ContextFilters& filters = NoMacroOrBuiltin,
                           CustomTestFunction customTestFunction = {},
                           // Using QStringLiteral fails here with Ubuntu Bionic's GNU 7.4.0 C++ compiler,
                           // Assembler messages:  Error: symbol `_ZZNK12_GLOBAL__N_1UlvE_clEvE15qstring_literal' is already defined
                           // Seems the symbol is not including the template arguments, but might be created
                           // per instantiation of the template
                           // Seems fixed in newer versions of GCC
                           const QString& fileExtension = QString::fromLatin1("cpp"))
{
    TestFile file(code, fileExtension);
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    executeCompletionTest(file.topContext(), expectedCompletionItems, filters, customTestFunction);
}

void executeCompletionPriorityTest(const QString& code, const CompletionPriorityItems& expectedCompletionItems,
                           const ClangCodeCompletionContext::ContextFilters& filters = NoMacroOrBuiltin)
{
    TestFile file(code, QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    // don't hold DUChain lock when constructing ClangCodeCompletionContext
    lock.unlock();

    auto context = createContext(top, sessionData, expectedCompletionItems.position);
    context->setFilters(filters);

    lock.lock();
    auto tester = ClangCodeCompletionItemTester(context);

    for(const auto& declaration : expectedCompletionItems.completions){
        qDebug() << "verifying declaration:" << declaration.name;

        const auto declarationItem = tester.findItem(declaration.name);
        if (!declarationItem || !declarationItem->declaration()) {
            qDebug() << "names of all completion-items:" << tester.names;
        }
        QVERIFY(declarationItem);
        QVERIFY(declarationItem->declaration());

        auto matchQuality = tester.itemData(declarationItem, KTextEditor::CodeCompletionModel::Name, KTextEditor::CodeCompletionModel::MatchQuality).toInt();
        auto inheritanceDepth = declarationItem->inheritanceDepth();

        if (!declaration.failMessage.isEmpty()) {
            QEXPECT_FAIL("", declaration.failMessage.toUtf8().constData(), Continue);
        }
        QVERIFY2(matchQuality == declaration.matchQuality && inheritanceDepth == declaration.inheritanceDepth,
                 qPrintable(QStringLiteral("%1 == %2 && %3 == %4")
                                .arg(matchQuality)
                                .arg(declaration.matchQuality)
                                .arg(inheritanceDepth)
                                .arg(declaration.inheritanceDepth)));
    }
}

void executeMemberAccessReplacerTest(const QString& code, const CompletionItems& expectedCompletionItems,
                                     const ClangCodeCompletionContext::ContextFilters& filters = NoMacroOrBuiltin)
{
    TestFile file(code, QStringLiteral("cpp"));

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

    lock.unlock();

    auto context = createContext(top, sessionData, expectedCompletionItems.position, code);

    QApplication::processEvents();

    if (expectedCompletionItems.code) {
        QCOMPARE(document->textDocument()->text(), *expectedCompletionItems.code);
    }

    document->close(KDevelop::IDocument::Silent);

    // The previous ClangCodeCompletionContext call should replace member access.
    // That triggers an update request in the duchain which we are not interested in,
    // so let's stop that request.
    ICore::self()->languageController()->backgroundParser()->removeDocument(file.url());

    context = createContext(top, sessionData, expectedCompletionItems.position);
    context->setFilters(filters);
    lock.lock();
    auto tester = ClangCodeCompletionItemTester(context);

    tester.names.sort();
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

constexpr const char* definesAndIncludesConfigName = "CustomDefinesAndIncludes";

}

void TestCodeCompletion::cleanup()
{
    KSharedConfig::openConfig()->deleteGroup(definesAndIncludesConfigName);
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
            "foo",
        }, {"foo"}};
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
            {"A", "B", "b", "c", "f",
#if CINDEX_VERSION_MINOR >= 30
             "f",
#endif
                         "main"},
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
            "LookAhead", "instance",
            "instance.intItem", "main",
            "pInstance", "pInstance->intItem",
        }};
    QTest::newRow("look-ahead class")
        << "class Class{}; struct LookAhead {Class classItem;}; int main() {LookAhead* pInstance; LookAhead instance; Class cl =\n }"
        << CompletionItems{{1, 0}, {
            "Class", "LookAhead",
            "instance", "instance.classItem",
            "main", "pInstance", "pInstance->classItem",
        }};
    QTest::newRow("look-ahead function argument")
        << "class Class{}; struct LookAhead {Class classItem;}; void function(Class cl);"
           "int main() {LookAhead* pInstance; LookAhead instance; function(\n }"
        << CompletionItems{{1, 0}, {
            "Class", "LookAhead", "function",
#if CINDEX_VERSION_MINOR >= 30
            "function",
#endif
            "instance", "instance.classItem",
            "main", "pInstance", "pInstance->classItem",
        }};
    QTest::newRow("look-ahead function primary type argument")
        << "struct LookAhead {double doubleItem;}; void function(double d);"
           "int main() {LookAhead* pInstance; LookAhead instance; function(\n }"
        << CompletionItems{{1, 0}, {
            "LookAhead",
            "function",
#if CINDEX_VERSION_MINOR >= 30
            "function",
#endif
            "instance",
            "instance.doubleItem", "main",
            "pInstance", "pInstance->doubleItem",
        }};
    QTest::newRow("look-ahead typedef")
        << "typedef double DOUBLE; struct LookAhead {DOUBLE doubleItem;};"
           "int main() {LookAhead* pInstance; LookAhead instance; double i =\n "
        << CompletionItems{{1, 0}, {
            "DOUBLE", "LookAhead",
            "instance", "instance.doubleItem",
            "main", "pInstance", "pInstance->doubleItem",
        }};
    QTest::newRow("look-ahead pointer")
        << "struct LookAhead {int* pInt;};"
           "int main() {LookAhead* pInstance; LookAhead instance; int* i =\n "
        << CompletionItems{{1, 0}, {
            "LookAhead", "instance",
            "instance.pInt", "main",
            "pInstance", "pInstance->pInt",
        }};
    QTest::newRow("look-ahead template")
        << "template <typename T> struct LookAhead {int intItem;};"
           "int main() {LookAhead<int>* pInstance; LookAhead<int> instance; int i =\n "
        << CompletionItems{{1, 0}, {
            "LookAhead", "instance",
            "instance.intItem", "main",
            "pInstance", "pInstance->intItem",
        }};
    QTest::newRow("look-ahead template parameter substitution")
        << "template <typename T> struct LookAhead {T itemT;};"
           "int main() {LookAhead<int>* pInstance; LookAhead<int> instance; int i =\n "
        << CompletionItems{{1, 0}, {
            "LookAhead", "instance",
            "instance.itemT", "main",
            "pInstance", "pInstance->itemT",
        }};
    QTest::newRow("look-ahead item access")
        << "class Class { public: int publicInt; protected: int protectedInt; private: int privateInt;};"
           "int main() {Class cl; int i =\n "
        << CompletionItems{{1, 0}, {
            "Class", "cl",
            "cl.publicInt",
            "main",
        }};

    QTest::newRow("look-ahead auto item")
        << "struct LookAhead { int intItem; };"
           "int main() {auto instance = LookAhead(); int i = \n "
        << CompletionItems{{1, 0}, {
            "LookAhead",
            "instance",
            "instance.intItem",
            "main"
        }};

    QTest::newRow("variadic template recursive class")
        << R"(
template <typename Head, typename ...Tail>
struct my_class : Head, my_class<Tail...>
{
    using base = Head;
};)"
        << CompletionItems{{3, 17}, { "Head", "Tail", "my_class" }};
}


void TestCodeCompletion::testClangCodeCompletionType()
{
    QFETCH(QString, fileExtension);
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);
    QFETCH(QString, expedtedItem);
    QFETCH(QString, expectedType);

    auto executeItem = [=] (const ClangCodeCompletionItemTester& tester) {
        auto item = tester.findItem(expedtedItem);
        QVERIFY(item);
        auto declaration = item->declaration();
        QVERIFY(declaration);
        QCOMPARE(declaration->abstractType()->toString(), expectedType);
    };

    executeCompletionTest(code, expectedItems, NoMacroOrBuiltin, executeItem, fileExtension);
}

void TestCodeCompletion::testClangCodeCompletionType_data()
{
    QTest::addColumn<QString>("fileExtension");
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");
    QTest::addColumn<QString>("expedtedItem");
    QTest::addColumn<QString>("expectedType");

    QTest::newRow("bug409041")
        << "c"
        << "typedef struct { int bitmask[1]; } bitmask_a;\n"
           "typedef struct { int bitmask[6]; } bitmask_c;\n"
           "typedef union { bitmask_c bitmask; } bitmask_union;\n"
           "int main() { bitmask_union u;\n"
           "u. \n "
        << CompletionItems{{4, 2}, { "bitmask" }}
        << "bitmask"
        << "bitmask_c";
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

    {
        const auto code = QString::fromLatin1(
            "struct Struct { void function(); };"
            "int main() { Struct s; \ns");

        CompletionItems cis;
        cis.position = {1, 3};
        cis.completions = QStringList{"function"};
        cis.code = code + ".";
        QTest::newRow("replace arrow to dot") << (code + "->") << cis;
    }

    {
        const auto code = QString::fromLatin1(
            "struct Struct { void function(); };"
            "int main() { Struct* s; \ns");

        CompletionItems cis;
        cis.position = {1, 3};
        cis.completions = QStringList{"function"};
        cis.code = code + "->";
        QTest::newRow("replace dot to arrow") << (code + ".") << cis;
    }

    {
        const auto code = QString::fromLatin1("int main() { double a = \n0.  ");

        CompletionItems cis;
        cis.position = {1, 2};
        cis.completions = QStringList{};
        cis.code = code;
        QTest::newRow("no replacement needed") << code << cis;
    }

    // See https://bugs.kde.org/show_bug.cgi?id=460870
    {
        const auto code = QString::fromLatin1(
            "struct S { double a; double b; };"
            "int main() { S c[2] = { {.6, \n.");

        CompletionItems cis;
        cis.position = {1, 1};
        cis.completions = QStringList{};
        cis.code = code;
        QTest::newRow("no replacement in aggregate initializer with floating point number") << code << cis;
    }

    // See https://bugs.kde.org/show_bug.cgi?id=468605
    {
        const auto code = QString::fromLatin1(
            "struct Test { int a; float b; };"
            "int main(int argc, char **argv) { Test foo{1, \n2.");

        CompletionItems cis;
        cis.position = {1, 2};
        cis.completions = QStringList{};
        cis.code = code;
        QTest::newRow("no replacement in aggregate initializer with floating point number 2") << code << cis;
    }

    {
        const auto code = QString::fromLatin1(
            "typedef struct myStruct { int x; } myStruct;"
            "myStruct fn() { return (myStruct) { \n.");

        CompletionItems cis;
        cis.position = {1, 1};
        cis.completions = QStringList{"x"};
        cis.code = code;
        QTest::newRow("no replacement in aggregate initializer with member") << code << cis;
    }
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
           "class Bar : Baz \n{int overridden(int i) override;\n}"
        << CompletionItems{{4, 1}, {"foo(int i)"}};

    QTest::newRow("repeated")
        << "class Foo { virtual int foo(int i); virtual int overridden(int i); };\n"
           "class Baz : Foo { int foo(int i) override; };\n"
           "class Bar : Baz \n{int overridden(int i) override;\n}"
        << CompletionItems{{4, 1}, {"foo(int i)"}};

    QTest::newRow("pure")
        << "class Foo { virtual void foo() = 0; virtual void overridden() = 0;};\n"
           "class Bar : Foo \n{void overridden() override;\n};"
        << CompletionItems{{3, 0}, {"foo() = 0"}};

    QTest::newRow("repeated-pure")
        << "class Foo { virtual void foo() = 0; virtual void overridden() = 0; };\n"
           "class Baz : Foo { void foo() override; };\n"
           "class Bar : Baz \n{void overridden() override;\n}"
        << CompletionItems{{4, 1}, {"foo()"}};

    QTest::newRow("const")
        << "class Foo { virtual void foo(const int b) const; virtual void overridden(const int b) const; }\n;"
           "class Bar : Foo \n{void overridden(const int b) const override;\n}"
        << CompletionItems{{3, 1}, {"foo(const int b) const"}};

    QTest::newRow("dont-override")
        << R"(class A {
                  virtual void something() = 0;
              };
              class B : public A
              {
              public:
                  void foo();
              };
              void B::foo() {}
        )" << CompletionItems{{8, 14}, {}};
}

void TestCodeCompletion::testOverrideExecute()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);
    QFETCH(QString, itemToExecute);
    QFETCH(QString, cppStandard);
    QFETCH(QString, expectedCode);

    QTemporaryDir directory;
    TestProject testProject {Path{directory.path()}};
    auto t = testProject.path().toLocalFile();
    auto configGroup = testProject.projectConfiguration()->group(definesAndIncludesConfigName).group("ProjectPath0");
    configGroup.writeEntry("Path", ".");
    configGroup.writeEntry("parserArguments", cppStandard);
    configGroup.sync();
    m_projectController->addProject(&testProject);

    TestFile file(code, QStringLiteral("cpp"), &testProject, directory.path());
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));

    auto executeItem = [=] (const ClangCodeCompletionItemTester& tester) {
        auto item = tester.findItem(itemToExecute);
        QVERIFY(item);
        auto view = createView(tester.completionContext->duContext()->url().toUrl());
        DUChainReadLocker lock;
        item->execute(view.get(), view->document()->wordRangeAt(expectedItems.position));
        QCOMPARE(view->document()->text(), expectedCode);
    };
    executeCompletionTest(file.topContext(), expectedItems, NoMacroOrBuiltin, executeItem);
    m_projectController->closeProject(&testProject);
}

void TestCodeCompletion::testOverrideExecute_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");
    QTest::addColumn<QString>("itemToExecute");
    QTest::addColumn<QString>("cppStandard");
    QTest::addColumn<QString>("expectedCode");

    QTest::newRow("override-modern")
        << "class Foo { virtual int bar(char c); };\nclass Baz : Foo\n{\n};"
        << CompletionItems{{3, 0}, {"Baz", "Foo", "bar(char c)"}}
        << "bar(char c)"
        << "-std=c++11"
        << "class Foo { virtual int bar(char c); };\n"
           "class Baz : Foo\n{\nint bar(char c) override;};";

    QTest::newRow("override-non-modern")
        << "class Foo { virtual int bar(char c); };\nclass Baz : Foo\n{\n};"
        << CompletionItems{{3, 0}, {"Baz", "Foo", "bar(char c)"}}
        << "bar(char c)"
        << "-std=c++98"
        << "class Foo { virtual int bar(char c); };\n"
           "class Baz : Foo\n{\nint bar(char c);};";

     QTest::newRow("override-unknown")
        << "class Foo { virtual int bar(char c); };\nclass Baz : Foo\n{\n};"
        << CompletionItems{{3, 0}, {"Baz", "Foo", "bar(char c)"}}
        << "bar(char c)"
        << "-std=c++17"
        << "class Foo { virtual int bar(char c); };\n"
           "class Baz : Foo\n{\nint bar(char c) override;};";
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

    QTest::newRow("anonymous-namespace")
        << R"(
                namespace {
                    int bar(char c, int i);
                };
            )"
        << CompletionItems{{3, 0}, {"bar(char c, int i)"}};

    QTest::newRow("anonymous-namespace2")
        << R"(
                namespace {
                    int bar(char c, int i);
                };
           )"
        << CompletionItems{{4, 0}, {}};

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

    QTest::newRow("noexcept")
        << "class Foo { int bar() noexcept; };"
        << CompletionItems{{3, 1}, {"Foo::bar() noexcept"}};

    QTest::newRow("throw()")
        << "class Foo { int bar() throw(); };"
        << CompletionItems{{3, 1}, {"Foo::bar() throw()"}};

    QTest::newRow("multiple-methods")
        << "class Foo { int bar(); void foo(); char asdf() const; };"
        << CompletionItems{{1, 1}, {"Foo::asdf() const", "Foo::bar()", "Foo::foo()"}};

    // explicitly deleted/defaulted functions should not appear in the implements completion
    QTest::newRow("deleted-copy-ctor")
        << "struct S { S(); S(const S&) = /*some comment*/ delete; };"
        << CompletionItems{{1,1}, {"S::S()"}};
    QTest::newRow("deleted-overload-member")
        << "struct Foo {\n"
           "  int x();\n"
           "  int x(int) =delete;\n"
           "};\n"
        << CompletionItems{{5,1}, {"Foo::x()"}};
    QTest::newRow("deleted-overload-global")
        << "int x();\n"
           "int x(int)=  delete;\n"
        << CompletionItems{{2,1}, {"x()"}};
    QTest::newRow("defaulted-copy-ctor")
        << "struct S { S(); S(const S&) = default; };"
        << CompletionItems{{1,1}, {"S::S()"}};
    QTest::newRow("defaulted-dtor")
        << "struct Foo {\n"
           "  Foo();\n"
           "  ~Foo() =default;\n"
           "};\n"
        << CompletionItems{{5,1}, {"Foo::Foo()"}};

    QTest::newRow("bug355163")
        << R"(
                #include <type_traits>
                namespace test {

                template<typename T, typename U>
                struct IsSafeConversion : public std::is_same<T, typename std::common_type<T, U>::type>
                {

                };

                } // namespace test
            )"
        << CompletionItems{{7,0}, {}};

    QTest::newRow("bug355954")
        << R"(
                struct Hello {
                    struct Private;
                };

                struct Hello::Private {
                    void test();
                };
            )"
        << CompletionItems{{8,0}, {"Hello::Private::test()"}};

    QTest::newRow("lineOfNextFunction")
        << "void foo();\nvoid bar() {}"
        << CompletionItems{{1,0}, {"foo()"}};

    QTest::newRow("pure")
        << R"(
                struct Hello {
                    virtual void foo() = 0;
                    virtual void bar();
                };
            )"
        << CompletionItems{{5, 0}, {"Hello::bar()"}};

    QTest::newRow("bug368544")
        << R"(
                class Klass {
                public:
                    template <typename T>
                    void func(int a, T x, int b) const;
                };
            )"
        << CompletionItems{{6, 0}, {"Klass::func(int a, T x, int b) const"}};

}

void TestCodeCompletion::testImplementOtherFile()
{
    TestFile header1(QStringLiteral("void foo();"), QStringLiteral("h"));
    QVERIFY(header1.parseAndWait());
    TestFile header2(QStringLiteral("void bar();"), QStringLiteral("h"));
    QVERIFY(header2.parseAndWait());
    TestFile impl(QString("#include \"%1\"\n"
                          "#include \"%2\"\n"
                          "void asdf();\n\n")
                    .arg(header1.url().str(), header2.url().str()),
                  QStringLiteral("cpp"), &header1);

    CompletionItems expectedItems{{3,1}, {"asdf()", "foo()"}};
    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    executeCompletionTest(impl.topContext(), expectedItems);
}

void TestCodeCompletion::testImplementAfterEdit()
{
    TestFile header1(QStringLiteral("void foo();"), QStringLiteral("h"));
    QVERIFY(header1.parseAndWait());
    TestFile impl(QString("#include \"%1\"\n"
                          "void asdf() {}\nvoid bar() {}")
                    .arg(header1.url().str()),
                  QStringLiteral("cpp"), &header1);

    auto document = ICore::self()->documentController()->openDocument(impl.url().toUrl());

    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));

    CompletionItems expectedItems{{2,0}, {"foo()"}};
    executeCompletionTest(impl.topContext(), expectedItems);

    document->textDocument()->insertText(expectedItems.position, QStringLiteral("\n"));
    expectedItems.position.setLine(3);

    executeCompletionTest(impl.topContext(), expectedItems);

    document->close(IDocument::Discard);
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

    QTest::newRow("global-1") << QStringLiteral("#include ") << KTextEditor::Cursor(0, 9)
                              << QStringLiteral("iostream") << QStringLiteral("#include <iostream>");
    QTest::newRow("global-2") << QStringLiteral("#include <") << KTextEditor::Cursor(0, 9)
                              << QStringLiteral("iostream") << QStringLiteral("#include <iostream>");
    QTest::newRow("global-3") << QStringLiteral("#include <") << KTextEditor::Cursor(0, 10)
                              << QStringLiteral("iostream") << QStringLiteral("#include <iostream>");
    QTest::newRow("global-4") << QStringLiteral("#  include <") << KTextEditor::Cursor(0, 12)
                              << QStringLiteral("iostream") << QStringLiteral("#  include <iostream>");
    QTest::newRow("global-5") << QStringLiteral("#  include   <") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("iostream") << QStringLiteral("#  include   <iostream>");
    QTest::newRow("global-6") << QStringLiteral("#  include   <> /* 1 */") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("iostream") << QStringLiteral("#  include   <iostream> /* 1 */");
    QTest::newRow("global-7") << QStringLiteral("#  include /* 1 */ <> /* 1 */") << KTextEditor::Cursor(0, 21)
                              << QStringLiteral("iostream") << QStringLiteral("#  include /* 1 */ <iostream> /* 1 */");
    QTest::newRow("global-8") << QStringLiteral("# /* 1 */ include /* 1 */ <> /* 1 */") << KTextEditor::Cursor(0, 28)
                              << QStringLiteral("iostream") << QStringLiteral("# /* 1 */ include /* 1 */ <iostream> /* 1 */");
    QTest::newRow("global-9") << QStringLiteral("#include <cstdint>") << KTextEditor::Cursor(0, 10)
                              << QStringLiteral("iostream") << QStringLiteral("#include <iostream>");
    QTest::newRow("global-10") << QStringLiteral("#include <cstdint>") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("cstdint") << QStringLiteral("#include <cstdint>");
    QTest::newRow("global-11") << QStringLiteral("#include <cstdint>") << KTextEditor::Cursor(0, 17)
                              << QStringLiteral("cstdint") << QStringLiteral("#include <cstdint>");
    QTest::newRow("local-0") << QStringLiteral("#include \"") << KTextEditor::Cursor(0, 10)
                              << QStringLiteral("foo/") << QStringLiteral("#include \"foo/\"");
    QTest::newRow("local-1") << QStringLiteral("#include \"foo/\"") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("bar/") << QStringLiteral("#include \"foo/bar/\"");
    QTest::newRow("local-2") << QStringLiteral("#include \"foo/") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("bar/") << QStringLiteral("#include \"foo/bar/\"");
    QTest::newRow("local-3") << QStringLiteral("# /* 1 */ include /* 1 */ \"\" /* 1 */") << KTextEditor::Cursor(0, 28)
                              << QStringLiteral("foo/") << QStringLiteral("# /* 1 */ include /* 1 */ \"foo/\" /* 1 */");
    QTest::newRow("local-4") << QStringLiteral("# /* 1 */ include /* 1 */ \"foo/\" /* 1 */") << KTextEditor::Cursor(0, 31)
                              << QStringLiteral("bar/") << QStringLiteral("# /* 1 */ include /* 1 */ \"foo/bar/\" /* 1 */");
    QTest::newRow("local-5") << QStringLiteral("#include \"foo/\"") << KTextEditor::Cursor(0, 10)
                              << QStringLiteral("foo/") << QStringLiteral("#include \"foo/\"");
    QTest::newRow("local-6") << QStringLiteral("#include \"foo/asdf\"") << KTextEditor::Cursor(0, 10)
                              << QStringLiteral("foo/") << QStringLiteral("#include \"foo/\"");
    QTest::newRow("local-7") << QStringLiteral("#include \"foo/asdf\"") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("bar/") << QStringLiteral("#include \"foo/bar/\"");
    QTest::newRow("dash-1") << QStringLiteral("#include \"") << KTextEditor::Cursor(0, 10)
                              << QStringLiteral("dash-file.h") << QStringLiteral("#include \"dash-file.h\"");
    QTest::newRow("dash-2") << QStringLiteral("#include \"dash-") << KTextEditor::Cursor(0, 15)
                              << QStringLiteral("dash-file.h") << QStringLiteral("#include \"dash-file.h\"");
    QTest::newRow("dash-4") << QStringLiteral("#include \"dash-file.h\"") << KTextEditor::Cursor(0, 13)
                              << QStringLiteral("dash-file.h") << QStringLiteral("#include \"dash-file.h\"");
    QTest::newRow("dash-5") << QStringLiteral("#include \"dash-file.h\"") << KTextEditor::Cursor(0, 14)
                              << QStringLiteral("dash-file.h") << QStringLiteral("#include \"dash-file.h\"");
    QTest::newRow("dash-6") << QStringLiteral("#include \"dash-file.h\"") << KTextEditor::Cursor(0, 15)
                              << QStringLiteral("dash-file.h") << QStringLiteral("#include \"dash-file.h\"");
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
    TestFile file(code, QStringLiteral("cpp"), nullptr, tempDir.path());
    {
        QFile otherFile(tempDir.path() + "/dash-file.h");
        QVERIFY(otherFile.open(QIODevice::WriteOnly));
    }
    IncludeTester tester(executeIncludePathCompletion(&file, cursor));
    QVERIFY(tester.completionContext);
    QVERIFY(tester.completionContext->isValid());

    auto item = tester.findItem(itemId);
    QVERIFY(item);

    auto view = createView(file.url().toUrl());
    QVERIFY(view.get());
    auto doc = view->document();
    item->execute(view.get(), KTextEditor::Range(cursor, cursor));
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
    TestFile header(QStringLiteral("int foo() { return 42; }\n"), QStringLiteral("h"));
    TestFile impl(QStringLiteral("#include \""), QStringLiteral("cpp"), &header);

    IncludeTester tester(executeIncludePathCompletion(&impl, {0, 10}));
    QVERIFY(tester.names.contains(header.url().toUrl().fileName()));
    QVERIFY(tester.names.contains("iostream"));
}

void TestCodeCompletion::testOverloadedFunctions()
{
    TestFile file(QStringLiteral("void f(); int f(int); void f(int, double){\n "), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    lock.unlock();

    const auto context = createContext(top, sessionData, {1, 0});
    context->setFilters(NoMacroOrBuiltin);
    lock.lock();
    const auto tester = ClangCodeCompletionItemTester(context);
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
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21}, {"b", 6, 0},
        {"c", 2, 0, QStringLiteral("Pointer to derived class is not added to the Best Matches group")}}};

    QTest::newRow("look-ahead pointer") << "class A{}; class B{}; struct LookAhead {A* a; B* b;}; "
                                           "int main() {LookAhead* pInstance; LookAhead instance; A* a; B* b; b =\n "
                                        << CompletionPriorityItems{{1, 0},
                                                                   {{"a", 0, 21},
                                                                    {"b", 6, 0},
                                                                    {"pInstance", 0, 21},
                                                                    {"instance", 0, 34},
                                                                    {"pInstance->b", 6, 0},
                                                                    {"instance.b", 6, 0}}};

    QTest::newRow("class")
        << "class A{}; class B{}; class C : public B{}; int main(){A a; B b; C c; b =\n "
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21}, {"b", 6, 0},
        {"c", 2, 0, QStringLiteral("Derived class is not added to the Best Matches group")}}};

    QTest::newRow("look-ahead class") << "class A{}; class B{}; struct LookAhead {A a; B b;}; "
                                         "int main() {LookAhead* pInstance; LookAhead instance; A a; B b; b =\n "
                                      << CompletionPriorityItems{{1, 0},
                                                                 {{"a", 0, 21},
                                                                  {"B", 4, 0},
                                                                  {"b", 6, 0},
                                                                  {"pInstance", 0, 34},
                                                                  {"instance", 0, 21},
                                                                  {"pInstance->b", 6, 0},
                                                                  {"instance.b", 6, 0}}};

    QTest::newRow("look-ahead class, no local variable of the type")
        << "class A{}; class B{}; struct LookAhead {A a; B b;}; "
           "int main() {LookAhead* pInstance; LookAhead instance; A a; B b =\n "
        << CompletionPriorityItems{{1, 0},
                                   {{"a", 0, 21},
                                    {"B", 4, 0},
                                    {"pInstance", 0, 34},
                                    {"instance", 0, 21},
                                    {"pInstance->b", 6, 0,
                                     QStringLiteral("No local variable with high match quality => look-ahead "
                                                    "match quality equals that of \"class B\" declaration")}}};

    QTest::newRow("primary-types") << "class A{}; int main(){A a; int b; bool c; bool d = \n "
                                   << CompletionPriorityItems{{1, 0}, {{"a", 0, 34}, {"b", 2, 0}, {"c", 6, 0}}};

    QTest::newRow("look-ahead primary-types")
        << "class A{}; struct LookAhead {A t; int x; bool y; unsigned z;}; "
           "int main() {LookAhead* pInstance; LookAhead instance; A a; int b; bool c; bool d = \n "
        << CompletionPriorityItems{{1, 0},
                                   {{"a", 0, 34},
                                    {"b", 2, 0},
                                    {"c", 6, 0},
                                    {"pInstance", 0, 34},
                                    {"instance", 0, 34},
                                    {"pInstance->x", 2, 0},
                                    {"instance.x", 2, 0},
                                    {"pInstance->y", 6, 0},
                                    {"instance.y", 6, 0}
#if 0
        // This fails, not just xfails, because "instance.z" is not among completion-items.
        , {"instance.z", 2, 0, QStringLiteral("No local variable with the type")}
#endif
                                   }};

    QTest::newRow("reference")
        << "class A{}; class B{}; class C : public B{};"
           "int main(){A tmp; A& a = tmp; C tmp2; C& c = tmp2; B& b =\n ;}"
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21},
        {"c", 2, 0, QStringLiteral("Reference to derived class is not added to the Best Matches group")}}};

    QTest::newRow("typedef")
        << "struct A{}; struct B{}; typedef A AA; typedef B BB; void f(A p);"
           "int main(){ BB b; AA a; f(\n }"
        << CompletionPriorityItems{{1,0}, {{"a", 6, 0}, {"b", 0, 21}}};

    QTest::newRow("returnType")
        << "struct A{}; struct B{}; struct Test{A f();B g(); Test() { A a =\n }};"
        << CompletionPriorityItems{{1,0}, {{"f", 6, 0}, {"g", 0, 21}}};

    QTest::newRow("look-ahead returnType")
        << "struct A{}; struct B{}; struct LookAhead {A a; B b;}; "
           "struct Test{A f();B g(); Test() { LookAhead* pInstance; LookAhead instance; A a =\n }};"
        << CompletionPriorityItems{{1, 0},
                                   {{"f", 6, 0},
                                    {"g", 0, 21},
                                    {"pInstance", 0, 34},
                                    {"instance", 0, 21},
                                    {"pInstance->a", 6, 0},
                                    {"instance.a", 6, 0}}};

    QTest::newRow("template")
        << "template <typename T> class Class{}; template <typename T> class Class2{};"
           "int main(){ Class<int> a; Class2<int> b =\n }"
        << CompletionPriorityItems{{1,0}, {{"a", 0, 21}}};

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
    TestFile file(QStringLiteral("int var; \nvoid test(int var) {int tmp =\n }"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    lock.unlock();

    const auto context = createContext(top, sessionData, {2, 0});
    context->setFilters(NoMacroOrBuiltin);
    lock.lock();
    const auto tester = ClangCodeCompletionItemTester(context);

    QCOMPARE(tester.items.size(), 3);
    auto item = tester.findItem(QStringLiteral("var"));
    VERIFY(item);
    QCOMPARE(item->declaration()->range().start, CursorInRevision(1, 14));
}

struct HintItem
{
    QString hint;
    bool hasDeclaration;
    bool operator==(const HintItem& rhs) const
    {
        return std::tie(hint, hasDeclaration) == std::tie(rhs.hint, rhs.hasDeclaration);
    }
    bool operator<(const HintItem& rhs) const
    {
        return std::tie(hint, hasDeclaration) < std::tie(rhs.hint, rhs.hasDeclaration);
    }
    QByteArray toString() const
    {
        return "HintItem(" + hint.toUtf8() + ", " + (hasDeclaration ? "true" : "false") + ')';
    }
};
Q_DECLARE_METATYPE(HintItem)
using HintItemList = QVector<HintItem>;
namespace QTest {
template<>
char *toString(const HintItem& hint)
{
    return qstrdup(hint.toString());
}
template<>
char *toString(const HintItemList& hints)
{
    QByteArray ba = "HintItemList(";
    for (int i = 0, c = hints.size(); i < c; ++i) {
        ba += hints[i].toString();
        if (i == c - 1) {
            ba += ')';
        } else {
            ba += ", ";
        }
    }
    return qstrdup(ba.constData());
}
}

void TestCodeCompletion::testArgumentHintCompletion()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);
    QFETCH(HintItemList, hints);

    executeCompletionTest(code, expectedItems, NoMacroOrBuiltin, [&](const ClangCodeCompletionItemTester& tester) {
        DUChainReadLocker lock;
        HintItemList actualHints;
        for (const auto& item : tester.items) {
            if (item->argumentHintDepth() == 1) {
                actualHints << HintItem{
                    tester.itemData(item).toString() + tester.itemData(item, KTextEditor:: CodeCompletionModel::Arguments).toString(),
                    item->declaration()
                };
            }
        }
        std::sort(hints.begin(), hints.end());
        std::sort(actualHints.begin(), actualHints.end());
        QEXPECT_FAIL("member function", "clang_getCompletionParent returns nothing, thus decl lookup fails", Continue);
        QEXPECT_FAIL("namespaced function", "clang_getCompletionParent returns nothing, thus decl lookup fails", Continue);
        QEXPECT_FAIL("namespaced constructor", "clang_getCompletionParent returns nothing, thus decl lookup fails", Continue);
        QCOMPARE(actualHints, hints);
    });
}

void TestCodeCompletion::testArgumentHintCompletion_data()
{
#if CINDEX_VERSION_MINOR < 30
    QSKIP("You need at least LibClang 3.7");
#endif
    qRegisterMetaType<HintItemList>("HintItemList");

    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");
    QTest::addColumn<HintItemList>("hints");

    QTest::newRow("global function")
        << "void foo(int);\n"
           "int main() { \nfoo( "
        << CompletionItems{{2,4}, {
            "foo", "foo",
            "main"
        }}
        << HintItemList{{"foo(int)", true}};

    QTest::newRow("namespaced function")
        << "namespace ns { void foo(int); }\n"
           "int main() { \nns::foo( "
        << CompletionItems{{2,4}, {
            "foo"
        }}
        << HintItemList{{"foo(int)", true}};

    QTest::newRow("member function")
        << "struct Struct{ void foo(int);}\n"
           "int main() {Struct s; \ns.foo( "
        << CompletionItems{{2,6}, {
            "Struct", "foo",
            "main", "s"
        }}
        << HintItemList{{"foo(int)", true}};

    QTest::newRow("template function")
        << "template <typename T> void foo(T);\n"
           "int main() { \nfoo( "
        << CompletionItems{{2,6}, {
            "foo", "foo",
            "main"
        }}
        << HintItemList{{"foo(T)", true}};

    QTest::newRow("overloaded functions")
        << "void foo(int); void foo(int, double)\n"
           "int main() { \nfoo( "
        << CompletionItems{{2,6}, {
            "foo", "foo", "foo", "foo",
            "main"
        }}
        << HintItemList{
            {"foo(int)", true},
            {"foo(int, double)", true}
        };

    QTest::newRow("overloaded functions2")
        << "void foo(int); void foo(int, double)\n"
           "int main() { foo(1,\n  "
        << CompletionItems{{2,1}, {
            "foo", "foo", "foo",
            "main"
        }}
        << HintItemList{{"foo(int, double)", true}};

    QTest::newRow("constructor")
        << "struct foo { foo(int); foo(int, double); }\n"
           "int main() { foo f(\n "
        << CompletionItems{{2,1}, {
            "f", "foo", "foo", "foo", "foo", "foo",
            "main"
        }}
        << HintItemList{
            {"foo(int)", true},
            {"foo(int, double)", true},
            {"foo(foo &&)", false},
            {"foo(const foo &)", false}
        };

    QTest::newRow("constructor2")
        << "struct foo { foo(int); foo(int, double); }\n"
           "int main() { foo f(1,\n "
        << CompletionItems{{2,1}, {
            "f", "foo", "foo",
            "main"
        }}
        << HintItemList{
            {"foo(int, double)", true}
        };

    QTest::newRow("namespaced constructor")
        << "namespace ns { struct foo { foo(int); foo(int, double); } }\n"
           "int main() { ns::foo f(\n "
        << CompletionItems{{2,1}, {
            "f", "foo", "foo", "foo", "foo",
            "main", "ns"
        }}
        << HintItemList{
            {"foo(int)", true},
            {"foo(int, double)", true},
            {"foo(ns::foo &&)", false},
            {"foo(const ns::foo &)", false}
        };
}

void TestCodeCompletion::testArgumentHintCompletionDefaultParameters()
{
#if CINDEX_VERSION_MINOR < 30
    QSKIP("You need at least LibClang 3.7");
#endif

    TestFile file(QStringLiteral("void f(int i, int j = 0, double k =1){\nf( "), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));
    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    const ParseSessionData::Ptr sessionData(dynamic_cast<ParseSessionData*>(top->ast().data()));
    QVERIFY(sessionData);

    lock.unlock();

    const auto context = createContext(top, sessionData, {1, 2});
    context->setFilters(NoMacroOrBuiltin);
    lock.lock();
    const auto tester = ClangCodeCompletionItemTester(context);
    QExplicitlySharedDataPointer<KDevelop::CompletionTreeItem> f;

    for (const auto& item : tester.items) {
        if (item->argumentHintDepth() == 1) {
            f = item;
            break;
        }
    }

    QVERIFY(f.data());

    const QString itemDisplay = tester.itemData(f).toString() + tester.itemData(f, KTextEditor:: CodeCompletionModel::Arguments).toString();
    QCOMPARE(itemDisplay, QStringLiteral("f(int i, int j = 0, double k = 1)"));
}

void TestCodeCompletion::testCompleteFunction()
{
    QFETCH(QString, code);
    QFETCH(CompletionItems, expectedItems);
    QFETCH(QString, itemToExecute);
    QFETCH(QString, expectedCode);

    auto executeItem = [=] (const ClangCodeCompletionItemTester& tester) {
        auto item = tester.findItem(itemToExecute);
        QVERIFY(item);
        auto view = createView(tester.completionContext->duContext()->url().toUrl());
        DUChainReadLocker lock;
        item->execute(view.get(), view->document()->wordRangeAt(expectedItems.position));
        QCOMPARE(view->document()->text(), expectedCode);
    };
    executeCompletionTest(code, expectedItems, NoMacroOrBuiltin, executeItem);
}

void TestCodeCompletion::testCompleteFunction_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItems>("expectedItems");
    QTest::addColumn<QString>("itemToExecute");
    QTest::addColumn<QString>("expectedCode");

    QTest::newRow("add-parens")
        << "int foo();\nint main() {\n\n}"
        << CompletionItems({2, 0}, {"foo", "main"})
        << "foo"
        << "int foo();\nint main() {\nfoo()\n}";

    QTest::newRow("keep-parens")
        << "int foo();\nint main() {\nfoo();\n}"
        << CompletionItems({2, 0}, {"foo", "main"})
        << "main"
        << "int foo();\nint main() {\nmain();\n}";

    QTest::newRow("bug375635")
        << "enum class Color {\nBlue, Green, Red, Yellow\n};\nvoid foo() {\nColor x;\nswitch (x) {\ncase : break;}\n}"
        << CompletionItems({6, 5}, {"Blue", "Green", "Red", "Yellow"})
        << "Yellow"
        << "enum class Color {\nBlue, Green, Red, Yellow\n};\nvoid foo() {\nColor x;\nswitch (x) {\ncase Color::Yellow: break;}\n}";

    QTest::newRow("bug368544")
        << "class Klass {\npublic:\ntemplate <typename T>\nvoid func(int a, T x, int b) const;\n};\n"
        << CompletionItems({5, 0}, {"Klass", "Klass::func(int a, T x, int b) const"})
        << "Klass::func(int a, T x, int b) const"
        << "class Klass {\npublic:\ntemplate <typename T>\nvoid func(int a, T x, int b) const;\n};\ntemplate<typename T> void Klass::func(int a, T x, int b) const\n{\n}\n";

    QTest::newRow("bug377397")
        << "template<typename T> class Foo {\nvoid bar();\n};\n"
        << CompletionItems({3, 0}, {"Foo", "Foo<T>::bar()"})
        << "Foo<T>::bar()"
        << "template<typename T> class Foo {\nvoid bar();\n};\ntemplate<typename T> void Foo<T>::bar()\n{\n}\n";

    QTest::newRow("template-template-parameter")
        << "template <template<class, int> class X, typename B>\nclass Test {\npublic:\nvoid bar(B a);\n};\n"
        << CompletionItems({5, 0}, {"Test", "Test<X, B>::bar(B a)"})
        << "Test<X, B>::bar(B a)"
        << "template <template<class, int> class X, typename B>\nclass Test {\npublic:\nvoid bar(B a);\n};\ntemplate<template<typename, int> class X, typename B> void Test<X, B>::bar(B a)\n{\n}\n";

    QTest::newRow("replace-leading-return-type")
        << "void foo(int x);\nvoid "
        << CompletionItems({1, 5}, {"foo(int x)"})
        << "foo(int x)"
        << "void foo(int x);\nvoid foo(int x)\n{\n}\n";

    QTest::newRow("replace-leading-function-name")
        << "void foo(int x);\nfoo"
        << CompletionItems({1, 3}, {"foo(int x)"})
        << "foo(int x)"
        << "void foo(int x);\nvoid foo(int x)\n{\n}\n";

    QTest::newRow("replace-leading-with-class-method")
        << "class Foo { int bar(int x); };\nint "
        << CompletionItems({1, 4}, {"Foo", "Foo::bar(int x)"})
        << "Foo::bar(int x)"
        << "class Foo { int bar(int x); };\nint Foo::bar(int x)\n{\n}\n";

    QTest::newRow("replace-leading-whitespace-mismatch")
        << "class Foo { int** bar(int x); };\nint * *"
        << CompletionItems({1, 7}, {"** Foo::bar(int x)"})
        << "** Foo::bar(int x)"
        << "class Foo { int** bar(int x); };\nint ** Foo::bar(int x)\n{\n}\n";
}

void TestCodeCompletion::testIgnoreGccBuiltins()
{
    // TODO: make it easier to change the compiler provider for testing purposes
    QTemporaryDir dir;
    auto project = new TestProject(Path(dir.path()), this);
    auto definesAndIncludesConfig = project->projectConfiguration()->group(definesAndIncludesConfigName);
    auto pathConfig = definesAndIncludesConfig.group("ProjectPath0");
    pathConfig.writeEntry("Path", ".");
    pathConfig.group("Compiler").writeEntry("Name", "GCC");
    m_projectController->addProject(project);

    {
        TestFile file(QString(), QStringLiteral("cpp"), project, dir.path());

        QVERIFY(file.parseAndWait(TopDUContext::AllDeclarationsContextsUsesAndAST));

        executeCompletionTest(file.topContext(), {});
    }
}

#include "moc_test_codecompletion.cpp"
