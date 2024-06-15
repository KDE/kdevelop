/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_duchain.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <tests/testproject.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/typealiastype.h>
#include <language/duchain/types/typeutils.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/use.h>
#include <language/duchain/duchaindumper.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <util/kdevstringhandler.h>

#include "duchain/clangparsingenvironmentfile.h"
#include "duchain/clangparsingenvironment.h"
#include "duchain/parsesession.h"
#include "duchain/clanghelpers.h"
#include "duchain/macrodefinition.h"

#include "testprovider.h"

#include <KConfigGroup>

#include <QTest>
#include <QSignalSpy>
#include <QLoggingCategory>
#include <QThread>
#include <QVector>
#include <QVersionNumber>

QTEST_MAIN(TestDUChain)

using namespace KDevelop;

namespace KDevelop {
char* toString(ClassMemberDeclaration::BitWidthSpecialValue bitWidth)
{
    return QTest::toString(QString::number(bitWidth));
}
}

TestDUChain::~TestDUChain() = default;

void TestDUChain::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));
    AutoTestShell::init({QStringLiteral("kdevclangsupport")});
    auto core = TestCore::initialize();
    delete core->projectController();
    m_projectController = new TestProjectController(core);
    core->setProjectController(m_projectController);
}

void TestDUChain::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDUChain::cleanup()
{
    if (m_provider) {
        IDefinesAndIncludesManager::manager()->unregisterBackgroundProvider(m_provider.data());
    }
    // process delayed events
    QTest::qWait(0);
}

void TestDUChain::init()
{
    m_provider.reset(new TestEnvironmentProvider);
    IDefinesAndIncludesManager::manager()->registerBackgroundProvider(m_provider.data());
}

struct ExpectedComment
{
    QString identifier;
    QString comment;
};
Q_DECLARE_METATYPE(ExpectedComment)
Q_DECLARE_METATYPE(AbstractType::WhichType)

void TestDUChain::testComments()
{
    QFETCH(QString, code);
    QFETCH(ExpectedComment, expectedComment);

    TestFile file(code, QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    auto candidates = top->findDeclarations(QualifiedIdentifier(expectedComment.identifier));
    QVERIFY(!candidates.isEmpty());
    auto decl = candidates.first();
    QString comment = QString::fromLocal8Bit(decl->comment());
    const auto plainText = KDevelop::htmlToPlainText(comment, KDevelop::CompleteMode);
    // if comment is e.g. "<this is bar", htmlToPlainText(comment) would just return an empty string; avoid this
    if (!plainText.isEmpty()) {
        comment = plainText;
    }
    QCOMPARE(comment, expectedComment.comment);
}

void TestDUChain::testComments_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<ExpectedComment>("expectedComment");

    // note: Clang only retrieves the comments when in doxygen-style format (i.e. '///', '/**', '///<')
    QTest::newRow("invalid1")
        << "//this is foo\nint foo;"
        << ExpectedComment{"foo", QString()};
    QTest::newRow("invalid2")
        << "/*this is foo*/\nint foo;"
        << ExpectedComment{"foo", QString()};
    QTest::newRow("basic1")
        << "///this is foo\nint foo;"
        << ExpectedComment{"foo", "this is foo"};
    QTest::newRow("basic2")
        << "/**this is foo*/\nint foo;"
        << ExpectedComment{"foo", "this is foo"};

    // as long as https://bugs.llvm.org/show_bug.cgi?id=35333 is not fixed, we don't fully parse and render
    // doxygen-style comments properly (cf. `makeComment` in builder.cpp)
#define PARSE_COMMENTS 0
    QTest::newRow("enumerator")
        << "enum Foo { bar1, ///<this is bar1\nbar2 ///<this is bar2\n };"
        << ExpectedComment{"Foo::bar1", "this is bar1"};

    QTest::newRow("comment-formatting")
        << "/** a\n * multiline\n *\n * comment\n */ int foo;"
#if PARSE_COMMENTS
        << ExpectedComment{"foo", "a multiline\ncomment"};
#else
        << ExpectedComment{"foo", "a multiline comment"};
#endif
    QTest::newRow("comment-doxygen-tags")
        << "/** @see bar()\n@param a foo\n*/\nvoid foo(int a);\nvoid bar();"
#if PARSE_COMMENTS
        << ExpectedComment{"foo", "bar()\na\nfoo"};
#else
        << ExpectedComment{"foo", "@see bar() @param a foo"};
#endif
}

void TestDUChain::testElaboratedType()
{
    QFETCH(QString, code);
    QFETCH(AbstractType::WhichType, type);

    TestFile file(code, QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);

    auto decl = file.topContext()->localDeclarations()[1];
    QVERIFY(decl);

    auto function = dynamic_cast<FunctionDeclaration*>(decl);
    QVERIFY(function);

    auto functionType = function->type<FunctionType>();
    QVERIFY(functionType);

#if CINDEX_VERSION_MINOR < 34
    QEXPECT_FAIL("namespace", "The ElaboratedType is not exposed through the libclang interface, not much we can do here", Abort);
#endif
    QVERIFY(functionType->returnType()->whichType() != AbstractType::TypeDelayed);
#if CINDEX_VERSION_MINOR < 34
    QEXPECT_FAIL("typedef", "After using clang_getCanonicalType on ElaboratedType all typedef information gets stripped away", Continue);
#endif
    QCOMPARE(functionType->returnType()->whichType(), type);
}

void TestDUChain::testElaboratedType_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<AbstractType::WhichType>("type");

    QTest::newRow("namespace")
        << "namespace NS{struct Type{};} struct NS::Type foo();"
        << AbstractType::TypeStructure;
    QTest::newRow("enum")
        << "enum Enum{}; enum Enum foo();"
        << AbstractType::TypeEnumeration;
    QTest::newRow("typedef")
        << "namespace NS{typedef int type;} NS::type foo();"
        << AbstractType::TypeAlias;
}

void TestDUChain::testMacroDefinition()
{
    QFETCH(QString, code);
    QFETCH(QString, definition);
    QFETCH(bool, isFunctionLike);
    QFETCH(QVector<QString>, parameters);

    TestFile file(code, QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    QCOMPARE(top->localDeclarations().size(), 1);

    const auto decl = top->localDeclarations().constFirst();
    QVERIFY(decl);

    const auto macro = dynamic_cast<const MacroDefinition*>(decl);
    QVERIFY(macro);

    QCOMPARE(macro->definition().str(), definition);

    QCOMPARE(macro->isFunctionLike(), isFunctionLike);

    // The displayed tooltip for these incorrectly parsed macros looks good, but the bogus
    // single macro parameter in the MacroDefinition object could cause issues in the future.
    QEXPECT_FAIL("only_escaped_newline_in_parens(\\\n)", "difficult to parse, uncommon, not yet a problem", Continue);
    QEXPECT_FAIL("only_comment_in_parens(/*comment*/)", "difficult to parse, uncommon, not yet a problem", Continue);
    QCOMPARE(macro->parametersSize(), parameters.size());

    const IndexedString* actualParam = macro->parameters();
    for (const auto& expectedParam : parameters) {
        QCOMPARE(actualParam->str(), expectedParam);
        ++actualParam;
    }
}

void TestDUChain::testMacroDefinition_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<QString>("definition");
    QTest::addColumn<bool>("isFunctionLike");
    QTest::addColumn<QVector<QString>>("parameters");

    const auto addTest = [](const QString& code, const QString& definition, bool isFunctionLike = false,
                            const QVector<QString>& parameters = {}) {
        QTest::addRow("%s", qPrintable(code))
            << QString{"#define " + code} << definition << isFunctionLike << parameters;
    };

    addTest("macro", "");
    addTest("m1 1", "1");
    addTest("m int x", "int x");
    addTest("m (u + v)", "(u + v)");
    addTest("m\tn", "n");
    addTest("m/*o*/long", "/*o*/long");
    addTest("m/*(x)*/ long", "/*(x)*/ long");

    addTest("m()", "", true);
    addTest("macro(    ) C", "C", true);

    addTest("mac(x)", "", true, {"x"});
    addTest("VARG_1(...)", "", true, {"..."});
    addTest("mac(x) x", "x", true, {"x"});
    addTest("mc(u)\tu *2 \t/  Limit", "u *2 \t/  Limit", true, {"u"});
    addTest("m(x)x", "x", true, {"x"});
    addTest("m(x)long x", "long x", true, {"x"});
    addTest("m(x)/*A*/x", "/*A*/x", true, {"x"});
    addTest("XYZ_(...) __VA_ARGS__", "__VA_ARGS__", true, {"..."});

    addTest("macro(may,be)", "", true, {"may", "be"});
    addTest("m(x, \ty)\tx", "x", true, {"x", "y"});
    addTest("m(x, y) x / y", "x / y", true, {"x", "y"});
    addTest("M_N(X, ...) f(X __VA_OPT__(,) __VA_ARGS__)", "f(X __VA_OPT__(,) __VA_ARGS__)", true, {"X", "..."});

    addTest("m( /*a)b*/ x )x", "x", true, {"/*a)b*/ x"});
    addTest("_(\t/* u,v,*/c\t)\tc/3*(c-5)", "c/3*(c-5)", true, {"/* u,v,*/c"});

    addTest("MM\t\\\n\t471", "\\\n\t471");
    addTest("S1\\\n get()", "get()");
    addTest("S1\\\nget(\t)", "", true);
    addTest("m_2\\  \t\n(){return;}", "{return;}", true);
    addTest("A0B9\\\n(N)\\\n(N-9)", "\\\n(N-9)", true, {"N"});
    addTest("\t     \\\t\nmacro\\ \n\\\nIden\\\ntifier(x,\\\t\n    y)x \t\ty", "x \t\ty", true, {"x", "\\\t\n    y"});

    addTest("only_escaped_newline_in_parens(\\\n)", "", true);
    addTest("only_comment_in_parens(/*comment*/)", "", true);
}

void TestDUChain::testInclude()
{
    TestFile header(QStringLiteral("int foo() { return 42; }\n"), QStringLiteral("h"));
    // NOTE: header is _not_ explicitly being parsed, instead the impl job does that

    TestFile impl("#include \"" + header.url().str() + "\"\n"
                  "int main() { return foo(); }", QStringLiteral("cpp"), &header);
    impl.parse(TopDUContext::AllDeclarationsContextsAndUses);

    auto implCtx = impl.topContext();
    QVERIFY(implCtx);

    DUChainReadLocker lock;
    QCOMPARE(implCtx->localDeclarations().size(), 1);

    auto headerCtx = DUChain::self()->chainForDocument(header.url());
    QVERIFY(headerCtx);
    QVERIFY(!headerCtx->parsingEnvironmentFile()->needsUpdate());
    QCOMPARE(headerCtx->localDeclarations().size(), 1);

    QVERIFY(implCtx->imports(headerCtx, CursorInRevision(0, 10)));

    Declaration* foo = headerCtx->localDeclarations().first();
    QCOMPARE(foo->uses().size(), 1);
    QCOMPARE(foo->uses().begin().key(), impl.url());
    QCOMPARE(foo->uses().begin()->size(), 1);
    QCOMPARE(foo->uses().begin()->first(), RangeInRevision(1, 20, 1, 23));
}

void TestDUChain::testMissingInclude()
{
    auto code = R"(
#pragma once
#include "missing1.h"

template<class T>
class A
{
    T a;
};

#include "missing2.h"

class B : public A<int>
{
};
    )";

    TestFile header(code, QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().str() + "\"\n", QStringLiteral("cpp"), &header);
    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses));

    DUChainReadLocker lock;

    auto top = impl.topContext();
    QVERIFY(top);

    QCOMPARE(top->importedParentContexts().count(), 1);

    TopDUContext* headerCtx = dynamic_cast<TopDUContext*>(top->importedParentContexts().first().context(top));
    QVERIFY(headerCtx);
    QCOMPARE(headerCtx->url(), header.url());

#if CINDEX_VERSION_MINOR < 34
    QEXPECT_FAIL("", "Second missing header isn't reported", Continue);
#endif
    QCOMPARE(headerCtx->problems().count(), 2);

    QCOMPARE(headerCtx->localDeclarations().count(), 2);

    auto a = dynamic_cast<ClassDeclaration*>(headerCtx->localDeclarations().first());
    QVERIFY(a);

    auto b = dynamic_cast<ClassDeclaration*>(headerCtx->localDeclarations().last());
    QVERIFY(b);

    // NOTE: This fails and needs fixing. If the include of "missing2.h"
    //       above is commented out, then it doesn't fail. Maybe
    //       clang stops processing when it encounters the second missing
    //       header, or similar.
    // XFAIL this check until https://bugs.llvm.org/show_bug.cgi?id=38155 is fixed
    if (QVersionNumber::fromString(ClangHelpers::clangVersion()) < QVersionNumber(9, 0, 0))
        QEXPECT_FAIL("", "Base class isn't assigned correctly", Continue);
    QCOMPARE(b->baseClassesSize(), 1u);

#if CINDEX_VERSION_MINOR < 34
    // at least the one problem we have should have been propagated
    QCOMPARE(top->problems().count(), 1);
#else
    // two errors:
    // /tmp/testfile_f32415.h:3:10: error: 'missing1.h' file not found
    // /tmp/testfile_f32415.h:11:10: error: 'missing2.h' file not found
    QCOMPARE(top->problems().count(), 2);
#endif
}

QByteArray createCode(const QByteArray& prefix, const int functions)
{
    QByteArray code;
    code += "#ifndef " + prefix + "_H\n";
    code += "#define " + prefix + "_H\n";
    for (int i = 0; i < functions; ++i) {
        code += "void myFunc_" + prefix + "(int arg1, char arg2, const char* arg3);\n";
    }
    code += "#endif\n";
    return code;
}

void TestDUChain::testIncludeLocking()
{
    TestFile header1(createCode("Header1", 1000), QStringLiteral("h"));
    TestFile header2(createCode("Header2", 1000), QStringLiteral("h"));
    TestFile header3(createCode("Header3", 1000), QStringLiteral("h"));

    ICore::self()->languageController()->backgroundParser()->setThreadCount(3);

    TestFile impl1("#include \"" + header1.url().str() + "\"\n"
                   "#include \"" + header2.url().str() + "\"\n"
                   "#include \"" + header3.url().str() + "\"\n"
                   "int main() { return 0; }", QStringLiteral("cpp"));

    TestFile impl2("#include \"" + header2.url().str() + "\"\n"
                   "#include \"" + header1.url().str() + "\"\n"
                   "#include \"" + header3.url().str() + "\"\n"
                   "int main() { return 0; }", QStringLiteral("cpp"));

    TestFile impl3("#include \"" + header3.url().str() + "\"\n"
                   "#include \"" + header1.url().str() + "\"\n"
                   "#include \"" + header2.url().str() + "\"\n"
                   "int main() { return 0; }", QStringLiteral("cpp"));

    impl1.parse(TopDUContext::AllDeclarationsContextsAndUses);
    impl2.parse(TopDUContext::AllDeclarationsContextsAndUses);
    impl3.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(impl1.waitForParsed(5000));
    QVERIFY(impl2.waitForParsed(5000));
    QVERIFY(impl3.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(DUChain::self()->chainForDocument(header1.url()));
    QVERIFY(DUChain::self()->chainForDocument(header2.url()));
    QVERIFY(DUChain::self()->chainForDocument(header3.url()));
}

void TestDUChain::testReparse()
{
    TestFile file(QStringLiteral("int main() { int i = 42; return i; }"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    DeclarationPointer mainDecl;
    DeclarationPointer iDecl;
    for (int i = 0; i < 3; ++i) {
        QVERIFY(file.waitForParsed(500));
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->childContexts().size(), 1);
        QCOMPARE(file.topContext()->localDeclarations().size(), 1);
        DUContext *exprContext = file.topContext()->childContexts().first()->childContexts().first();
        QCOMPARE(exprContext->localDeclarations().size(), 1);

        if (i) {
            QVERIFY(mainDecl);
            QCOMPARE(mainDecl.data(), file.topContext()->localDeclarations().first());

            QVERIFY(iDecl);
            QCOMPARE(iDecl.data(), exprContext->localDeclarations().first());
        }
        mainDecl = file.topContext()->localDeclarations().first();
        iDecl = exprContext->localDeclarations().first();

        QVERIFY(mainDecl->uses().isEmpty());
        QCOMPARE(iDecl->uses().size(), 1);
        QCOMPARE(iDecl->uses().begin()->size(), 1);

        if (i == 1) {
            file.setFileContents(QStringLiteral("int main()\n{\nfloat i = 13; return i - 5;\n}\n"));
        }

        file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive);
    }
}

void TestDUChain::testReparseError()
{
    TestFile file(QStringLiteral("int i = 1 / 0;\n"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(file.waitForParsed(500));
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        if (!i) {
            QCOMPARE(file.topContext()->problems().size(), 1);
            file.setFileContents(QStringLiteral("int i = 0;\n"));
        } else {
            QCOMPARE(file.topContext()->problems().size(), 0);
        }

        file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive);
    }
}

void TestDUChain::testTemplate()
{
    TestFile file("template<typename T> struct foo { T bar; };\n"
                  "int main() { foo<int> myFoo; return myFoo.bar; }\n", QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto fooDecl = file.topContext()->localDeclarations().first();
    QVERIFY(fooDecl->internalContext());
    QCOMPARE(fooDecl->internalContext()->localDeclarations().size(), 2);

    QCOMPARE(file.topContext()->findDeclarations(QualifiedIdentifier(u"foo< T >")).size(), 1);
    QCOMPARE(file.topContext()->findDeclarations(QualifiedIdentifier(u"foo< T >::bar")).size(), 1);

    auto mainCtx = file.topContext()->localDeclarations().last()->internalContext()->childContexts().first();
    QVERIFY(mainCtx);
    auto myFoo = mainCtx->localDeclarations().first();
    QVERIFY(myFoo);
    QCOMPARE(myFoo->abstractType()->toString().remove(' '), QStringLiteral("foo<int>"));
}

void TestDUChain::testNamespace()
{
    TestFile file("namespace foo { struct bar { int baz; }; }\n"
                  "int main() { foo::bar myBar; }\n", QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto fooDecl = file.topContext()->localDeclarations().first();
    QVERIFY(fooDecl->internalContext());
    QCOMPARE(fooDecl->internalContext()->localDeclarations().size(), 1);

    DUContext* top = file.topContext().data();
    DUContext* mainCtx = file.topContext()->childContexts().last();

    auto foo = top->localDeclarations().first();
    QCOMPARE(foo->qualifiedIdentifier().toString(), QString("foo"));

    DUContext* fooCtx = file.topContext()->childContexts().first();
    QCOMPARE(fooCtx->localScopeIdentifier().toString(), QString("foo"));
    QCOMPARE(fooCtx->scopeIdentifier(true).toString(), QString("foo"));
    QCOMPARE(fooCtx->localDeclarations().size(), 1);
    auto bar = fooCtx->localDeclarations().first();
    QCOMPARE(bar->qualifiedIdentifier().toString(), QString("foo::bar"));
    QCOMPARE(fooCtx->childContexts().size(), 1);

    DUContext* barCtx = fooCtx->childContexts().first();
    QCOMPARE(barCtx->localScopeIdentifier().toString(), QString("bar"));
    QCOMPARE(barCtx->scopeIdentifier(true).toString(), QString("foo::bar"));
    QCOMPARE(barCtx->localDeclarations().size(), 1);
    auto baz = barCtx->localDeclarations().first();
    QCOMPARE(baz->qualifiedIdentifier().toString(), QString("foo::bar::baz"));

    for (auto ctx : {top, mainCtx}) {
        QCOMPARE(ctx->findDeclarations(QualifiedIdentifier(u"foo")).size(), 1);
        QCOMPARE(ctx->findDeclarations(QualifiedIdentifier(u"foo::bar")).size(), 1);
        QCOMPARE(ctx->findDeclarations(QualifiedIdentifier(u"foo::bar::baz")).size(), 1);
    }
}

void TestDUChain::testAutoTypeDeduction()
{
    TestFile file(QStringLiteral(R"(
        const volatile auto foo = 5;
        template<class T> struct myTemplate {};
        myTemplate<myTemplate<int>& > templRefParam;
        auto autoTemplRefParam = templRefParam;
    )"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;

    DUContext* ctx = file.topContext().data();
    QVERIFY(ctx);
    QCOMPARE(ctx->localDeclarations().size(), 4);
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier(u"foo")).size(), 1);
    Declaration* decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("foo")))[0];
    QCOMPARE(decl->identifier(), Identifier(u"foo"));
#if CINDEX_VERSION_MINOR < 31
    QEXPECT_FAIL("", "No type deduction here unfortunately, missing API in Clang", Continue);
#endif
    QVERIFY(decl->type<IntegralType>());
#if CINDEX_VERSION_MINOR < 31
    QCOMPARE(decl->toString(), QStringLiteral("const volatile auto foo"));
#else
    QCOMPARE(decl->toString(), QStringLiteral("const volatile int foo"));
#endif

    decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("autoTemplRefParam")))[0];
    QVERIFY(decl);
    QVERIFY(decl->abstractType());
#if CINDEX_VERSION_MINOR < 31
    QEXPECT_FAIL("", "Auto type is not exposed via LibClang", Continue);
#endif
    QCOMPARE(decl->abstractType()->toString(), QStringLiteral("myTemplate< myTemplate< int >& >"));
}

void TestDUChain::testTypeDeductionInTemplateInstantiation()
{
    // see: http://clang-developers.42468.n3.nabble.com/RFC-missing-libclang-query-functions-features-td2504253.html
    TestFile file(QStringLiteral("template<typename T> struct foo { T member; } foo<int> f; auto i = f.member;"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;

    DUContext* ctx = file.topContext().data();
    QVERIFY(ctx);
    QCOMPARE(ctx->localDeclarations().size(), 3);
    Declaration* decl = nullptr;

    // check 'foo' declaration
    decl = ctx->localDeclarations()[0];
    QVERIFY(decl);
    QCOMPARE(decl->identifier(), Identifier(u"foo< T >"));

    // check type of 'member' inside declaration-scope
    QCOMPARE(ctx->childContexts().size(), 1);
    DUContext* fooCtx = ctx->childContexts().first();
    QVERIFY(fooCtx);
    // Should there really be two declarations?
    QCOMPARE(fooCtx->localDeclarations().size(), 2);
    decl = fooCtx->localDeclarations()[1];
    QCOMPARE(decl->identifier(), Identifier(u"member"));

    // check type of 'member' in definition of 'f'
    decl = ctx->localDeclarations()[1];
    QCOMPARE(decl->identifier(), Identifier(u"f"));
    decl = ctx->localDeclarations()[2];
    QCOMPARE(decl->identifier(), Identifier(u"i"));
#if CINDEX_VERSION_MINOR < 31
    QEXPECT_FAIL("", "No type deduction here unfortunately, missing API in Clang", Continue);
#endif
    QVERIFY(decl->type<IntegralType>());
}

void TestDUChain::testVirtualMemberFunction()
{
    //Forward-declarations with "struct" or "class" are considered equal, so make sure the override is detected correctly.
    TestFile file(QStringLiteral("struct S {}; struct A { virtual S* ret(); }; struct B : public A { virtual S* ret(); };"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    DUContext* top = file.topContext().data();
    QVERIFY(top);

    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->localDeclarations().count(), 3);
    QCOMPARE(top->childContexts()[2]->localDeclarations().count(), 1);
    Declaration* decl = top->childContexts()[2]->localDeclarations()[0];
    QCOMPARE(decl->identifier(), Identifier(u"ret"));
    QVERIFY(DUChainUtils::overridden(decl));
}

void TestDUChain::testBaseClasses()
{
    TestFile file(QStringLiteral("class Base {}; class Inherited : public Base {};"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    DUContext* top = file.topContext().data();
    QVERIFY(top);

    QCOMPARE(top->localDeclarations().count(), 2);
    Declaration* baseDecl = top->localDeclarations().first();
    QCOMPARE(baseDecl->identifier(), Identifier(u"Base"));

    ClassDeclaration* inheritedDecl = dynamic_cast<ClassDeclaration*>(top->localDeclarations()[1]);
    QCOMPARE(inheritedDecl->identifier(), Identifier(u"Inherited"));

    QVERIFY(inheritedDecl);
    QCOMPARE(inheritedDecl->baseClassesSize(), 1u);

    QCOMPARE(baseDecl->uses().count(), 1);
    QCOMPARE(baseDecl->uses().first().count(), 1);
    QCOMPARE(baseDecl->uses().first().first(), RangeInRevision(0, 40, 0, 44));
}

void TestDUChain::testReparseBaseClasses()
{
    TestFile file(QStringLiteral("struct a{}; struct b : a {};\n"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    for (int i = 0; i < 2; ++i) {
        qDebug() << "run: " << i;
        QVERIFY(file.waitForParsed(500));
        DUChainWriteLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->childContexts().size(), 2);
        QCOMPARE(file.topContext()->childContexts().first()->importers().size(), 1);
        QCOMPARE(file.topContext()->childContexts().last()->importedParentContexts().size(), 1);

        QCOMPARE(file.topContext()->localDeclarations().size(), 2);
        auto aDecl = dynamic_cast<ClassDeclaration*>(file.topContext()->localDeclarations().first());
        QVERIFY(aDecl);
        QCOMPARE(aDecl->baseClassesSize(), 0u);
        auto bDecl = dynamic_cast<ClassDeclaration*>(file.topContext()->localDeclarations().last());
        QVERIFY(bDecl);
        QCOMPARE(bDecl->baseClassesSize(), 1u);
        int distance = 0;
        QVERIFY(bDecl->isPublicBaseClass(aDecl, file.topContext(), &distance));
        QCOMPARE(distance, 1);

        file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive);
    }
}

void TestDUChain::testReparseBaseClassesTemplates()
{
    TestFile file(QStringLiteral("template<typename T> struct a{}; struct b : a<int> {};\n"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    for (int i = 0; i < 2; ++i) {
        qDebug() << "run: " << i;
        QVERIFY(file.waitForParsed(500));
        DUChainWriteLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->childContexts().size(), 2);
        QCOMPARE(file.topContext()->childContexts().first()->importers().size(), 1);
        QCOMPARE(file.topContext()->childContexts().last()->importedParentContexts().size(), 1);

        QCOMPARE(file.topContext()->localDeclarations().size(), 2);
        auto aDecl = dynamic_cast<ClassDeclaration*>(file.topContext()->localDeclarations().first());
        QVERIFY(aDecl);
        QCOMPARE(aDecl->baseClassesSize(), 0u);
        auto bDecl = dynamic_cast<ClassDeclaration*>(file.topContext()->localDeclarations().last());
        QVERIFY(bDecl);
        QCOMPARE(bDecl->baseClassesSize(), 1u);
        int distance = 0;
        QVERIFY(bDecl->isPublicBaseClass(aDecl, file.topContext(), &distance));
        QCOMPARE(distance, 1);

        file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive);
    }
}

void TestDUChain::testGetInheriters_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("inline") << "struct Base { struct Inner {}; }; struct Inherited : Base, Base::Inner {};";
    QTest::newRow("outline") << "struct Base { struct Inner; }; struct Base::Inner {}; struct Inherited : Base, Base::Inner {};";
}

void TestDUChain::testGetInheriters()
{
    QFETCH(QString, code);
    TestFile file(code, QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    QVERIFY(top->problems().isEmpty());

    QCOMPARE(top->localDeclarations().count(), 2);
    Declaration* baseDecl = top->localDeclarations().first();
    QCOMPARE(baseDecl->identifier(), Identifier(u"Base"));

    DUContext* baseCtx = baseDecl->internalContext();
    QVERIFY(baseCtx);
    QCOMPARE(baseCtx->localDeclarations().count(), 1);

    Declaration* innerDecl = baseCtx->localDeclarations().first();
    QCOMPARE(innerDecl->identifier(), Identifier(u"Inner"));
    if (auto forward = dynamic_cast<ForwardDeclaration*>(innerDecl)) {
        innerDecl = forward->resolve(top);
    }
    QVERIFY(dynamic_cast<ClassDeclaration*>(innerDecl));

    Declaration* inheritedDecl = top->localDeclarations().last();
    QVERIFY(inheritedDecl);
    QCOMPARE(inheritedDecl->identifier(), Identifier(u"Inherited"));

    uint maxAllowedSteps = uint(-1);
    auto baseInheriters = DUChainUtils::inheriters(baseDecl, maxAllowedSteps);
    QCOMPARE(baseInheriters, QList<Declaration*>() << inheritedDecl);

    maxAllowedSteps = uint(-1);
    auto innerInheriters = DUChainUtils::inheriters(innerDecl, maxAllowedSteps);
    QCOMPARE(innerInheriters, QList<Declaration*>() << inheritedDecl);

    maxAllowedSteps = uint(-1);
    auto inheritedInheriters = DUChainUtils::inheriters(inheritedDecl, maxAllowedSteps);
    QCOMPARE(inheritedInheriters.count(), 0);
}

void TestDUChain::testGlobalFunctionDeclaration()
{
    TestFile file(QStringLiteral("void foo(int arg1, char arg2);\n"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    file.waitForParsed();

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 1);
    QCOMPARE(file.topContext()->childContexts().size(), 1);
    QVERIFY(!file.topContext()->childContexts().first()->inSymbolTable());
}

void TestDUChain::testFunctionDefinitionVsDeclaration()
{
    TestFile file(QStringLiteral("void func(); void func() {}\n"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto funcDecl = file.topContext()->localDeclarations()[0];
    QVERIFY(!funcDecl->isDefinition());
    QVERIFY(!dynamic_cast<FunctionDefinition*>(funcDecl));
    auto funcDef = file.topContext()->localDeclarations()[1];
    QVERIFY(dynamic_cast<FunctionDefinition*>(funcDef));
    QVERIFY(funcDef->isDefinition());
}

void TestDUChain::testEnsureNoDoubleVisit()
{
    // On some language construct, we may up visiting the same cursor multiple times
    // Example: "struct SomeStruct {} s;"
    // decl: "SomeStruct SomeStruct " of kind StructDecl (2) in main.cpp@[(1,1),(1,17)]
    // decl: "struct SomeStruct s " of kind VarDecl (9) in main.cpp@[(1,1),(1,19)]
    // decl: "SomeStruct SomeStruct " of kind StructDecl (2) in main.cpp@[(1,1),(1,17)]
    //
    // => We end up visiting the StructDecl twice (or more)
    //    That's because we use clang_visitChildren not just on the translation unit cursor.
    //    Apparently just "recursing" vs. "visiting children explicitly"
    //    results in a different AST traversal

    TestFile file(QStringLiteral("struct SomeStruct {} s;\n"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);

    // there should only be one declaration for "SomeStruct"
    auto candidates = top->findDeclarations(QualifiedIdentifier(QStringLiteral("SomeStruct")));
    QCOMPARE(candidates.size(), 1);
}

void TestDUChain::testParsingEnvironment()
{
    const TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses;

    IndexedTopDUContext indexed;
    ClangParsingEnvironment lastEnv;
    {
        TestFile file(QStringLiteral("int main() {}\n"), QStringLiteral("cpp"));
        const auto astFeatures = features | TopDUContext::AST;
        file.parse(astFeatures);
        file.setKeepDUChainData(true);
        QVERIFY(file.waitForParsed());

        DUChainWriteLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
        auto sessionData = ParseSessionData::Ptr(dynamic_cast<ParseSessionData*>(top->ast().data()));
        lock.unlock();
        ParseSession session(sessionData);
        lock.lock();
        QVERIFY(session.data());
        QVERIFY(top);

        auto envFile = QExplicitlySharedDataPointer<ClangParsingEnvironmentFile>(
            dynamic_cast<ClangParsingEnvironmentFile*>(file.topContext()->parsingEnvironmentFile().data()));

        QCOMPARE(envFile->features(), astFeatures);
        QVERIFY(envFile->featuresSatisfied(astFeatures));
        QCOMPARE(envFile->environmentQuality(), ClangParsingEnvironment::Source);

        // if no environment is given, no update should be triggered
        QVERIFY(!envFile->needsUpdate());

        // same env should also not trigger a reparse
        ClangParsingEnvironment env = session.environment();
        QCOMPARE(env.quality(), ClangParsingEnvironment::Source);
        QVERIFY(!envFile->needsUpdate(&env));

        // but changing the environment should trigger an update
        env.addIncludes(Path::List() << Path(QStringLiteral("/foo/bar/baz")));
        QVERIFY(envFile->needsUpdate(&env));
        envFile->setEnvironment(env);
        QVERIFY(!envFile->needsUpdate(&env));

        // setting the environment quality higher should require an update
        env.setQuality(ClangParsingEnvironment::BuildSystem);
        QVERIFY(envFile->needsUpdate(&env));
        envFile->setEnvironment(env);
        QVERIFY(!envFile->needsUpdate(&env));

        // changing defines requires an update
        env.addDefines(QHash<QString, QString>{ { "foo", "bar" } });
        QVERIFY(envFile->needsUpdate(&env));

        // but only when changing the defines for the envFile's TU
        const auto barTU = IndexedString("bar.cpp");
        const auto oldTU = env.translationUnitUrl();
        env.setTranslationUnitUrl(barTU);
        QCOMPARE(env.translationUnitUrl(), barTU);
        QVERIFY(!envFile->needsUpdate(&env));
        env.setTranslationUnitUrl(oldTU);
        QVERIFY(envFile->needsUpdate(&env));

        // update it again
        envFile->setEnvironment(env);
        QVERIFY(!envFile->needsUpdate(&env));
        lastEnv = env;

        // now compare against a lower quality environment
        // in such a case, we do not want to trigger an update
        env.setQuality(ClangParsingEnvironment::Unknown);
        env.setTranslationUnitUrl(barTU);
        QVERIFY(!envFile->needsUpdate(&env));

        // even when the environment changes
        env.addIncludes(Path::List() << Path(QStringLiteral("/lalalala")));
        QVERIFY(!envFile->needsUpdate(&env));

        indexed = top->indexed();
    }

    DUChain::self()->storeToDisk();

    {
        DUChainWriteLocker lock;
        QVERIFY(!DUChain::self()->isInMemory(indexed.index()));
        QVERIFY(indexed.data());
        QVERIFY(DUChain::self()->environmentFileForDocument(indexed));
        auto envFile = QExplicitlySharedDataPointer<ClangParsingEnvironmentFile>(
            dynamic_cast<ClangParsingEnvironmentFile*>(DUChain::self()->environmentFileForDocument(indexed).data()));
        QVERIFY(envFile);

        QCOMPARE(envFile->features(), features);
        QVERIFY(envFile->featuresSatisfied(features));
        QVERIFY(!envFile->needsUpdate(&lastEnv));
        DUChain::self()->removeDocumentChain(indexed.data());
    }
}

void TestDUChain::testActiveDocumentHasASTAttached()
{
  const TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses;

    IndexedTopDUContext indexed;
    ClangParsingEnvironment lastEnv;
    {
        TestFile file(QStringLiteral("int main() {}\n"), QStringLiteral("cpp"));
        const auto astFeatures = features | TopDUContext::AST;
        file.parse(astFeatures);
        file.setKeepDUChainData(true);
        QVERIFY(file.waitForParsed());

        DUChainWriteLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
        auto sessionData = ParseSessionData::Ptr(dynamic_cast<ParseSessionData*>(top->ast().data()));
        lock.unlock();
        ParseSession session(sessionData);
        lock.lock();
        QVERIFY(session.data());
        QVERIFY(top);
        QVERIFY(top->ast());

        indexed = top->indexed();
    }

    DUChain::self()->storeToDisk();

    {
        DUChainWriteLocker lock;
        QVERIFY(!DUChain::self()->isInMemory(indexed.index()));
        QVERIFY(indexed.data());
    }

    QUrl url;
    {
        DUChainReadLocker lock;
        auto ctx = indexed.data();
        QVERIFY(ctx);
        QVERIFY(!ctx->ast());
        url = ctx->url().toUrl();
    }

    QVERIFY(!QFileInfo::exists(url.toLocalFile()));
    QFile file(url.toLocalFile());
    file.open(QIODevice::WriteOnly);
    Q_ASSERT(file.isOpen());

    auto document = ICore::self()->documentController()->openDocument(url);
    QVERIFY(document);
    ICore::self()->documentController()->activateDocument(document);

    QApplication::processEvents();
    ICore::self()->languageController()->backgroundParser()->parseDocuments();
    QThread::sleep(1);

    document->close(KDevelop::IDocument::Discard);
    {
        DUChainReadLocker lock;
        auto ctx = indexed.data();
        QVERIFY(ctx);
        QVERIFY(ctx->ast());
    }

    DUChainWriteLocker lock;
    DUChain::self()->removeDocumentChain(indexed.data());
}

void TestDUChain::testActiveDocumentsGetBestPriority()
{
    // note: this test would make more sense in kdevplatform, but we don't have a language plugin available there
    // (required for background parsing)
    // TODO: Create a fake-language plugin in kdevplatform for testing purposes, use that.

    TestFile file1(QStringLiteral("int main() {}\n"), QStringLiteral("cpp"));
    TestFile file2(QStringLiteral("int main() {}\n"), QStringLiteral("cpp"));
    TestFile file3(QStringLiteral("int main() {}\n"), QStringLiteral("cpp"));

    DUChain::self()->storeToDisk();

    auto backgroundParser = ICore::self()->languageController()->backgroundParser();
    QVERIFY(!backgroundParser->isQueued(file1.url()));

    auto documentController = ICore::self()->documentController();

    // open first document (no activation)
    auto doc = documentController->openDocument(file1.url().toUrl(), KTextEditor::Range::invalid(), {IDocumentController::DoNotActivate});
    QVERIFY(doc);
    QVERIFY(backgroundParser->isQueued(file1.url()));
    QCOMPARE(backgroundParser->priorityForDocument(file1.url()), (int)BackgroundParser::NormalPriority);

    // open second document, activate
    doc = documentController->openDocument(file2.url().toUrl());
    QVERIFY(doc);
    QVERIFY(backgroundParser->isQueued(file2.url()));
    QCOMPARE(backgroundParser->priorityForDocument(file2.url()), (int)BackgroundParser::BestPriority);

    // open third document, activate, too
    doc = documentController->openDocument(file3.url().toUrl());
    QVERIFY(doc);
    QVERIFY(backgroundParser->isQueued(file3.url()));
    QCOMPARE(backgroundParser->priorityForDocument(file3.url()), (int)BackgroundParser::BestPriority);
}

void TestDUChain::testSystemIncludes()
{
    ClangParsingEnvironment env;

    Path::List projectIncludes = {
        Path("/projects/1"),
        Path("/projects/1/sub"),
        Path("/projects/2"),
        Path("/projects/2/sub")
    };
    env.addIncludes(projectIncludes);
    auto includes = env.includes();
    // no project paths set, so everything is considered a system include
    QCOMPARE(includes.system, projectIncludes);
    QVERIFY(includes.project.isEmpty());

    Path::List systemIncludes = {
        Path("/sys"),
        Path("/sys/sub")
    };
    env.addIncludes(systemIncludes);
    includes = env.includes();
    QCOMPARE(includes.system, projectIncludes + systemIncludes);
    QVERIFY(includes.project.isEmpty());

    Path::List projects = {
        Path("/projects/1"),
        Path("/projects/2")
    };
    env.setProjectPaths(projects);
    // now the list should be properly separated
    QCOMPARE(env.projectPaths(), projects);
    includes = env.includes();
    QCOMPARE(includes.system, systemIncludes);
    QCOMPARE(includes.project, projectIncludes);
}

void TestDUChain::testReparseWithAllDeclarationsContextsAndUses()
{
    TestFile file(QStringLiteral("int foo() { return 0; } int main() { return foo(); }"), QStringLiteral("cpp"));
    file.parse(TopDUContext::VisibleDeclarationsAndContexts);

    QVERIFY(file.waitForParsed(1000));

    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->childContexts().size(), 2);
        QCOMPARE(file.topContext()->localDeclarations().size(), 2);

        auto dec = file.topContext()->localDeclarations().at(0);
        QEXPECT_FAIL("", "Skipping of function bodies is disabled for now", Continue);
        QVERIFY(dec->uses().isEmpty());
    }

    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(file.waitForParsed(500));

    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->childContexts().size(), 2);
        QCOMPARE(file.topContext()->localDeclarations().size(), 2);

        auto mainDecl = file.topContext()->localDeclarations()[1];
        QVERIFY(mainDecl->uses().isEmpty());
        auto foo = file.topContext()->localDeclarations().first();
        QCOMPARE(foo->uses().size(), 1);
    }
}

void TestDUChain::testReparseOnDocumentActivated()
{
    TestFile file(QStringLiteral("int foo() { return 0; } int main() { return foo(); }"), QStringLiteral("cpp"));
    file.parse(TopDUContext::VisibleDeclarationsAndContexts);

    QVERIFY(file.waitForParsed(1000));

    {
        DUChainReadLocker lock;
        auto ctx = file.topContext();
        QVERIFY(ctx);
        QCOMPARE(ctx->childContexts().size(), 2);
        QCOMPARE(ctx->localDeclarations().size(), 2);

        auto dec = ctx->localDeclarations().at(0);
        QEXPECT_FAIL("", "Skipping of function bodies was disabled for now", Continue);
        QVERIFY(dec->uses().isEmpty());

        QVERIFY(!ctx->ast());
    }

    auto backgroundParser = ICore::self()->languageController()->backgroundParser();
    QVERIFY(!backgroundParser->isQueued(file.url()));

    auto doc = ICore::self()->documentController()->openDocument(file.url().toUrl());
    QVERIFY(doc);
    QVERIFY(backgroundParser->isQueued(file.url()));

    QSignalSpy spy(backgroundParser, &BackgroundParser::parseJobFinished);
    spy.wait();

    doc->close(KDevelop::IDocument::Discard);

    {
        DUChainReadLocker lock;
        auto ctx = file.topContext();
        QCOMPARE(ctx->features() & TopDUContext::AllDeclarationsContextsAndUses,
                 TopDUContext::AllDeclarationsContextsAndUses);
        QVERIFY(ctx->topContext()->ast());
    }
}

void TestDUChain::testReparseInclude()
{
    TestFile header(QStringLiteral("int foo() { return 42; }\n"), QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().str() + "\"\n"
                  "int main() { return foo(); }", QStringLiteral("cpp"), &header);

    // Use TopDUContext::AST to imitate that document is opened in the editor, so that ClangParseJob can store translation unit, that'll be used for reparsing.
    impl.parse(TopDUContext::AllDeclarationsAndContexts | TopDUContext::AST);
    QVERIFY(impl.waitForParsed(5000));
    {
        DUChainReadLocker lock;
        auto implCtx = impl.topContext();
        QVERIFY(implCtx);
        QCOMPARE(implCtx->importedParentContexts().size(), 1);
    }

    impl.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST);
    QVERIFY(impl.waitForParsed(5000));

    DUChainReadLocker lock;
    auto implCtx = impl.topContext();
    QVERIFY(implCtx);
    QCOMPARE(implCtx->localDeclarations().size(), 1);

    QCOMPARE(implCtx->importedParentContexts().size(), 1);

    auto headerCtx = DUChain::self()->chainForDocument(header.url());
    QVERIFY(headerCtx);
    QVERIFY(!headerCtx->parsingEnvironmentFile()->needsUpdate());
    QCOMPARE(headerCtx->localDeclarations().size(), 1);

    QVERIFY(implCtx->imports(headerCtx, CursorInRevision(0, 10)));

    Declaration* foo = headerCtx->localDeclarations().first();
    QCOMPARE(foo->uses().size(), 1);
    QCOMPARE(foo->uses().begin().key(), impl.url());
    QCOMPARE(foo->uses().begin()->size(), 1);
    QCOMPARE(foo->uses().begin()->first(), RangeInRevision(1, 20, 1, 23));

    QCOMPARE(DUChain::self()->allEnvironmentFiles(header.url()).size(), 1);
    QCOMPARE(DUChain::self()->allEnvironmentFiles(impl.url()).size(), 1);
    QCOMPARE(DUChain::self()->chainsForDocument(header.url()).size(), 1);
    QCOMPARE(DUChain::self()->chainsForDocument(impl.url()).size(), 1);
}

void TestDUChain::testReparseChangeEnvironment()
{
    TestFile header(QStringLiteral("int foo() { return 42; }\n"), QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().str() + "\"\n"
                  "int main() { return foo(); }", QStringLiteral("cpp"), &header);

    size_t hashes[3] = {0, 0, 0};

    for (int i = 0; i < 3; ++i) {
        impl.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);
        QVERIFY(impl.waitForParsed(5000));

        {
            DUChainReadLocker lock;
            QVERIFY(impl.topContext());
            auto env = dynamic_cast<ClangParsingEnvironmentFile*>(impl.topContext()->parsingEnvironmentFile().data());
            QVERIFY(env);
            QCOMPARE(env->environmentQuality(), ClangParsingEnvironment::Source);
            hashes[i] = env->environmentHash();
            QVERIFY(hashes[i]);

            // we should never end up with multiple env files or chains in memory for these files
            QCOMPARE(DUChain::self()->allEnvironmentFiles(impl.url()).size(), 1);
            QCOMPARE(DUChain::self()->chainsForDocument(impl.url()).size(), 1);
            QCOMPARE(DUChain::self()->allEnvironmentFiles(header.url()).size(), 1);
            QCOMPARE(DUChain::self()->chainsForDocument(header.url()).size(), 1);
        }

        // in every run, we expect the environment to have changed
        for (int j = 0; j < i; ++j) {
            QVERIFY(hashes[i] != hashes[j]);
        }

        if (i == 0) {
            // 1) change defines
            m_provider->defines.insert(QStringLiteral("foooooooo"), QStringLiteral("baaar!"));
        } else if (i == 1) {
            // 2) change includes
            m_provider->includes.append(Path(QStringLiteral("/foo/bar/asdf/lalala")));
        } // 3) stop
    }
}

void TestDUChain::testMacroDependentHeader()
{
    TestFile header(QStringLiteral("struct MY_CLASS { struct Q{Q(); int m;}; int m; };\n"), QStringLiteral("h"));
    TestFile impl("#define MY_CLASS A\n"
                  "#include \"" + header.url().str() + "\"\n"
                  "#undef MY_CLASS\n"
                  "#define MY_CLASS B\n"
                  "#include \"" + header.url().str() + "\"\n"
                  "#undef MY_CLASS\n"
                  "A a;\n"
                  "const A::Q aq;\n"
                  "B b;\n"
                  "const B::Q bq;\n"
                  "int am = a.m;\n"
                  "int aqm = aq.m;\n"
                  "int bm = b.m;\n"
                  "int bqm = bq.m;\n"
                  , QStringLiteral("cpp"), &header);

    impl.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);
    QVERIFY(impl.waitForParsed(500000));

    DUChainReadLocker lock;
    TopDUContext* top = impl.topContext().data();
    QVERIFY(top);
    QCOMPARE(top->localDeclarations().size(), 10); // 2x macro, then a, aq, b, bq
    QCOMPARE(top->importedParentContexts().size(), 1);
    AbstractType::Ptr type = top->localDeclarations()[2]->abstractType();
    auto* sType = dynamic_cast<StructureType*>(type.data());
    QVERIFY(sType);
    QCOMPARE(sType->toString(), QString("A"));
    Declaration* decl = sType->declaration(top);
    QVERIFY(decl);
    AbstractType::Ptr type2 = top->localDeclarations()[4]->abstractType();
    auto* sType2 = dynamic_cast<StructureType*>(type2.data());
    QVERIFY(sType2);
    QCOMPARE(sType2->toString(), QString("B"));
    Declaration* decl2 = sType2->declaration(top);
    QVERIFY(decl2);

    TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].context(top));
    QVERIFY(top2);
    QCOMPARE(top2->localDeclarations().size(), 2);
    QCOMPARE(top2->localDeclarations()[0], decl);
    QCOMPARE(top2->localDeclarations()[1], decl2);
    qDebug() << "DECL RANGE:" << top2->localDeclarations()[0]->range().castToSimpleRange();
    qDebug() << "CTX RANGE:" << top2->localDeclarations()[0]->internalContext()->range().castToSimpleRange();

    // validate uses:
    QCOMPARE(top->usesCount(), 14);
    QCOMPARE(top->uses()[0].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"A"));
    QCOMPARE(top->uses()[1].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"A"));
    QCOMPARE(top->uses()[2].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"A::Q"));
    QCOMPARE(top->uses()[3].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"B"));
    QCOMPARE(top->uses()[4].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"B"));
    QCOMPARE(top->uses()[5].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"B::Q"));
    QCOMPARE(top->uses()[6].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"a"));
    QCOMPARE(top->uses()[7].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"A::m"));
    QCOMPARE(top->uses()[8].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"aq"));
    QCOMPARE(top->uses()[9].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"A::Q::m"));
    QCOMPARE(top->uses()[10].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"b"));
    QCOMPARE(top->uses()[11].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"B::m"));
    QCOMPARE(top->uses()[12].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"bq"));
    QCOMPARE(top->uses()[13].usedDeclaration(top)->qualifiedIdentifier(), QualifiedIdentifier(u"B::Q::m"));
}

void TestDUChain::testHeaderParsingOrder1()
{
    TestFile header(QStringLiteral("typedef const A<int> B;\n"), QStringLiteral("h"));
    TestFile impl("template<class T> class A{};\n"
                  "#include \"" + header.url().str() + "\"\n"
                  "B c;", QStringLiteral("cpp"), &header);

    impl.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);
    QVERIFY(impl.waitForParsed(500000));

    DUChainReadLocker lock;
    TopDUContext* top = impl.topContext().data();
    QVERIFY(top);
    QCOMPARE(top->localDeclarations().size(), 2);
    QCOMPARE(top->importedParentContexts().size(), 1);
    AbstractType::Ptr type = top->localDeclarations()[1]->abstractType();
    auto* aType = dynamic_cast<TypeAliasType*>(type.data());
    QVERIFY(aType);
    AbstractType::Ptr targetType = aType->type();
    QVERIFY(targetType);
    auto *idType = dynamic_cast<IdentifiedType*>(targetType.data());
    QVERIFY(idType);
    // this declaration could be resolved, because it was created with an
    // indirect DeclarationId that is resolved from the perspective of 'top'
    Declaration* decl = idType->declaration(top);
    // NOTE: the decl. doesn't know (yet) about the template insantiation <int>
    QVERIFY(decl);
    QCOMPARE(decl, top->localDeclarations()[0]);

    // now ensure that a use was build for 'A' in header1
    TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[0].context(top));
    QVERIFY(top2);
    QEXPECT_FAIL("", "the use could not be created because the corresponding declaration didn't exist yet", Continue);
    QCOMPARE(top2->usesCount(), 1);
    // Declaration* decl2 = top2->uses()[0].usedDeclaration(top2);
    // QVERIFY(decl2);
    // QCOMPARE(decl, decl2);
}

void TestDUChain::testHeaderParsingOrder2()
{
    TestFile header(QStringLiteral("template<class T> class A{};\n"), QStringLiteral("h"));
    TestFile header2(QStringLiteral("typedef const A<int> B;\n"), QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().str() + "\"\n"
                  "#include \"" + header2.url().str() + "\"\n"
                  "B c;", QStringLiteral("cpp"), &header);

    impl.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);
    QVERIFY(impl.waitForParsed(500000));

    DUChainReadLocker lock;
    TopDUContext* top = impl.topContext().data();
    QVERIFY(top);
    QCOMPARE(top->localDeclarations().size(), 1);
    QCOMPARE(top->importedParentContexts().size(), 2);
    AbstractType::Ptr type = top->localDeclarations()[0]->abstractType();
    auto* aType = dynamic_cast<TypeAliasType*>(type.data());
    QVERIFY(aType);
    AbstractType::Ptr targetType = aType->type();
    QVERIFY(targetType);
    auto *idType = dynamic_cast<IdentifiedType*>(targetType.data());
    QVERIFY(idType);
    Declaration* decl = idType->declaration(top);
    // NOTE: the decl. doesn't know (yet) about the template insantiation <int>
    QVERIFY(decl);

    // now ensure that a use was build for 'A' in header2
    TopDUContext* top2 = dynamic_cast<TopDUContext*>(top->importedParentContexts()[1].context(top));
    QVERIFY(top2);
    QCOMPARE(top2->usesCount(), 1);
    Declaration* decl2 = top2->uses()[0].usedDeclaration(top2);
    QCOMPARE(decl, decl2);
}

void TestDUChain::testMacrosRanges()
{
    TestFile file(QStringLiteral("#define FUNC_MACROS(x) struct str##x{};\nFUNC_MACROS(x);"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto macroDefinition = file.topContext()->localDeclarations()[0];
    QVERIFY(macroDefinition);
    QCOMPARE(macroDefinition->range(), RangeInRevision(0,8,0,19));
    auto structDeclaration = file.topContext()->localDeclarations()[1];
    QVERIFY(structDeclaration);
    QCOMPARE(structDeclaration->range(), RangeInRevision(1,0,1,0));

    QCOMPARE(macroDefinition->uses().size(), 1);
    QCOMPARE(macroDefinition->uses().begin()->first(), RangeInRevision(1,0,1,11));
}

void TestDUChain::testMacroUses()
{
    TestFile file(QStringLiteral("#define USER(x) x\n#define USED\nUSER(USED)"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto macroDefinition1 = file.topContext()->localDeclarations()[0];
    auto macroDefinition2 = file.topContext()->localDeclarations()[1];

    QCOMPARE(macroDefinition1->uses().size(), 1);
    QCOMPARE(macroDefinition1->uses().begin()->first(), RangeInRevision(2,0,2,4));
#if CINDEX_VERSION_MINOR < 32
    QEXPECT_FAIL("", "This appears to be a clang bug, the AST doesn't contain the macro use", Continue);
#endif
    QCOMPARE(macroDefinition2->uses().size(), 1);
    if (macroDefinition2->uses().size())
    {
        QCOMPARE(macroDefinition2->uses().begin()->first(), RangeInRevision(2,5,2,9));
    }
}

void TestDUChain::testMultiLineMacroRanges()
{
    TestFile file(QStringLiteral("#define FUNC_MACROS(x) struct str##x{};\nFUNC_MACROS(x\n);"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto macroDefinition = file.topContext()->localDeclarations()[0];
    QVERIFY(macroDefinition);
    QCOMPARE(macroDefinition->range(), RangeInRevision(0,8,0,19));
    auto structDeclaration = file.topContext()->localDeclarations()[1];
    QVERIFY(structDeclaration);
    QCOMPARE(structDeclaration->range(), RangeInRevision(1,0,1,0));

    QCOMPARE(macroDefinition->uses().size(), 1);
    QCOMPARE(macroDefinition->uses().begin()->first(), RangeInRevision(1,0,1,11));
}

void TestDUChain::testNestedMacroRanges()
{
    TestFile file(QStringLiteral("#define INNER int var; var = 0;\n#define MACRO() INNER\nint main(){MACRO(\n);}"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 3);
    auto main = file.topContext()->localDeclarations()[2];
    QVERIFY(main);
    auto mainCtx = main->internalContext()->childContexts().first();
    QVERIFY(mainCtx);
    QCOMPARE(mainCtx->localDeclarations().size(), 1);
    auto var = mainCtx->localDeclarations().first();
    QVERIFY(var);
    QCOMPARE(var->range(), RangeInRevision(2,11,2,11));

    QCOMPARE(var->uses().size(), 1);
    QCOMPARE(var->uses().begin()->first(), RangeInRevision(2,11,2,11));
}

void TestDUChain::testNestedImports()
{
    TestFile B(QStringLiteral("#pragma once\nint B();\n"), QStringLiteral("h"));
    TestFile C("#pragma once\n#include \"" + B.url().str() + "\"\nint C();\n", QStringLiteral("h"));
    TestFile A("#include \"" + B.url().str() + "\"\n" + "#include \"" + C.url().str() + "\"\nint A();\n", QStringLiteral("cpp"));

    A.parse();
    QVERIFY(A.waitForParsed(5000));

    DUChainReadLocker lock;

    auto BCtx = DUChain::self()->chainForDocument(B.url().toUrl());
    QVERIFY(BCtx);
    QVERIFY(BCtx->importedParentContexts().isEmpty());

    auto CCtx = DUChain::self()->chainForDocument(C.url().toUrl());
    QVERIFY(CCtx);
    QCOMPARE(CCtx->importedParentContexts().size(), 1);
    QVERIFY(CCtx->imports(BCtx, CursorInRevision(1, 10)));

    auto ACtx = A.topContext();
    QVERIFY(ACtx);
    QCOMPARE(ACtx->importedParentContexts().size(), 2);
    QVERIFY(ACtx->imports(BCtx, CursorInRevision(0, 10)));
    QVERIFY(ACtx->imports(CCtx, CursorInRevision(1, 10)));
}

void TestDUChain::testEnvironmentWithDifferentOrderOfElements()
{
    TestFile file(QStringLiteral("int main();\n"), QStringLiteral("cpp"));

    m_provider->includes.clear();
    m_provider->includes.append(Path(QStringLiteral("/path1")));
    m_provider->includes.append(Path(QStringLiteral("/path2")));

    m_provider->defines.clear();
    m_provider->defines.insert(QStringLiteral("key1"), QStringLiteral("value1"));
    m_provider->defines.insert(QStringLiteral("key2"), QStringLiteral("value2"));
    m_provider->defines.insert(QStringLiteral("key3"), QStringLiteral("value3"));

    size_t previousHash = 0;
    for (int i: {0, 1, 2, 3}) {
        file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);

        QVERIFY(file.waitForParsed(5000));

        {
            DUChainReadLocker lock;
            QVERIFY(file.topContext());
            auto env = dynamic_cast<ClangParsingEnvironmentFile*>(file.topContext()->parsingEnvironmentFile().data());
            QVERIFY(env);
            QCOMPARE(env->environmentQuality(), ClangParsingEnvironment::Source);
            if (previousHash) {
                if (i == 3) {
                    QVERIFY(previousHash != env->environmentHash());
                } else {
                    QCOMPARE(previousHash, env->environmentHash());
                }
            }
            previousHash = env->environmentHash();
            QVERIFY(previousHash);
        }

        if (i == 0) {
            //Change order of defines. Hash of the environment should stay the same.
            m_provider->defines.clear();
            m_provider->defines.insert(QStringLiteral("key3"), QStringLiteral("value3"));
            m_provider->defines.insert(QStringLiteral("key1"), QStringLiteral("value1"));
            m_provider->defines.insert(QStringLiteral("key2"), QStringLiteral("value2"));
        } else if (i == 1) {
            //Add the same macros twice. Hash of the environment should stay the same.
            m_provider->defines.clear();
            m_provider->defines.insert(QStringLiteral("key2"), QStringLiteral("value2"));
            m_provider->defines.insert(QStringLiteral("key3"), QStringLiteral("value3"));
            m_provider->defines.insert(QStringLiteral("key3"), QStringLiteral("value3"));
            m_provider->defines.insert(QStringLiteral("key1"), QStringLiteral("value1"));
        } else if (i == 2) {
            //OTOH order of includes should change hash of the environment.
            m_provider->includes.clear();
            m_provider->includes.append(Path(QStringLiteral("/path2")));
            m_provider->includes.append(Path(QStringLiteral("/path1")));
        }
    }
}

void TestDUChain::testReparseMacro()
{
    TestFile file(QStringLiteral("#define DECLARE(a) typedef struct a##_ {} *a;\nDECLARE(D);\nD d;"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST);
    QVERIFY(file.waitForParsed(5000));

    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
    }

    file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 5);

    auto macroDefinition = file.topContext()->localDeclarations()[0];
    QVERIFY(macroDefinition);
    QCOMPARE(macroDefinition->range(), RangeInRevision(0,8,0,15));
    QCOMPARE(macroDefinition->uses().size(), 1);
    QCOMPARE(macroDefinition->uses().begin()->first(), RangeInRevision(1,0,1,7));

    auto structDeclaration = file.topContext()->localDeclarations()[1];
    QVERIFY(structDeclaration);
    QCOMPARE(structDeclaration->range(), RangeInRevision(1,0,1,0));

    auto structTypedef = file.topContext()->localDeclarations()[3];
    QVERIFY(structTypedef);
    QCOMPARE(structTypedef->range(), RangeInRevision(1,8,1,9));
    QCOMPARE(structTypedef->uses().size(), 1);
    QCOMPARE(structTypedef->uses().begin()->first(), RangeInRevision(2,0,2,1));
}

void TestDUChain::testGotoStatement()
{
    TestFile file(QStringLiteral("int main() {\ngoto label;\ngoto label;\nlabel: return 0;}"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 1);
    auto main = file.topContext()->localDeclarations()[0];
    QVERIFY(main);
    auto mainCtx = main->internalContext()->childContexts().first();
    QVERIFY(mainCtx);
    QCOMPARE(mainCtx->localDeclarations().size(), 1);
    auto label = mainCtx->localDeclarations().first();
    QVERIFY(label);
    QCOMPARE(label->range(), RangeInRevision(3,0,3,5));

    QCOMPARE(label->uses().size(), 1);
    QCOMPARE(label->uses().begin()->first(), RangeInRevision(1,5,1,10));
    QCOMPARE(label->uses().begin()->last(), RangeInRevision(2,5,2,10));
}

void TestDUChain::testRangesOfOperatorsInsideMacro()
{
    TestFile file(QStringLiteral("class Test{public: Test& operator++(int);};\n#define MACRO(var) var++;\nint main(){\nTest tst; MACRO(tst)}"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 3);
    auto testClass = file.topContext()->localDeclarations()[0];
    QVERIFY(testClass);
    auto operatorPlusPlus = testClass->internalContext()->localDeclarations().first();
    QVERIFY(operatorPlusPlus);
    QCOMPARE(operatorPlusPlus->uses().size(), 1);
    QCOMPARE(operatorPlusPlus->uses().begin()->first(), RangeInRevision(3,10,3,10));
}

void TestDUChain::testUsesCreatedForDeclarations()
{
    auto code = R"(template<typename T> void functionTemplate(T);
            template<typename U> void functionTemplate(U) {}

            namespace NS { class Class{}; }
            using NS::Class;

            Class function();
            NS::Class function() { return {}; }

            int main () {
                functionTemplate(int());
                function(); }
    )";
    TestFile file(code, QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());

    auto functionTemplate = file.topContext()->findDeclarations(QualifiedIdentifier(QStringLiteral("functionTemplate")));
    QVERIFY(!functionTemplate.isEmpty());
    auto functionTemplateDeclaration = DUChainUtils::declarationForDefinition(functionTemplate.first());
    QVERIFY(!functionTemplateDeclaration->isDefinition());
#if CINDEX_VERSION_MINOR < 29
    QEXPECT_FAIL("", "No API in LibClang to determine function template type", Continue);
#endif
    QCOMPARE(functionTemplateDeclaration->uses().count(), 1);

    auto function = file.topContext()->findDeclarations(QualifiedIdentifier(QStringLiteral("function")));
    QVERIFY(!function.isEmpty());
    auto functionDeclaration = DUChainUtils::declarationForDefinition(function.first());
    QVERIFY(!functionDeclaration->isDefinition());
    QCOMPARE(functionDeclaration->uses().count(), 1);
}

void TestDUChain::testReparseIncludeGuard()
{
    TestFile header(QStringLiteral("#ifndef GUARD\n#define GUARD\nint something;\n#endif\n"), QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().str() + "\"\n", QStringLiteral("cpp"), &header);

    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST));
    {
        DUChainReadLocker lock;
        QCOMPARE(static_cast<TopDUContext*>(impl.topContext()->
            importedParentContexts().first().context(impl.topContext()))->problems().size(), 0);
    }
    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    {
        DUChainReadLocker lock;
        QCOMPARE(static_cast<TopDUContext*>(impl.topContext()->
            importedParentContexts().first().context(impl.topContext()))->problems().size(), 0);
    }
}

void TestDUChain::testIncludeGuardHeaderHeaderOnly()
{
    QFETCH(QString, code);

    // test that we do NOT get a warning for single standalone header file with include guards
    TestFile header(code, QStringLiteral("h"));

    QVERIFY(header.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST));
    {
        DUChainReadLocker lock;
        QCOMPARE(header.topContext()->problems().size(), 0);
    }
}

void TestDUChain::testIncludeGuardHeaderHeaderOnly_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("guard-ifdef") << QStringLiteral(
        "#ifndef GUARD\n"
        "#define GUARD\n"
        "int something;\n"
        "#endif\n"
    );

    QTest::newRow("guard-pragma") << QStringLiteral(
        "#pragma once\n"
        "int something;\n"
    );
}

void TestDUChain::testIncludeGuardHeaderWarning()
{
    // test that we do get a warning for a header file without include guards
    TestFile header(QStringLiteral("int something;\n"), QStringLiteral("h"));
    TestFile impl("#include \"" + header.url().str() + "\"\n", QStringLiteral("cpp"));

    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate));
    {
        DUChainReadLocker lock;
        QVERIFY(impl.topContext());

        auto context = DUChain::self()->chainForDocument(impl.url());
        QVERIFY(context);
        QCOMPARE(context->problems().size(), 0);

        context = DUChain::self()->chainForDocument(header.url());
        QVERIFY(context);
        QCOMPARE(context->problems().size(), 1);
    }
}

void TestDUChain::testExternC()
{
    auto code = R"(extern "C" { void foo(); })";
    TestFile file(code, QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    QVERIFY(!top->findDeclarations(QualifiedIdentifier(u"foo")).isEmpty());
}

void TestDUChain::testIncludeExternC()
{
    TestFile header(QStringLiteral("int foo() { return 42; }\n"), QStringLiteral("h"));
    // NOTE: header is _not_ explicitly being parsed, instead the impl job does that

    const auto code = R"(
        extern "C" {
            #include "%1"
        }
        int main() { return foo(); }
    )";
    TestFile impl(QString::fromUtf8(code).arg(header.url().str()), QStringLiteral("cpp"), &header);
    impl.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(impl.waitForParsed());

    DUChainReadLocker lock;

    auto implCtx = impl.topContext();
    QVERIFY(implCtx);
    QCOMPARE(implCtx->localDeclarations().size(), 1);

    auto headerCtx = DUChain::self()->chainForDocument(header.url());
    QVERIFY(headerCtx);
    QCOMPARE(headerCtx->localDeclarations().size(), 1);
    Declaration* foo = headerCtx->localDeclarations().first();
    QCOMPARE(foo->uses().size(), 1);
    QCOMPARE(foo->uses().begin().key(), impl.url());
    QCOMPARE(foo->uses().begin()->size(), 1);
    QCOMPARE(foo->uses().begin()->first(), RangeInRevision(4, 28, 4, 31));
}

void TestDUChain::testReparseUnchanged_data()
{
    QTest::addColumn<QString>("headerCode");
    QTest::addColumn<QString>("implCode");

    QTest::newRow("include-guards") << R"(
        #ifndef GUARD
        #define GUARD
        int something;
        #endif
    )" << R"(
        #include "%1"
    )";

    QTest::newRow("template-default-parameters") << R"(
        #ifndef TEST_H
        #define TEST_H

        template<unsigned T=123, unsigned... U>
        class dummy;

        template<unsigned T, unsigned... U>
        class dummy {
            int field[T];
        };

        #endif
    )" << R"(
        #include "%1"

        int main(int, char **) {
            dummy<> x;
            (void)x;
        }
    )";
}

void TestDUChain::testReparseUnchanged()
{
    QFETCH(QString, headerCode);
    QFETCH(QString, implCode);
    TestFile header(headerCode, QStringLiteral("h"));
    TestFile impl(implCode.arg(header.url().str()), QStringLiteral("cpp"), &header);

    auto checkProblems = [&] (bool reparsed) {
        DUChainReadLocker lock;
        auto headerCtx = DUChain::self()->chainForDocument(header.url());
        QVERIFY(headerCtx);
        QVERIFY(headerCtx->problems().isEmpty());
        auto implCtx = DUChain::self()->chainForDocument(impl.url());
        QVERIFY(implCtx);
        if (reparsed && CINDEX_VERSION_MINOR > 29 && CINDEX_VERSION_MINOR < 33) {
            QEXPECT_FAIL("template-default-parameters", "the precompiled preamble messes the default template parameters up in clang 3.7", Continue);
        }
        QVERIFY(implCtx->problems().isEmpty());
    };

    impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST);
    checkProblems(false);

    impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive);
    checkProblems(true);
}

void TestDUChain::testTypeAliasTemplate()
{
    TestFile file(QStringLiteral("template <typename T> using Alias = T; using Foo = Alias<int>;"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());

    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto templateAlias = file.topContext()->localDeclarations().first();
    QVERIFY(templateAlias);
#if CINDEX_VERSION_MINOR < 31
    QEXPECT_FAIL("", "TypeAliasTemplate is not exposed via LibClang", Abort);
#endif
    QVERIFY(templateAlias->isTypeAlias());
    QVERIFY(templateAlias->abstractType());
    QCOMPARE(templateAlias->abstractType()->toString(), QStringLiteral("Alias"));
    QCOMPARE(templateAlias->uses().size(), 1);
    QCOMPARE(templateAlias->uses().first().size(), 1);
    QCOMPARE(templateAlias->uses().first().first(), RangeInRevision(0, 51, 0, 56));
}

void TestDUChain::testDeclarationsInsideMacroExpansion()
{
    TestFile header(QStringLiteral("#define DECLARE(a) typedef struct a##__ {int var;} *a\nDECLARE(D);\n"), QStringLiteral("h"));
    TestFile file("#include \"" + header.url().str() + "\"\nint main(){\nD d; d->var;}\n", QStringLiteral("cpp"));

    file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST);
    QVERIFY(file.waitForParsed(5000));

    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
    }

    file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST | TopDUContext::ForceUpdate);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 1);

    auto context = file.topContext()->childContexts().first()->childContexts().first();
    QVERIFY(context);
    QCOMPARE(context->localDeclarations().size(), 1);
    QCOMPARE(context->usesCount(), 3);

    QCOMPARE(context->uses()[0].m_range, RangeInRevision({2, 0}, {2, 1}));
    QCOMPARE(context->uses()[1].m_range, RangeInRevision({2, 5}, {2, 6}));
    QCOMPARE(context->uses()[2].m_range, RangeInRevision({2, 8}, {2, 11}));
}

// see also: https://bugs.kde.org/show_bug.cgi?id=368067
void TestDUChain::testForwardTemplateTypeParameterContext()
{
    TestFile file(QStringLiteral(R"(
        template<typename MatchingName> class Foo;

        class MatchingName { void bar(); };
        void MatchingName::bar() {  }
    )"), QStringLiteral("cpp"));

    file.parse();
    QVERIFY(file.waitForParsed(500));
    DUChainReadLocker lock;
    const auto top = file.topContext();
    QVERIFY(top);
    DUChainDumper dumper(DUChainDumper::Features(DUChainDumper::DumpContext | DUChainDumper::DumpProblems));
    dumper.dump(top);

    auto declarations = top->localDeclarations();
    QCOMPARE(declarations.size(), 2);
}

// see also: https://bugs.kde.org/show_bug.cgi?id=368460
void TestDUChain::testTemplateFunctionParameterName()
{
    TestFile file(QStringLiteral(R"(
        template<class T>
        void foo(int name);

        void bar(int name);
    )"), QStringLiteral("cpp"));

    file.parse();
    QVERIFY(file.waitForParsed(500));
    DUChainReadLocker lock;
    const auto top = file.topContext();
    QVERIFY(top);
    DUChainDumper dumper(DUChainDumper::Features(DUChainDumper::DumpContext | DUChainDumper::DumpProblems));
    dumper.dump(top);

    const auto declarations = top->localDeclarations();
    QCOMPARE(declarations.size(), 2);

    for (auto decl : declarations) {
        auto ctx = DUChainUtils::argumentContext(decl);
        QVERIFY(ctx);
        auto args = ctx->localDeclarations();
        if (decl == declarations.first())
            QEXPECT_FAIL("", "We get two declarations, for both template and args :(", Continue);
        QCOMPARE(args.size(), 1);
        if (decl == declarations.first())
            QEXPECT_FAIL("", "see above, this then triggers T T here", Continue);
        QCOMPARE(args.first()->toString(), QStringLiteral("int name"));
    }
}

static bool containsErrors(const QList<Problem::Ptr>& problems)
{
    auto it = std::find_if(problems.begin(), problems.end(), [] (const Problem::Ptr& problem) {
        return problem->severity() == Problem::Error;
    });
    return it != problems.end();
}

static bool expectedError(const QList<Problem::Ptr>& problems, QLatin1String message)
{
    for (const auto& problem : problems) {
        if (problem->severity() == Problem::Error && !problem->description().contains(message)) {
            return false;
        }
    }
    return true;
}

static void verifyNoErrors(TopDUContext* top, QSet<TopDUContext*>& checked)
{
    const auto problems = top->problems();
    if (containsErrors(problems)) {
        qDebug() << top->url() << top->problems();
        const auto url = top->url().str();
        if (url.endsWith(QLatin1String("xmmintrin.h"))
            && expectedError(problems, QLatin1String("Cannot initialize a parameter of type"))) {
            QEXPECT_FAIL("", "there are still some errors in xmmintrin.h b/c some clang provided intrinsincs are more strict than the GCC ones.", Continue);
            QVERIFY(false);
        } else if (url.endsWith(QLatin1String("c++config.h"))
                   && expectedError(problems, QLatin1String("Invalid suffix 'bf16' on floating constant"))) {
            QEXPECT_FAIL("", "there are still some errors in c++config.h b/c GCC headers use C++23 suffixes", Continue);
            QVERIFY(false);
        } else {
            QFAIL("parse error detected");
        }
    }
    const auto imports = top->importedParentContexts();
    for (const auto& import : imports) {
        auto ctx = import.context(top);
        QVERIFY(ctx);
        auto importedTop = ctx->topContext();
        if (checked.contains(importedTop)) {
            continue;
        }
        checked.insert(importedTop);
        verifyNoErrors(importedTop, checked);
    }
}

void TestDUChain::testFriendDeclaration()
{
    TestFile file(QStringLiteral(R"(
        struct FriendFoo
        {
            friend class FriendBar;
        };

        class FriendBar{};

        FriendBar friendBar;
    )"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(file.waitForParsed(1000));
    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->localDeclarations().size(), 3);

        auto friendBar = file.topContext()->localDeclarations()[1];
        if (CINDEX_VERSION_MINOR < 37) {
            QEXPECT_FAIL("", "Your clang version is too old", Abort);
        }
        QCOMPARE(friendBar->uses().size(), 1);
        QCOMPARE(friendBar->uses().begin()->first(), RangeInRevision(3,25,3,34));
        QCOMPARE(friendBar->uses().begin()->last(), RangeInRevision(8,8,8,17));
    }
}

void TestDUChain::testVariadicTemplateArguments()
{
    TestFile file(QStringLiteral(R"(
        template<typename T, typename... Targs>
        class VariadicTemplate {};

        VariadicTemplate<int, double, bool> variadic;
    )"), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(file.waitForParsed(1000));
    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->localDeclarations().size(), 2);

        auto decl = file.topContext()->localDeclarations()[1];
        QVERIFY(decl);
        if (CINDEX_VERSION_MINOR < 37) {
            QEXPECT_FAIL("", "Your clang version is too old", Abort);
        }
        QCOMPARE(decl->toString(), QStringLiteral("VariadicTemplate< int, double, bool > variadic"));

        QVERIFY(decl->abstractType());
        QCOMPARE(decl->abstractType()->toString(), QStringLiteral("VariadicTemplate< int, double, bool >"));
    }
}

void TestDUChain::testProblemRequestedHere()
{
    auto headerCode = QStringLiteral(R"(
#pragma once

template <typename T>
T AddObjects(const T& a, const T& b)
{
  return a + b;
}
    )");
    TestFile header(headerCode, QStringLiteral("h"));

    QString sourceCode = QStringLiteral(R"(
#include "%1"
struct A {};

int main()
{
    A a, b;
    AddObjects(a, b);
    return 0;
}
    )").arg(header.url().str());
    TestFile impl(sourceCode, QStringLiteral("cpp"), &header);
    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses));

    DUChainReadLocker lock;

    auto top = impl.topContext();
    QVERIFY(top);

    auto* headerCtx = dynamic_cast<TopDUContext*>(top->importedParentContexts().first().context(top));
    QVERIFY(headerCtx);
    QCOMPARE(headerCtx->url(), header.url());

    QCOMPARE(headerCtx->problems().count(), 1);
    QCOMPARE(headerCtx->localDeclarations().count(), 1);

    // Verify that the problem is reported for the source file.
    QCOMPARE(top->problems().count(), 1);
    QCOMPARE(top->localDeclarations().count(), 2);
}

void TestDUChain::testProblemRequestedHereSameFile()
{
    auto sourceCode = QStringLiteral(R"(
struct A {};

template <typename T>
T AddObjects(const T& a, const T& b)
{
  return a + b;
}

int main()
{
    A a, b;
    AddObjects(a, b);
    return 0;
}
    )");
    TestFile impl(sourceCode, QStringLiteral("cpp"));
    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses));

    DUChainReadLocker lock;

    auto top = impl.topContext();
    QVERIFY(top);

    QCOMPARE(top->problems().count(), 2);
}

void TestDUChain::testProblemRequestedHereChain()
{
    auto headerCode = QStringLiteral(R"(
#pragma once

template <typename T>
T AddObjects(const T& a, const T& b)
{
  return a + b;
}
    )");
    TestFile header(headerCode, QStringLiteral("h"));

    QString sourceCode = QStringLiteral(R"(
#include "%1"
struct A {};

template <typename T>
T AddObjects2(const T& a, const T& b)
{
  return AddObjects(a, b);
}

int main()
{
    A a, b;
    AddObjects2(a, b);
    return 0;
}
    )").arg(header.url().str());
    TestFile impl(sourceCode, QStringLiteral("cpp"), &header);
    QVERIFY(impl.parseAndWait(TopDUContext::AllDeclarationsContextsAndUses));

    DUChainReadLocker lock;

    auto top = impl.topContext();
    QVERIFY(top);

    auto* headerCtx = dynamic_cast<TopDUContext*>(top->importedParentContexts().first().context(top));
    QVERIFY(headerCtx);
    QCOMPARE(headerCtx->url(), header.url());

    QCOMPARE(headerCtx->problems().count(), 1);
    QCOMPARE(headerCtx->localDeclarations().count(), 1);

    // Verify that the problem is reported for the source file.
    QCOMPARE(top->problems().count(), 2);
    QCOMPARE(top->localDeclarations().count(), 3);
}

void TestDUChain::testGccCompatibility_data()
{
    QTest::addColumn<QString>("parserArguments");
    QTest::addColumn<QString>("code");

    QTest::newRow("x86intrin") << QString() << QStringLiteral(R"(
            #include <x86intrin.h>

            int main() { return 0; }
        )");
    QTest::newRow("sized-dealloc") << QStringLiteral("-Wall -std=c++14") << QStringLiteral(R"(
            #include <memory>

            int main() {
                auto test = std::make_shared<uint32_t>();
                return 0;
            }
        )");
}

void TestDUChain::testGccCompatibility()
{
    QFETCH(QString, parserArguments);
    QFETCH(QString, code);

    // TODO: make it easier to change the compiler provider for testing purposes
    QTemporaryDir dir;
    auto project = new TestProject(Path(dir.path()), this);
    auto definesAndIncludesConfig = project->projectConfiguration()->group("CustomDefinesAndIncludes");
    auto pathConfig = definesAndIncludesConfig.group("ProjectPath0");
    pathConfig.writeEntry("Path", ".");
    if (!parserArguments.isEmpty()) {
        pathConfig.writeEntry("parserArguments", parserArguments);
    }
    pathConfig.group("Compiler").writeEntry("Name", "GCC");
    m_projectController->addProject(project);

    {
        // TODO: Also test in C mode. Currently it doesn't work (some intrinsics missing?)
        TestFile file(code, QStringLiteral("cpp"), project, dir.path());

        file.parse();
        QVERIFY(file.waitForParsed(50000));

        DUChainReadLocker lock;
        QSet<TopDUContext*> checked;
        verifyNoErrors(file.topContext(), checked);
    }

    m_projectController->closeAllProjects();
}

void TestDUChain::testLambda()
{
    TestFile file(QStringLiteral("auto lambda = [](int p1, int p2, int p3) { int var1, var2; };"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());
    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->localDeclarations().size(), 1);
        QCOMPARE(file.topContext()->childContexts().size(), 1);

        auto lambdaContext = file.topContext()->childContexts().first();

        QCOMPARE(lambdaContext->type(), DUContext::Function);

        QCOMPARE(lambdaContext->localDeclarations().size(), 3);
        QCOMPARE(lambdaContext->childContexts().size(), 1);
        QCOMPARE(lambdaContext->childContexts().first()->type(), DUContext::Other);
        QCOMPARE(lambdaContext->childContexts().first()->localDeclarations().size(), 2);
    }
}

void TestDUChain::testQtIntegration()
{
    QTemporaryDir includeDir;
    {
        QDir dir(includeDir.path());
        dir.mkdir(QStringLiteral("QtCore"));
        // create the file but don't put anything in it
        QFile header(includeDir.path() + "/QtCore/qobjectdefs.h");
        QVERIFY(header.open(QIODevice::WriteOnly | QIODevice::Text));
    }
    QTemporaryDir dir;
    auto project = new TestProject(Path(dir.path()), this);
    m_provider->defines.clear();
    m_provider->includes = {Path(includeDir.path() + "/QtCore")};

    m_projectController->addProject(project);

    {
        TestFile file(QStringLiteral(R"(
            #define slots
            #define signals
            #define Q_SLOTS
            #define Q_SIGNALS
            #include <QtCore/qobjectdefs.h>

            struct MyObject {
            public:
              void other1();
            public slots:
              void slot1();
            signals:
              void signal1();
            private Q_SLOTS:
              void slot2();
            Q_SIGNALS:
              void signal2();
            public:
              void other2();
            };
        )"), QStringLiteral("cpp"), project, dir.path());

        file.parse();
        QVERIFY(file.waitForParsed(5000));

        DUChainReadLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->problems().isEmpty());
        const auto methods = top->childContexts().last()->localDeclarations();
        QCOMPARE(methods.size(), 6);
        for (auto method : methods) {
            auto classFunction = dynamic_cast<ClassFunctionDeclaration*>(method);
            QVERIFY(classFunction);
            auto id = classFunction->identifier().toString();
            QCOMPARE(classFunction->isSignal(), id.startsWith(QLatin1String("signal")));
            QCOMPARE(classFunction->isSlot(), id.startsWith(QLatin1String("slot")));
        }
    }

    m_projectController->closeAllProjects();
}

void TestDUChain::testHasInclude()
{
    TestFile header(QStringLiteral(R"(
        #pragma once
        #if __has_include_next(<atomic>)
        // good
        #else
        #error broken c++11 setup (__has_include_next)
        #endif
    )"), QStringLiteral("h"));
    // NOTE: header is _not_ explicitly being parsed, instead the impl job does that

    TestFile file(QStringLiteral(R"(
        #if __has_include(<atomic>)
        // good
        #else
        #error broken c++11 setup (__has_include)
        #endif
        #include "%1"
    )").arg(header.url().str()), QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(file.waitForParsed(1000));
    {
        DUChainDumper dumper{DUChainDumper::DumpProblems};

        DUChainReadLocker lock;

        QVERIFY(file.topContext());
        dumper.dump(file.topContext());
        QVERIFY(file.topContext()->problems().isEmpty());

        auto headerCtx = DUChain::self()->chainForDocument(header.url());
        QVERIFY(headerCtx);
        dumper.dump(headerCtx);
        QVERIFY(headerCtx->problems().count() <= 1);
        const auto& headerProblems = headerCtx->problems();
        for (const auto& problem : headerProblems) {
            // ignore the following error:  "#include_next with absolute path [-Winclude-next-absolute-path]" "" [ (2, 12)  ->  (2, 30) ]
            QVERIFY(problem->description().contains(QLatin1String("-Winclude-next-absolute-path")));
        }
    }
}

void TestDUChain::testSameFunctionDefinition()
{
    QString source(QStringLiteral(R"(
        #include <stdio.h>
        #include <stdlib.h>

        void configure()
        {
            printf("do stuff\n");
        }
    )"));

    QTemporaryDir dir;
    auto project = new TestProject(Path(dir.path()), this);
    m_projectController->addProject(project);

    TestFile file1(source, QStringLiteral("c"), project);
    TestFile file2(source, QStringLiteral("c"), project);
    TestFile file3(source, QStringLiteral("c"), project);

    file1.parse(TopDUContext::AllDeclarationsContextsAndUses);
    file2.parse(TopDUContext::AllDeclarationsContextsAndUses);
    file3.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(file1.waitForParsed(1000));
    QVERIFY(file2.waitForParsed(1000));
    QVERIFY(file3.waitForParsed(1000));

    auto checkFunctionDefinition = [] (TestFile & file) {
        DUChainReadLocker lock;
        QCOMPARE(file.topContext()->localDeclarations().count(), 1);
        auto configureFunc = file.topContext()->localDeclarations().first();
        QCOMPARE(FunctionDefinition::definition(configureFunc), configureFunc);
    };

    checkFunctionDefinition(file1);
    checkFunctionDefinition(file2);
    checkFunctionDefinition(file3);

    m_projectController->closeAllProjects();
}

void TestDUChain::testSizeAlignOf()
{
    TestFile file(QStringLiteral(R"(
        template<typename T>
        struct Foo { T arg; };

        using Bar = Foo<int>;
        Bar asdf;
        Foo<double> blub;
        template<typename F>
        using Asdf = Foo<F>;
        Asdf<char> c;

        struct MyClass {
            int i;
            int j;
        };
        void *ptr = nullptr;

        enum E { A, B };
        E e = A;
        enum class E2 : int { A, B };
        E2 e2 = E2::A;
        enum class E3 : long { A, B };
        E3 e3 = E3::A;

        int i = 0;
        int& ref = i;
        struct ref_struct { int& ref; };

        int fun();

        namespace myns { struct bar { int foo; }; }
        myns::bar myns_use;

        struct MyClass c_struct;
    )"),
                  QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(1000));

    DUChainReadLocker lock;
    const auto top = file.topContext();
    QVERIFY(top);
    QVERIFY(top->problems().isEmpty());

    const auto decs = top->localDeclarations();
    QCOMPARE(decs.size(), 21);

    auto check = [&](int i, const QString& id, qint64 sizeOf, qint64 alignOf) {
        QCOMPARE(decs[i]->identifier().toString(), id);
        const auto type = TypeUtils::unAliasedType(decs[i]->abstractType());
        QVERIFY(type);
        QCOMPARE(type->sizeOf(), sizeOf);
        QCOMPARE(type->alignOf(), alignOf);
    };
    check(0, "Foo< T >", -1, -1);
    check(1, "Bar", 4, 4);
    check(2, "asdf", 4, 4);
    check(3, "blub", 8, 8);
    check(4, "Asdf", -1, -1);
    check(5, "c", 1, 1);
    check(6, "MyClass", 8, 4);
    check(7, "ptr", 8, 8);
    check(8, "E", 4, 4);
    check(9, "e", 4, 4);
    check(10, "E2", 4, 4);
    check(11, "e2", 4, 4);
    check(12, "E3", 8, 8);
    check(13, "e3", 8, 8);
    check(14, "i", 4, 4);
    // unexpected, but apparently correct
    // https://stackoverflow.com/questions/26631169/why-does-sizeof-a-reference-type-give-you-the-sizeof-the-type
    check(15, "ref", 4, 4);
    check(16, "ref_struct", 8, 8);
    check(17, "fun", -1, -1);
    check(19, "myns_use", 4, 4);
    check(20, "c_struct", 8, 4);
}

void TestDUChain::testSizeAlignOfUpdate()
{
    TestFile file(QStringLiteral(R"(
        struct foo { int i; };
    )"),
                  QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(1000));

    {
        DUChainReadLocker lock;
        const auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->problems().isEmpty());
        QCOMPARE(top->localDeclarations().size(), 1);
        const auto type = top->localDeclarations().constFirst()->abstractType();
        QCOMPARE(type->sizeOf(), 4);
        QCOMPARE(type->alignOf(), 4);
    }

    file.setFileContents(QStringLiteral(R"(
        struct foo { char i[124]; };
    )"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate);
    QVERIFY(file.waitForParsed(1000));

    {
        DUChainReadLocker lock;
        const auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->problems().isEmpty());
        QCOMPARE(top->localDeclarations().size(), 1);
        const auto type = top->localDeclarations().constFirst()->abstractType();
        QCOMPARE(type->sizeOf(), 124);
        QCOMPARE(type->alignOf(), 1);
    }
}

void TestDUChain::testBitWidth()
{
#if CINDEX_VERSION_MINOR >= 16
    TestFile file(QStringLiteral(R"(
        #include <climits>

        struct foo {
            int a;
            unsigned int b:1;
            unsigned char c:7;
            int d:32;
            int e[2];
            unsigned       length    : (sizeof(int)*CHAR_BIT - 1);

            // error case
            int f:0;
            int g:-1;
            int h:33;
        };)"), QStringLiteral("cpp"));

    QVERIFY(file.parseAndWait());
    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->localDeclarations().size(), 1);
        QCOMPARE(file.topContext()->childContexts().size(), 1);

        auto fooContext = file.topContext()->childContexts().first();
        QVERIFY(fooContext);
        QCOMPARE(fooContext->type(), DUContext::Class);
        QCOMPARE(fooContext->localDeclarations().size(), 9);
        QCOMPARE(fooContext->childContexts().size(), 0);

        auto varA = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(0));
        QVERIFY(varA);
        QCOMPARE(varA->bitWidth(), ClassMemberDeclaration::NotABitField);
        auto varB = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(1));
        QVERIFY(varB);
        QCOMPARE(varB->bitWidth(), 1);
        auto varC = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(2));
        QVERIFY(varC);
        QCOMPARE(varC->bitWidth(), 7);
        auto varD = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(3));
        QVERIFY(varD);
        QCOMPARE(varD->bitWidth(), 32);
        auto varE = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(4));
        QVERIFY(varE);
        QCOMPARE(varE->bitWidth(), ClassMemberDeclaration::NotABitField);
        const auto varLength = dynamic_cast<const ClassMemberDeclaration*>(fooContext->localDeclarations().at(5));
        QVERIFY(varLength);
        QCOMPARE(varLength->bitWidth(), sizeof(int) * CHAR_BIT - 1);

        // error case
        auto top = file.topContext();
        QVERIFY(top);
        QCOMPARE(top->problems().count(), 3);

        // error: named bit-field 'f' has zero width
        auto varF = dynamic_cast<ClassMemberDeclaration*>(fooContext->localDeclarations().at(6));
        QVERIFY(varF);
        QCOMPARE(varF->bitWidth(), ClassMemberDeclaration::NotABitField);
        // error: bit-field 'g' has negative width (-1)
        auto varG = dynamic_cast<ClassMemberDeclaration*>(fooContext->localDeclarations().at(7));
        QVERIFY(varG);
        QCOMPARE(varG->bitWidth(), ClassMemberDeclaration::NotABitField);
        // warning: width of bit-field 'h' (33 bits) exceeds the width of its type; value will be truncated to 32 bits
        auto varH = dynamic_cast<ClassMemberDeclaration*>(fooContext->localDeclarations().at(8));
        QVERIFY(varH);
        QCOMPARE(varH->bitWidth(), 33);
    }
#endif
}

void TestDUChain::testValueDependentBitWidth()
{
    if (QVersionNumber::fromString(ClangHelpers::clangVersion()) < QVersionNumber(17, 0, 0)) {
        QSKIP("Parsing value-dependent bit-fields may cause an assertion failure or a crash in libclang version < 17");
    }

    TestFile file(QStringLiteral(R"(
        #include <climits>

        template<class T, unsigned I>
        struct Str {
            typedef typename T::size_type size_type;

            int w : sizeof(double);
            int x : sizeof(T);
            short y : I + 1;
            unsigned       length1   : (sizeof(size_type)*CHAR_BIT - 1);
            size_type      length2   : (sizeof(size_type)*CHAR_BIT - 1);
            size_type z = 5;
        };
    )"),
                  QStringLiteral("cpp"));

    QVERIFY(file.parseAndWait());
    {
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        QCOMPARE(file.topContext()->localDeclarations().size(), 1);
        QCOMPARE(file.topContext()->childContexts().size(), 1);

        const auto strContext = file.topContext()->childContexts().constFirst();
        QVERIFY(strContext);
        QCOMPARE(strContext->type(), DUContext::Class);
        QCOMPARE(strContext->childContexts().size(), 0);

        const auto strDeclarations = strContext->localDeclarations();
        QCOMPARE(strDeclarations.size(), 9);

        const auto varW = dynamic_cast<const ClassMemberDeclaration*>(strDeclarations.at(3));
        QVERIFY(varW);
        QCOMPARE(varW->bitWidth(), sizeof(double));

        for (int i = 4; i < strDeclarations.size() - 1; ++i) {
            const auto member = dynamic_cast<const ClassMemberDeclaration*>(strDeclarations.at(i));
            QVERIFY(member);
            QCOMPARE(member->bitWidth(), ClassMemberDeclaration::ValueDependentBitWidth);
        }

        const auto varZ = dynamic_cast<const ClassMemberDeclaration*>(strDeclarations.constLast());
        QVERIFY(varZ);
        QCOMPARE(varZ->bitWidth(), ClassMemberDeclaration::NotABitField);
    }
}

void TestDUChain::testBitWidthUpdate()
{
#if CINDEX_VERSION_MINOR >= 16
    TestFile file(QStringLiteral(R"(
        struct foo { int i:7; };
    )"),
                  QStringLiteral("cpp"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(1000));

    {
        DUChainReadLocker lock;
        const auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->problems().isEmpty());
        QCOMPARE(top->localDeclarations().size(), 1);
        QCOMPARE(top->childContexts().size(), 1);

        const auto fooContext = top->childContexts().first();
        QVERIFY(fooContext);
        QCOMPARE(fooContext->type(), DUContext::Class);
        QCOMPARE(fooContext->localDeclarations().size(), 1);
        QCOMPARE(fooContext->childContexts().size(), 0);

        const auto varI = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(0));
        QCOMPARE(varI->bitWidth(), 7);
    }

    file.setFileContents(QStringLiteral(R"(
        struct foo { int i; };
    )"));
    file.parse(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate);
    QVERIFY(file.waitForParsed(1000));

    {
        DUChainReadLocker lock;
        const auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->problems().isEmpty());
        QCOMPARE(top->localDeclarations().size(), 1);
        QCOMPARE(top->childContexts().size(), 1);

        const auto fooContext = top->childContexts().first();
        QVERIFY(fooContext);
        QCOMPARE(fooContext->type(), DUContext::Class);
        QCOMPARE(fooContext->localDeclarations().size(), 1);
        QCOMPARE(fooContext->childContexts().size(), 0);

        const auto varI = dynamic_cast<ClassMemberDeclaration *>(fooContext->localDeclarations().at(0));
        QCOMPARE(varI->bitWidth(), ClassMemberDeclaration::NotABitField);
    }
#endif
}

#include "moc_test_duchain.cpp"
