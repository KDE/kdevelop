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

#include "duchaintest.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/dumpchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/classdeclaration.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>

QTEST_KDEMAIN(DUChainTest, NoGUI);

using namespace KDevelop;

void DUChainTest::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void DUChainTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void DUChainTest::testInclude()
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

void DUChainTest::testIncludeLocking()
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

void DUChainTest::testReparse()
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

void DUChainTest::testReparseError()
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

void DUChainTest::testTemplate()
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

void DUChainTest::testNamespace()
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

void DUChainTest::testAutoTypeDeduction()
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

void DUChainTest::testTypeDeductionInTemplateInstantiation()
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

void DUChainTest::testVirtualMemberFunction()
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

void DUChainTest::testBaseClasses()
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

void DUChainTest::testReparseBaseClasses()
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

void DUChainTest::testReparseBaseClassesTemplates()
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

void DUChainTest::testGlobalFunctionDeclaration()
{
    TestFile file("void foo(int arg1, char arg2);\n", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);
    file.waitForParsed();

    DUChainReadLocker lock;
    QVERIFY(file.topContext());
    QCOMPARE(file.topContext()->localDeclarations().size(), 1);
}

#include "duchaintest.moc"
