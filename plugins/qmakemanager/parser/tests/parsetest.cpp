/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*

Some notes for special cases to test:


somescope:anotherscope {
}

scope | scope : SOMEVAR = foo

eval($${subdir}.depends = $$basename($${subdir})/$(MAKEFILE)
eval( $$somefunc( $$VAR, else ) ) {
} else {
}

scopename:  (no statement, is accepted by qmake)



*/

#include "parsetest.h"
#include "qmakedriver.h"
#include "ast.h"
#include "testhelpers.h"

QTEST_GUILESS_MAIN(ParseTest)

ParseTest::ParseTest(QObject* parent)
    : QObject(parent)
{
}

ParseTest::~ParseTest()
{
}

BEGINTESTFUNCIMPL(ParseTest, successSimpleProject, 2)
ENDTESTFUNCIMPL

DATAFUNCIMPL(ParseTest, successSimpleProject, "VAR = VALUE\nfunc1(arg1)\n")

BEGINTESTFAILFUNCIMPL(ParseTest, failSimpleProject, "Missing value for variable and no lineending")
ENDTESTFUNCIMPL

DATAFUNCIMPL(ParseTest, failSimpleProject, "foo(")

BEGINTESTFUNCIMPL(ParseTest, successFullProject, 10)
ENDTESTFUNCIMPL

DATAFUNCIMPL(ParseTest, successFullProject,
             "#Comment\n"
             "VARIABLE1 = Value1 Value2\n"
             "VARIABLE2= Value1 Value2\n"
             "VARIABLE3 =Value1 Value2\n"
             "VARIABLE4=Value1 Value2\n"
             "VARIABLE = Value1 Value2 #some comment\n"
             "VARIABLE = $$Value1 $(Value2) $${Value3} #some comment\n"
             "VARIABLE = $$Value1 $(Value2) $${Value3} \\\nValue4\n"
             "message( foo, bar, $$foobar( foo, $$FOOBAR ), $${FOOBAR}, $(SHELL) ) : FO=0\n"
             "message( foo, bar, $$foobar( foo, $$FOOBAR ), $${FOOBAR}, $(SHELL) ) {  \n"
             "FOO = bar\n"
             "}\n"
             "!do()\n")

BEGINTESTFAILFUNCIMPL(ParseTest, failFullProject, "Missing closing brace in scope for fo()")
ENDTESTFUNCIMPL

DATAFUNCIMPL(ParseTest, failFullProject, "#Comment\n"
                                         "VARIABLE1 = Value1 Value2\n"
                                         "VARIABLE2= Value1 Value2\n"
                                         "VARIABLE3 =Value1 Value2\n"
                                         "VARIABLE4=Value1 Value2\n"
                                         "VARIABLE4=Value1 Value2 \\\n"
                                         "  Value3 Value4\n"
                                         "fo()\n{\n"
                                         "VARIABLE = Value1 Value2 \\#some comment\n"
                                         "win32 : FOOBAR=Value1\n"
                                         "fun1()|!fun2(): FOOBAR=Value1\n")

void ParseTest::init()
{
    QVERIFY(ast == nullptr);
    ast = new QMake::ProjectAST();
    QVERIFY(ast != nullptr);
}

void ParseTest::cleanup()
{
    delete ast;
    ast = nullptr;
    QVERIFY(ast == nullptr);
}
