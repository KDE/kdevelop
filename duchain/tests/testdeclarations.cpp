/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
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

#include "testdeclarations.h"

#include "../parsesession.h"
#include "../declarationbuilder.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/problem.h>
#include <language/duchain/classdeclaration.h>

#include <QtTest>

QTEST_GUILESS_MAIN(TestDeclarations);

using namespace KDevelop;

void TestDeclarations::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestDeclarations::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDeclarations::testJSProblems()
{
    const IndexedString file(QUrl("file:///internal/jsproblems.js"));
    ParseSession session(file,
        "function f(a) {}\n"
        "f(2);\n"
        "f(true);\n",
    0);

    QVERIFY(session.ast());
    DeclarationBuilder builder(&session);

    builder.build(file, session.ast());
    auto problems = session.problems();

    QCOMPARE(problems.count(), 1);
    QCOMPARE((KTextEditor::Range)problems.at(0)->finalLocation(), KTextEditor::Range(2, 2, 2, 6));
}

void TestDeclarations::testFunction()
{
    const IndexedString file(QUrl("file:///internal/functionArgs.js"));
    //                          0         1         2         3
    //                          01234567890123456789012345678901234567890
    ParseSession session(file, "/**\n * some comment\n */\n"
                               "function foo(arg1, arg2, arg3) { var i = 0; }", 0);
    QVERIFY(session.ast());
    QVERIFY(session.problems().isEmpty());
    QCOMPARE(session.language(), QmlJS::Language::JavaScript);

    DeclarationBuilder builder(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->localDeclarations().size(), 3); // module, exports and foo

    FunctionDeclaration* fooDec = dynamic_cast<FunctionDeclaration*>(top->localDeclarations().at(2));
    QVERIFY(fooDec);
    QCOMPARE(fooDec->range(), RangeInRevision(3, 9, 3, 12));
    QCOMPARE(QString::fromUtf8(fooDec->comment()), QString("some comment"));

    QVERIFY(fooDec->internalContext());
    DUContext* argCtx = fooDec->internalContext();
    QCOMPARE(argCtx->localDeclarations().size(), 3);

    Declaration* arg1 = argCtx->localDeclarations().at(0);
    QCOMPARE(arg1->identifier().toString(), QString("arg1"));
    QCOMPARE(arg1->range(), RangeInRevision(3, 13, 3, 17));

    Declaration* arg2 = argCtx->localDeclarations().at(1);
    QCOMPARE(arg2->identifier().toString(), QString("arg2"));
    QCOMPARE(arg2->range(), RangeInRevision(3, 19, 3, 23));

    Declaration* arg3 = argCtx->localDeclarations().at(2);
    QCOMPARE(arg3->identifier().toString(), QString("arg3"));
    QCOMPARE(arg3->range(), RangeInRevision(3, 25, 3, 29));

    FunctionType::Ptr funType = fooDec->type<FunctionType>();
    QVERIFY(funType);
    QVERIFY(funType->returnType().cast<IntegralType>());
    QCOMPARE(funType->returnType().cast<IntegralType>()->dataType(), (uint) IntegralType::TypeVoid);

    QCOMPARE(argCtx->childContexts().size(), 2);
    DUContext* bodyCtx = argCtx->childContexts().at(1);
    QVERIFY(bodyCtx);
    QVERIFY(bodyCtx->findDeclarations(arg1->identifier()).contains(arg1));
    QVERIFY(bodyCtx->findDeclarations(arg2->identifier()).contains(arg2));
    QVERIFY(bodyCtx->findDeclarations(arg3->identifier()).contains(arg3));

    QCOMPARE(bodyCtx->localDeclarations().count(), 1);
}

void TestDeclarations::testQMLId()
{
    const IndexedString file(QUrl("file:///internal/qmlId.qml"));

    ReferencedTopDUContext top;

    DeclarationPointer oldDec;
    {
        //                          0         1         2         3
        //                          01234567890123456789012345678901234567890
        ParseSession session(file, "/** file comment **/\n"
                                   "import QtQuick 1.0\n"
                                   "/**\n * some comment\n */\n"
                                   "Text { id: test; Text { id: child; } }", 0);
        QVERIFY(session.ast());
        QVERIFY(session.problems().isEmpty());
        QCOMPARE(session.language(), QmlJS::Language::Qml);

        DeclarationBuilder builder(&session);
        top = builder.build(file, session.ast());
        QVERIFY(top);

        DUChainReadLocker lock;

        QCOMPARE(top->localDeclarations().size(), 5);       // module, exports, Text, test and child are all in the global scope

        // First declaration, the anonymous class
        ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration *>(top->localDeclarations().at(2));
        QVERIFY(classDecl);
        QCOMPARE(classDecl->abstractType()->toString(), QString("qmlId"));
        QCOMPARE(classDecl->range(), RangeInRevision(5, 0, 5, 0));
        QVERIFY(classDecl->internalContext());
        QCOMPARE(classDecl->internalContext()->range(), RangeInRevision(5, 6, 5, 37));

        // Second declaration: test
        Declaration* dec = top->localDeclarations().at(3);
        QVERIFY(dec);
        QCOMPARE(dec->identifier().toString(), QString("test"));
        QCOMPARE(dec->abstractType()->toString(), QString("qmlId"));

        oldDec = dec;
    }

    // test recompile
    {
        //                          0         1         2         3
        //                          01234567890123456789012345678901234567890
        ParseSession session(file, "/** file comment **/\n"
                                   "import QtQuick 1.0\n"
                                   "/**\n * some comment\n */\n"
                                   "Text { id: test; Text { id: child;}\n"
                                   " Text {id: foo;} }", 0);
        QVERIFY(session.ast());
        QVERIFY(session.problems().isEmpty());
        QCOMPARE(session.language(), QmlJS::Language::Qml);

        DeclarationBuilder builder(&session);
        ReferencedTopDUContext top2 = builder.build(file, session.ast(), top);
        QVERIFY(top2);
        QCOMPARE(top2.data(), top.data());

        DUChainReadLocker lock;

        QCOMPARE(top->localDeclarations().size(), 6); // module, exports, Text, test, child and foo
        // First declaration, the anonymous class
        ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration *>(top->localDeclarations().at(2));
        QVERIFY(classDecl);
        QCOMPARE(classDecl->abstractType()->toString(), QString("qmlId"));
        QCOMPARE(classDecl->range(), RangeInRevision(5, 0, 5, 0));
        QVERIFY(classDecl->internalContext());
        QCOMPARE(classDecl->internalContext()->range(), RangeInRevision(5, 6, 6, 17));

        // Second declaration: test
        Declaration* dec = top->localDeclarations().at(3);
        QVERIFY(dec);
        QCOMPARE(dec->identifier().toString(), QString("test"));
        QCOMPARE(dec->abstractType()->toString(), QString("qmlId"));
    }
}

void TestDeclarations::testProperty()
{
    const IndexedString file(QUrl("file:///internal/qmlProperty.qml"));
    //                          0         1         2         3
    //                          01234567890123456789012345678901234567890
    ParseSession session(file, "Text {\n"
                               " /// some comment\n"
                               " property int foo;\n"
                               "}", 0);
    QVERIFY(session.ast());
    QVERIFY(session.problems().isEmpty());
    QCOMPARE(session.language(), QmlJS::Language::Qml);

    DeclarationBuilder builder(&session);
    ReferencedTopDUContext top = builder.build(file, session.ast());
    QVERIFY(top);

    DUChainReadLocker lock;

    QCOMPARE(top->localDeclarations().size(), 3);        // module, exports, Text
    ClassDeclaration* text = dynamic_cast<ClassDeclaration*>(top->localDeclarations().at(2));
    QVERIFY(text);
    QVERIFY(text->internalContext());
    QCOMPARE(text->internalContext()->type(), DUContext::Class);
    QCOMPARE(text->internalContext()->localDeclarations().size(), 1);
    ClassMemberDeclaration* foo = dynamic_cast<ClassMemberDeclaration*>(text->internalContext()->localDeclarations().first());
    QVERIFY(foo);
    QCOMPARE(foo->identifier().toString(), QString("foo"));
    QVERIFY(foo->abstractType());
    QCOMPARE(foo->abstractType()->toString(), QString("int"));
    QCOMPARE(QString::fromUtf8(foo->comment()), QString("some comment"));
}

#include "testdeclarations.moc"
