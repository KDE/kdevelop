/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "expressionparser.h"

#include <language/duchain/duchain.h>
#include "cppduchain/usebuilder.h"
#include "cppduchain/declarationbuilder.h"
#include "cppduchain/dumpchain.h"
#include "cppduchain/dumptypes.h"
#include <language/duchain/declaration.h>

#include <language/duchain/ducontext.h>
#include "ast.h"
#include "parsesession.h"
#include "parser.h"
#include "control.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/identifier.h>
#include "expressionvisitor.h"
#include <parser/rpp/chartools.h>


namespace Cpp {
using namespace KDevelop;

ExpressionParser::ExpressionParser( bool strict, bool debug, bool propagateConstness, bool mapAst )
: m_strict(strict)
, m_debug(debug)
, m_propagateConstness(propagateConstness)
, m_mapAst(mapAst)
{
}

ExpressionEvaluationResult ExpressionParser::evaluateType( const QByteArray& unit, DUContextPointer context, const TopDUContext* source, bool forceExpression ) {

  if( m_debug )
    kDebug(9007) << "==== .Evaluating ..:" << endl << unit;

  ParseSession session;

  Control control;
  DumpChain dumper;

  Parser parser(&control);

  AST* ast = 0;

  session.setContentsAndGenerateLocationTable(tokenizeFromByteArray(unit));

  ast = parser.parseTypeOrExpression(&session, forceExpression);

  if(!ast) {
    kDebug(9007) << "Failed to parse \"" << unit << "\"";
    return ExpressionEvaluationResult();
  }

  if (m_debug) {
    kDebug(9007) << "===== AST:";
    dumper.dump(ast, &session);
  }

  ast->ducontext = context.data();

  if(!ast->ducontext) {
    kDebug() << "context disappeared";
    return ExpressionEvaluationResult();
  }
  
  ExpressionEvaluationResult ret = evaluateType( ast, &session, source );


  return ret;
}

ExpressionEvaluationResult ExpressionParser::evaluateExpression( const QByteArray& expression, DUContextPointer context, const TopDUContext* source )
{
  return evaluateType( expression, context, source, true );
}

ExpressionEvaluationResult ExpressionParser::evaluateType( AST* ast, ParseSession* session, const TopDUContext* source )
{
  if (m_debug) {
    DumpChain dumper;
    kDebug(9007) << "===== AST:";
    dumper.dump(ast, session);
  }

  ExpressionEvaluationResult ret;
  ExpressionVisitor v(session, source, m_strict, m_propagateConstness, m_mapAst);
  v.parse( ast );

  DUChainReadLocker lock(DUChain::lock());

  ret.type = v.lastType()->indexed();
  ret.isInstance = v.lastInstance().isInstance;

  if(v.lastInstance().declaration)
    ret.instanceDeclaration = DeclarationId(IndexedDeclaration(v.lastInstance().declaration.data()));

  foreach(const DeclarationPointer &decl, v.lastDeclarations()) {
    if(decl)
      ret.allDeclarations.append(decl->id());
  }

  return ret;
}

}
