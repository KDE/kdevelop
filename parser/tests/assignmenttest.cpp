/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "assignmenttest.h"

#include "qmakedriver.h"
#include "ast.h"

#include "testhelpers.h"

QTEST_MAIN( AssignmentTest )

AssignmentTest::AssignmentTest( QObject* parent )
    : QObject( parent ), ast(0)
{}

AssignmentTest::~AssignmentTest()
{}

BEGINTESTFUNCIMPL( AssignmentTest, simpleParsed, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VAR", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL(AssignmentTest, simpleParsed, "VAR = VALUE\n" )

BEGINTESTFUNCIMPL( AssignmentTest, assignInValue, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL(AssignmentTest, assignInValue, "VARIABLE = value1=value++\n")

BEGINTESTFUNCIMPL( AssignmentTest, commentCont, 1 )
    QMake::AssignmentAST* assignment;
    assignment= dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 2 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, commentCont, "VARIABLE = foobar\\#somecomment\nnextval\n" )

BEGINTESTFUNCIMPL( AssignmentTest, quotedBrace, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, quotedBrace, "VARIABLE += \" value ( \"\n" )

BEGINTESTFUNCIMPL( AssignmentTest, quotedVal, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, quotedVal, "VARIABLE += \" value \"\n" )

BEGINTESTFUNCIMPL( AssignmentTest, quotedValEscapeQuote, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
    qDebug() << assignment->values.count();
    foreach( QMake::ValueAST* v, assignment->values )
        qDebug() << v->value;
TESTASSIGNMENT( assignment, "VARIABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, quotedValEscapeQuote, "VARIABLE += \" va\\\"lue \"\n" )

BEGINTESTFUNCIMPL( AssignmentTest, dotVariable, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VAR.IABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, dotVariable, "VAR.IABLE += value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, underscoreVariable, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VAR_IABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, underscoreVariable, "VAR_IABLE += value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, dashVariable, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VAR-IABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, dashVariable, "VAR-IABLE += value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, opNoSpace, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, opNoSpace, "VARIABLE+=value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, opPlusEq, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, opPlusEq, "VARIABLE += value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, opTildeEq, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "~=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, opTildeEq, "VARIABLE ~= value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, opStarEq, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "*=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, opStarEq, "VARIABLE *= value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, opMinusEq, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "-=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, opMinusEq, "VARIABLE -= value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, opEqual, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, opEqual, "VARIABLE = value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, otherVarSimple, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, otherVarSimple, "VARIABLE = $$OTHERVAR\n" )

BEGINTESTFUNCIMPL( AssignmentTest, otherVarBrace, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, otherVarBrace, "VARIABLE = $${OTHERVAR}\n" )

BEGINTESTFUNCIMPL( AssignmentTest, otherVarBracket, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, otherVarBracket, "VARIABLE = $$[OTHERVAR]\n" )

BEGINTESTFUNCIMPL( AssignmentTest, shellVar, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, shellVar, "VARIABLE = $(OTHERVAR)\n" )

BEGINTESTFUNCIMPL( AssignmentTest, commandExec, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, commandExec, "VARIABLE = ( cd /home && ls $(SHELLVAR) );\n" )

BEGINTESTFUNCIMPL( AssignmentTest, bracketVarQuote, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, bracketVarQuote, "VARIABLE = \"$${VAR}\"\n" )

BEGINTESTFUNCIMPL( AssignmentTest, pathValue, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, pathValue, "VARIABLE = $${VAR}/html/*\n" )

BEGINTESTFUNCIMPL( AssignmentTest, commandExecQMakeVar, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, commandExecQMakeVar, "VARIABLE = (ls $$VAR/html/*);\n" )

BEGINTESTFUNCIMPL( AssignmentTest, varComment, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VARIABLE", "=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, varComment, "VARIABLE = value #comment\n" )

BEGINTESTFUNCIMPL( AssignmentTest, varNameDollar, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "$$VAR", "+=", 1 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, varNameDollar, "$$VAR += value\n" )

BEGINTESTFUNCIMPL( AssignmentTest, commentInMultiline, 1 )
    QMake::AssignmentAST* assignment;
    assignment = dynamic_cast<QMake::AssignmentAST*>( ast->statements.first() );
TESTASSIGNMENT( assignment, "VAR", "+=", 3 )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, commentInMultiline, "VAR += foo \\\n  bar \\\n# ass\n  foo")

//---------------- HERE start the expected fails

BEGINTESTFAILFUNCIMPL( AssignmentTest, contAfterComment,
    "No Comments before a continuation character" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, contAfterComment, "VAR- += value#comment\\\n" )


BEGINTESTFAILFUNCIMPL( AssignmentTest, noDashEndVar,
    "No dash at the end of variable names" )
ENDTESTFUNCIMPL

DATAFUNCIMPL( AssignmentTest, noDashEndVar, "VAR- += value\n" )


void AssignmentTest::init()
{
    ast = new QMake::ProjectAST();
    QVERIFY( ast != 0 );
}

void AssignmentTest::cleanup()
{
    delete ast;
    ast = 0;
    QVERIFY( ast == 0 );
}
