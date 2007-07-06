/* This file is part of KDevelop
    Copyright (C) 2007 David Nolden [david.nolden.kdevelop  art-master.de]

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

#include <duchain.h>
#include "duchainbuilder/usebuilder.h"
#include "duchainbuilder/declarationbuilder.h"
#include "duchainbuilder/dumpchain.h"
#include "duchainbuilder/dumptypes.h"
#include <declaration.h>

#include "ducontext.h"
#include "ast.h"
#include "parsesession.h"
#include "parser.h"
#include "control.h"
#include "duchainlock.h"
//#include "typerepository.h"
#include <identifier.h>
#include "expressionvisitor.h"


namespace Cpp {
using namespace KDevelop;

bool ExpressionEvaluationResult::isValid() const {
  return (bool)type;
}

bool ExpressionEvaluationResult::isLValue() const {
  return instance && (instance.declaration || dynamic_cast<const ReferenceType*>( type.data() ));
}

QString ExpressionEvaluationResult::toString() const {
  if( DUChain::lock()->currentThreadHasReadLock() )
    return QString(isLValue() ? "lvalue " : "") + QString(instance ? "instance " : "") + (type ? type->toString() : QString("<no type>"));
  
  DUChainReadLocker lock(DUChain::lock());
  return QString(isLValue() ? "lvalue " : "") + QString(instance ? "instance " : "") + (type ? type->toString() : QString("<no type>"));
}

ExpressionParser::ExpressionParser( bool strict, bool debug ) : m_strict(strict), m_debug(debug) {
}

ExpressionEvaluationResult ExpressionParser::evaluateType( const QByteArray& unit, DUContext* context, bool statement ) {

  if( m_debug )
    kDebug() << "==== .Evaluating ..:" << endl << unit << endl << endl;

  ParseSession* session = new ParseSession();

  Control control;
  DumpChain dumper;
  
  Parser parser(&control);
  
  AST* ast = 0;

  DUContext::ContextType type;
  {
    DUChainReadLocker lock(DUChain::lock());
    type = context->type();
  }

  if( statement ) {
      session->setContents("{" + unit + ";}");
      ast = parser.parseStatement(session);
  } else {
      session->setContents(unit);
      ast = parser.parse(session);
      ((TranslationUnitAST*)ast)->session = session;
  }

  if (m_debug) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, session);
  }

  ast->ducontext = context;
  
  ///@todo think how useful it is to compute contexts and uses here. The main thing we need is the AST.
  /*
  static int testNumber = 0; //@todo what this url for?
  KUrl url(QString("file:///internal/evaluate_%1").arg(testNumber++));
  kDebug() << "url: " << url << endl;

  DeclarationBuilder definitionBuilder(session);
  DUContext* top = definitionBuilder.buildSubDeclarations(url, ast, context);

  UseBuilder useBuilder(session);
  useBuilder.buildUses(ast);

  if (m_debug) {
    kDebug() << "===== DUChain:" << endl;

    DUChainReadLocker lock(DUChain::lock());
    dumper.dump(top, false);
  }

  if (m_debug) {
    kDebug() << "===== Types:" << endl;
    DumpTypes dt;
    DUChainReadLocker lock(DUChain::lock());
    foreach (const AbstractType::Ptr& type, definitionBuilder.topTypes())
      dt.dump(type.data());
  }

  if (m_debug)
    kDebug() << "===== Finished evaluation." << endl;
  */
  ExpressionEvaluationResult ret = evaluateType( ast, session );

  delete session;

  /*
  {
    DUChainReadLocker lock(DUChain::lock());
    delete top;
  }*/

  return ret;
}

ExpressionEvaluationResult ExpressionParser::evaluateType( AST* ast, ParseSession* session) {
  ExpressionEvaluationResult ret;
  ExpressionVisitor v(session, m_strict);
  v.parse( ast );
  ret.type = v.lastType();
  ret.instance = v.lastInstance();
  ret.allDeclarations = v.lastDeclarations();
  return ret;
}

}
