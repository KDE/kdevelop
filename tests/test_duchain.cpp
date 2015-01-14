/*
 * Copyright 2014  Milian Wolff <mail@milianw.de>
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

#include "test_duchain.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>
#include <util/kdevstringhandler.h>

#include "duchain/clangparsingenvironmentfile.h"
#include "duchain/clangparsingenvironment.h"
#include "duchain/parsesession.h"

#include <custom-definesandincludes/idefinesandincludesmanager.h>

#include <QtTest>

QTEST_GUILESS_MAIN(TestDUChain);

using namespace KDevelop;

class TestEnvironmentProvider final : public IDefinesAndIncludesManager::BackgroundProvider
{
public:
    virtual ~TestEnvironmentProvider() = default;
    virtual QHash< QString, QString > definesInBackground(const QString& /*path*/) const
    {
        return defines;
    }

    virtual Path::List includesInBackground(const QString& /*path*/) const
    {
        return includes;
    }

    virtual IDefinesAndIncludesManager::Type type() const
    {
        return IDefinesAndIncludesManager::UserDefined;
    }

    QHash<QString, QString> defines;
    Path::List includes;
};

TestDUChain::~TestDUChain() = default;

void TestDUChain::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

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

void TestDUChain::testComments()
{
    QFETCH(QString, code);
    QFETCH(ExpectedComment, expectedComment);

    TestFile file(code, "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);
    auto candidates = top->findDeclarations(QualifiedIdentifier(expectedComment.identifier));
    QVERIFY(!candidates.isEmpty());
    auto decl = candidates.first();
    QString comment = QString::fromLocal8Bit(decl->comment());
    comment = KDevelop::htmlToPlainText(comment, KDevelop::CompleteMode);
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
    QTest::newRow("enumerator")
        << "enum Foo { bar1, ///<this is bar1\nbar2 ///<this is bar2\n };"
        << ExpectedComment{"Foo::bar1", "this is bar1"};
    QTest::newRow("comment-formatting")
        << "/** a\n * multiline\n *\n * comment\n */ int foo;"
        << ExpectedComment{"foo", "a multiline\ncomment"};
    QTest::newRow("comment-doxygen-tags")
        << "/** @see bar()\n@param a foo\n*/\nvoid foo(int a);\nvoid bar();"
        << ExpectedComment{"foo", "bar()\na\nfoo"};
}

void TestDUChain::testInclude()
{
    TestFile header("int foo() { return 42; }\n", "h");
    // NOTE: header is _not_ explictly being parsed, instead the impl job does that

    TestFile impl("#include \"" + header.url().byteArray() + "\"\n"
                  "int main() { return foo(); }", "cpp", &header);
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
    TestFile header1(createCode("Header1", 1000), "h");
    TestFile header2(createCode("Header2", 1000), "h");
    TestFile header3(createCode("Header3", 1000), "h");

    ICore::self()->languageController()->backgroundParser()->setThreadCount(3);

    TestFile impl1("#include \"" + header1.url().byteArray() + "\"\n"
                   "#include \"" + header2.url().byteArray() + "\"\n"
                   "#include \"" + header3.url().byteArray() + "\"\n"
                   "int main() { return 0; }", "cpp");

    TestFile impl2("#include \"" + header2.url().byteArray() + "\"\n"
                   "#include \"" + header1.url().byteArray() + "\"\n"
                   "#include \"" + header3.url().byteArray() + "\"\n"
                   "int main() { return 0; }", "cpp");

    TestFile impl3("#include \"" + header3.url().byteArray() + "\"\n"
                   "#include \"" + header1.url().byteArray() + "\"\n"
                   "#include \"" + header2.url().byteArray() + "\"\n"
                   "int main() { return 0; }", "cpp");

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
    TestFile file("int main() { int i = 42; return i; }", "cpp");
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
            file.setFileContents("int main()\n{\nfloat i = 13; return i - 5;\n}\n");
        }

        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    }
}

void TestDUChain::testReparseError()
{
    TestFile file("int i = 1 / 0;\n", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(file.waitForParsed(500));
        DUChainReadLocker lock;
        QVERIFY(file.topContext());
        if (!i) {
            QCOMPARE(file.topContext()->problems().size(), 1);
            file.setFileContents("int i = 0;\n");
        } else {
            QCOMPARE(file.topContext()->problems().size(), 0);
        }

        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    }
}

void TestDUChain::testTemplate()
{
    TestFile file("template<typename T> struct foo { T bar; };\n"
                  "int main() { foo<int> myFoo; return myFoo.bar; }\n", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto fooDecl = file.topContext()->localDeclarations().first();
    QVERIFY(fooDecl->internalContext());
    QCOMPARE(fooDecl->internalContext()->localDeclarations().size(), 2);

    QCOMPARE(file.topContext()->findDeclarations(QualifiedIdentifier("foo")).size(), 1);
    QCOMPARE(file.topContext()->findDeclarations(QualifiedIdentifier("foo::bar")).size(), 1);
}

void TestDUChain::testNamespace()
{
    TestFile file("namespace foo { struct bar { int baz; }; }\n"
                  "int main() { foo::bar myBar; }\n", "cpp");
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
        QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("foo")).size(), 1);
        QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("foo::bar")).size(), 1);
        QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("foo::bar::baz")).size(), 1);
    }
}

void TestDUChain::testAutoTypeDeduction()
{
    TestFile file("auto foo = 5;\n", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;

    DUContext* ctx = file.topContext().data();
    QVERIFY(ctx);
    QCOMPARE(ctx->localDeclarations().size(), 1);
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("foo")).size(), 1);
    Declaration* decl = ctx->findDeclarations(QualifiedIdentifier("foo"))[0];
    QCOMPARE(decl->identifier(), Identifier("foo"));
    QEXPECT_FAIL("", "No type deduction here unfortunately, missing API in Clang", Continue);
    QVERIFY(decl->type<IntegralType>());
}

void TestDUChain::testTypeDeductionInTemplateInstantiation()
{
    // see: http://clang-developers.42468.n3.nabble.com/RFC-missing-libclang-query-functions-features-td2504253.html
    TestFile file("template<typename T> struct foo { T member; } foo<int> f; auto i = f.member;", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;

    DUContext* ctx = file.topContext().data();
    QVERIFY(ctx);
    QCOMPARE(ctx->localDeclarations().size(), 3);
    Declaration* decl = 0;

    // check 'foo' declaration
    decl = ctx->localDeclarations()[0];
    QVERIFY(decl);
    QCOMPARE(decl->identifier(), Identifier("foo"));

    // check type of 'member' inside declaration-scope
    QCOMPARE(ctx->childContexts().size(), 1);
    DUContext* fooCtx = ctx->childContexts().first();
    QVERIFY(fooCtx);
    // Should there really be two declarations?
    QCOMPARE(fooCtx->localDeclarations().size(), 2);
    decl = fooCtx->localDeclarations()[1];
    QCOMPARE(decl->identifier(), Identifier("member"));

    // check type of 'member' in definition of 'f'
    decl = ctx->localDeclarations()[1];
    QCOMPARE(decl->identifier(), Identifier("f"));
    decl = ctx->localDeclarations()[2];
    QCOMPARE(decl->identifier(), Identifier("i"));
    QEXPECT_FAIL("", "No type deduction here unfortunately, missing API in Clang", Continue);
    QVERIFY(decl->type<IntegralType>());
}

void TestDUChain::testVirtualMemberFunction()
{
    //Forward-declarations with "struct" or "class" are considered equal, so make sure the override is detected correctly.
    TestFile file("struct S {}; struct A { virtual S* ret(); }; struct B : public A { virtual S* ret(); };", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    DUContext* top = file.topContext().data();
    QVERIFY(top);

    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->localDeclarations().count(), 3);
    QCOMPARE(top->childContexts()[2]->localDeclarations().count(), 1);
    Declaration* decl = top->childContexts()[2]->localDeclarations()[0];
    QCOMPARE(decl->identifier(), Identifier("ret"));
    QVERIFY(DUChainUtils::getOverridden(decl));
}

void TestDUChain::testBaseClasses()
{
    TestFile file("class Base {}; class Inherited : public Base {};", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    DUContext* top = file.topContext().data();
    QVERIFY(top);

    QCOMPARE(top->localDeclarations().count(), 2);
    Declaration* baseDecl = top->localDeclarations().first();
    QCOMPARE(baseDecl->identifier(), Identifier("Base"));

    ClassDeclaration* inheritedDecl = dynamic_cast<ClassDeclaration*>(top->localDeclarations()[1]);
    QCOMPARE(inheritedDecl->identifier(), Identifier("Inherited"));

    QVERIFY(inheritedDecl);
    QCOMPARE(inheritedDecl->baseClassesSize(), 1u);
}

void TestDUChain::testReparseBaseClasses()
{
    TestFile file("struct a{}; struct b : a {};\n", "cpp");
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

        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    }
}

void TestDUChain::testReparseBaseClassesTemplates()
{
    TestFile file("template<typename T> struct a{}; struct b : a<int> {};\n", "cpp");
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

        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    }
}

void TestDUChain::testGlobalFunctionDeclaration()
{
    TestFile file("void foo(int arg1, char arg2);\n", "cpp");
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
    TestFile file("void func(); void func() {}\n", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed());

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);
    auto funcDecl = file.topContext()->localDeclarations()[0];
    QVERIFY(!dynamic_cast<FunctionDefinition*>(funcDecl));
    auto funcDef = file.topContext()->localDeclarations()[1];
    QVERIFY(dynamic_cast<FunctionDefinition*>(funcDef));
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

    TestFile file("struct SomeStruct {} s;\n", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed());

    DUChainReadLocker lock;
    auto top = file.topContext();
    QVERIFY(top);

    // there should only be one declaration for "SomeStruct"
    auto candidates = top->findDeclarations(QualifiedIdentifier("SomeStruct"));
    QCOMPARE(candidates.size(), 1);
}

void TestDUChain::testParsingEnvironment()
{
    const TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses;

    IndexedTopDUContext indexed;
    ClangParsingEnvironment lastEnv;
    {
        TestFile file("int main() {}\n", "cpp");
        auto astFeatures = static_cast<TopDUContext::Features>(features | TopDUContext::AST);
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
        env.addIncludes(Path::List() << Path("/foo/bar/baz"));
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
        env.addIncludes(Path::List() << Path("/lalalala"));
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

void TestDUChain::benchDUChainBuilder()
{
    QBENCHMARK_ONCE {
        TestFile file(
            "#include <vector>\n"
            "#include <map>\n"
            "#include <set>\n"
            "#include <algorithm>\n"
            "#include <functional>\n"
            "#include <limits>\n"
            "#include <bitset>\n"
            "#include <iostream>\n"
            "#include <string>\n"
            "#include <mutex>\n", "cpp");
        file.parse(TopDUContext::AllDeclarationsContextsAndUses);
        QVERIFY(file.waitForParsed(60000));

        DUChainReadLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
    }
}

void TestDUChain::testReparseWithAllDeclarationsContextsAndUses()
{
    TestFile file("int foo() { return 0; } int main() { return foo(); }", "cpp");
    file.parse(TopDUContext::VisibleDeclarationsAndContexts);

    QVERIFY(file.waitForParsed(1000));

    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    QVERIFY(file.waitForParsed(500));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->childContexts().size(), 2);
    QCOMPARE(file.topContext()->localDeclarations().size(), 2);

    DeclarationPointer mainDecl;
    mainDecl = file.topContext()->localDeclarations()[1];
    DeclarationPointer foo;
    foo = file.topContext()->localDeclarations().first();
    QVERIFY(mainDecl->uses().isEmpty());
    QCOMPARE(foo->uses().size(), 1);
}

void TestDUChain::testReparseInclude()
{
    TestFile header("int foo() { return 42; }\n", "h");
    TestFile impl("#include \"" + header.url().byteArray() + "\"\n"
                  "int main() { return foo(); }", "cpp", &header);

    // Use TopDUContext::AST to imitate that document is opened in the editor, so that ClangParseJob can store translation unit, that'll be used for reparsing.
    impl.parse(TopDUContext::Features(TopDUContext::AllDeclarationsAndContexts|TopDUContext::AST));
    QVERIFY(impl.waitForParsed(5000));
    {
        DUChainReadLocker lock;
        auto implCtx = impl.topContext();
        QVERIFY(implCtx);
        QCOMPARE(implCtx->importedParentContexts().size(), 1);
    }

    impl.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses|TopDUContext::AST));
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
    TestFile header("int foo() { return 42; }\n", "h");
    TestFile impl("#include \"" + header.url().byteArray() + "\"\n"
                  "int main() { return foo(); }", "cpp", &header);

    uint hashes[3] = {0, 0, 0};

    for (int i = 0; i < 3; ++i) {
        impl.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses|TopDUContext::AST|TopDUContext::ForceUpdate));
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
            m_provider->defines.insert("foooooooo", "baaar!");
        } else if (i == 1) {
            // 2) change includes
            m_provider->includes.append(Path("/foo/bar/asdf/lalala"));
        } // 3) stop
    }
}

void TestDUChain::testMacrosRanges()
{
    TestFile file("#define FUNC_MACROS(x) x\nFUNC_MACROS(1);", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(file.waitForParsed(5000));

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 1);
    auto macroDefinition = file.topContext()->localDeclarations()[0];
    QVERIFY(macroDefinition);
    QCOMPARE(macroDefinition->range(), RangeInRevision(0,8,0,19));

    QCOMPARE(macroDefinition->uses().size(), 1);
    QCOMPARE(macroDefinition->uses().begin()->first(), RangeInRevision(1,0,1,11));
}

void TestDUChain::testNestedImports()
{
    TestFile B("#pragma once\nint B();\n", "h");
    TestFile C("#pragma once\n#include \"" + B.url().byteArray() + "\"\nint C();\n", "h");
    TestFile A("#include \"" + B.url().byteArray() + "\"\n" + "#include \"" + C.url().byteArray() + "\"\nint A();\n", "cpp");

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
    TestFile file("int main();\n", "cpp");

    m_provider->includes.clear();
    m_provider->includes.append(Path("/path1"));
    m_provider->includes.append(Path("/path2"));

    m_provider->defines.clear();
    m_provider->defines.insert("key1", "value1");
    m_provider->defines.insert("key2", "value2");
    m_provider->defines.insert("key3", "value3");

    uint previousHash = 0;
    for (int i: {0, 1, 2, 3}) {
        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses|TopDUContext::AST|TopDUContext::ForceUpdate));

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
            m_provider->defines.insert("key3", "value3");
            m_provider->defines.insert("key1", "value1");
            m_provider->defines.insert("key2", "value2");
        } else if (i == 1) {
            //Add the same macros twice. Hash of the environment should stay the same.
            m_provider->defines.clear();
            m_provider->defines.insert("key2", "value2");
            m_provider->defines.insert("key3", "value3");
            m_provider->defines.insert("key3", "value3");
            m_provider->defines.insert("key1", "value1");
        } else if (i == 2) {
            //OTOH order of includes should change hash of the environment.
            m_provider->includes.clear();
            m_provider->includes.append(Path("/path2"));
            m_provider->includes.append(Path("/path1"));
        }
    }
}
