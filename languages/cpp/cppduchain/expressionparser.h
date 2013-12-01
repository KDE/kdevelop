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

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <language/duchain/duchainpointer.h>
#include "cppduchainexport.h"

class ParseSession;
class AST;
namespace KDevelop {
class Declaration;
class DUContext;
}

namespace Cpp  {
using namespace KDevelop;

class ExpressionEvaluationResult;

/**
 * A class that simplifies the usage of CppExpressionVisitor by eventually parsing the expression and using CppExpressionVisitor to evaluate its type
 **/
class KDEVCPPDUCHAIN_EXPORT ExpressionParser {
  public:
     /**
     * @param strict When this is false, the expression-visitor tries to recover from problems.
     *               For example when it cannot find a matching function, it returns the first of the candidates.
     * @param debug  Enables additional output
     * @param propagateConstness When this is set to true, the expression visitor will propagate the constness
     *                           in member accesses. Required for decltype support, i.e.:
     *                           'const A* a; decltype((a->x)) b;', here b should be const
     * */

    explicit ExpressionParser( bool strict = false, bool debug = false, bool propagateConstness = false, bool mapAst = false );
    /**
     * Evaluates the type of an expression given as a string within a given context.
     * The expression can either be a simple type-id, or a valid C++ expression.
     * If the expression may either represent a type-id, or an expression, the type-id is chosen, unless forceExpression is true.
     *
     * Unfortunately, the parser accepts some expressions like "d->bla = 5" as type-ids with the name "d", so forceExpression
     * should be used whenever possible.
     *
     * This function should be perfect for places in C++ where either a type-id, or a static expression are allowed, like template-arguments.
     *
     * The duchain does not strictly need to be locked when this is called, but it should be locked if the entity evaluating the expression has no control over
     * the lifetime of @p context 
     *
     * @param exp The expression to evaluate
     * @param context the context within which the expression should be evaluated
     * @param forceExpression do not consider the expression to be a type-id
     * @param debug whether additional output to kdDebug should be issued
     *
     *
    */
    ExpressionEvaluationResult evaluateType( const QByteArray& expression, DUContextPointer context, const KDevelop::TopDUContext* source = 0, bool forceExpression = false );

    /**
     * Same as evaluateType, except that it does not consider type-ids, only expressions.
     *
     * The duchain does not strictly need to be locked when this is called, but it should be locked if the entity evaluating the expression has no control over
     * the lifetime of @p context 
     *
     * Equivalent with calling evaluateType(.., .., true), but should be preferred for better overview.
     * */
    ExpressionEvaluationResult evaluateExpression( const QByteArray& expression, DUContextPointer context, const KDevelop::TopDUContext* source = 0 );
    /**
     * Evaluates the type of an expression given as an AST.
     *
     * The duchain does not strictly need to be locked when this is called, but it should be locked if the entity evaluating the expression has no control over
     * the lifetime of ast->context
     *
     * @param ast the AST. @warning its ducontext must be built already, the ducontext member variable must be filled.
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult evaluateType( AST* ast, ParseSession* session, const KDevelop::TopDUContext* source = 0 );

  private:
    bool m_strict;
    bool m_debug;
    bool m_propagateConstness;
    bool m_mapAst;
};

}
#endif
