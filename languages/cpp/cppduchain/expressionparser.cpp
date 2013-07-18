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

QHash<QByteArray, ExpressionEvaluationResult> buildStaticLookupTable()
{
  QHash<QByteArray, ExpressionEvaluationResult> ret;
  ExpressionEvaluationResult res;
  IntegralType::Ptr type(new IntegralType());

  type->setDataType(IntegralType::TypeBoolean);
  res.type = type->indexed();
  ret.insert("bool", res);

  type->setDataType(IntegralType::TypeChar);
  res.type = type->indexed();
  ret.insert("char", res);

  type->setDataType(IntegralType::TypeFloat);
  res.type = type->indexed();
  ret.insert("float", res);

  type->setDataType(IntegralType::TypeDouble);
  res.type = type->indexed();
  ret.insert("double", res);

  type->setDataType(IntegralType::TypeInt);
  res.type = type->indexed();
  ret.insert("int", res);

  type->setDataType(IntegralType::TypeVoid);
  res.type = type->indexed();
  ret.insert("void", res);

  type->setDataType(IntegralType::TypeWchar_t);
  res.type = type->indexed();
  ret.insert("wchar_t", res);

  ConstantIntegralType::Ptr constType(new ConstantIntegralType);
  constType->setDataType(IntegralType::TypeBoolean);
  constType->setValue<bool>(true);
  res.type = constType->indexed();
  ret.insert("true", res);
  ///NOTE: the trailing space is by intention, apparently thats what gets queried
  ret.insert("true ", res);

  constType->setValue<bool>(false);
  res.type = constType->indexed();
  ret.insert("false", res);
  ///NOTE: the trailing space is by intention, apparently thats what gets queried
  ret.insert("false ", res);

  DelayedType::Ptr ellipsis(new DelayedType);
  ellipsis->setKind(DelayedType::Unresolved);
  ellipsis->setIdentifier(IndexedTypeIdentifier("..."));
  res.type = ellipsis->indexed();
  ret.insert("...", res);

  ///TODO: extend at will

  return ret;
}

bool tryDirectLookup(const QByteArray& unit)
{
  if (unit.isEmpty()) {
    return false;
  }
  if (!std::isalpha(unit.at(0)) && unit.at(0) != '_') {
    return false;
  }
  for (QByteArray::const_iterator it = unit.constBegin() + 1; it != unit.constEnd(); ++it) {
    if (!std::isalnum(*it) && *it != ':' && *it != '_') {
      return false;
    }
  }
  return true;
}

ExpressionEvaluationResult ExpressionParser::evaluateType( const QByteArray& unit, DUContextPointer context, const TopDUContext* source, bool forceExpression ) {

  if( m_debug )
    kDebug(9007) << "==== .Evaluating ..:" << endl << unit;

  // fast path for common built-in types
  static const QHash<QByteArray, ExpressionEvaluationResult> staticLookupTable = buildStaticLookupTable();
  QHash< QByteArray, ExpressionEvaluationResult >::const_iterator it = staticLookupTable.constFind(unit);
  if (it != staticLookupTable.constEnd()) {
    return it.value();
  }

  // fast path for direct lookup of identifiers
  if (!forceExpression && tryDirectLookup(unit)) {
    DUChainReadLocker lock;
    QList< Declaration* > decls = context->findDeclarations(QualifiedIdentifier(QString::fromUtf8(unit)),
                                                            CursorInRevision::invalid(),
                                                            AbstractType::Ptr(),
                                                            source);
    if (!decls.isEmpty()) {
      ExpressionEvaluationResult res;
      foreach(Declaration* decl, decls) {
        res.allDeclarations.append(decl->id());
      }
      res.type = decls.first()->indexedType();
      return res;
    }
  }

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
