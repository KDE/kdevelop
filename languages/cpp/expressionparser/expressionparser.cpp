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

ExpressionEvaluationResult::Ptr ExpressionParser::evaluateType( const QByteArray& unit, DUContext* context, bool debug ) {

  if( debug )
    kDebug() << "==== .Evaluating ..:" << endl << unit << endl << endl;

  ParseSession* session = new ParseSession();
  session->setContents(unit);

  Control control;
  DumpChain dumper;
  
  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session);
  ast->session = session;

  if (debug) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, session);
  }

  static int testNumber = 0; //@todo what this url for?
  KUrl url(QString("file:///internal/%1").arg(testNumber++));

  QList<KDevelop::DUContext*> includes;
  includes << context;
  
  DeclarationBuilder definitionBuilder(session);
  TopDUContext* top = definitionBuilder.buildDeclarations(url, ast, &includes);

  UseBuilder useBuilder(session);
  useBuilder.buildUses(ast);

  if (debug) {
    kDebug() << "===== DUChain:" << endl;

    DUChainWriteLocker lock(DUChain::lock());
    //dumper.dump(top, false);
  }

  if (debug) {
    kDebug() << "===== Types:" << endl;
    DumpTypes dt;
    foreach (const AbstractType::Ptr& type, definitionBuilder.topTypes())
      dt.dump(type.data());
  }

  if (debug)
    kDebug() << "===== Finished evaluation." << endl;

  ExpressionEvaluationResult::Ptr ret = evaluateType( ast, session, debug );

  delete session;

  return ret;
}

ExpressionEvaluationResult::Ptr ExpressionParser::evaluateType( AST* ast, ParseSession* session, bool debug ) {
  ExpressionEvaluationResult::Ptr ret( new ExpressionEvaluationResult );
  ExpressionVisitor v(session);
  v.parse( ast );
  ret->type = v.lastType();
  ret->instanceDeclaration = v.lastDeclaration();
  return ret;
}

}