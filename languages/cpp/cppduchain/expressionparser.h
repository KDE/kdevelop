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

#include <ksharedptr.h>
#include <typesystem.h>
#include <duchain/duchainpointer.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/ducontext.h>
#include "visitor.h"
#include "cppduchainexport.h"
#include "expressionvisitor.h"

class TranslationUnitAST;
class AST;
namespace KDevelop {
class Declaration;
class DUContext;
}

namespace Cpp  {
using namespace KDevelop;

/**
 *
 * The functions in ExpressionEvaluationResult are inlined for now, so they can be used from within the duchainbuilder module
 * (It's a circular dependency)
 * */
class KDEVCPPDUCHAIN_EXPORT ExpressionEvaluationResult {
  public:
    ExpressionEvaluationResult() {
    }

    AbstractType::Ptr type; ///Type the expression evaluated to, may be zero when the expression failed to evaluate
    ExpressionVisitor::Instance instance; ///If the expression evaluates to an instance, this contains a pointer to the instance's declaration(@see CppExpressionVisitor::expressionType())

    QList<DeclarationPointer> allDeclarations; ///This contains the declarations found for the item evaluated.

    ///@return whether the result is an lvalue
    bool isLValue() const {
      return instance && (instance.declaration || dynamic_cast<const ReferenceType*>( type.data() ));
    }

    ///@return whether this result is valid(has a type)
    bool isValid() const {
      return (bool)type;
    }

    ///@return A short version, that only contains the name or value, without instance-information etc.
    QString toShortString() const
    {
      //Inline for now, so it can be used from the duchainbuilder module
      if( DUChain::lock()->currentThreadHasReadLock() )
        return type ? type->toString() : QString("<no type>");

      DUChainReadLocker lock(DUChain::lock());
      return type ? type->toString() : QString("<no type>");
    }
    
    ///it does not matter whether du-chain is locked or not
    QString toString() const {
      if( DUChain::lock()->currentThreadHasReadLock() )
        return QString(isLValue() ? "lvalue " : "") + QString(instance ? "instance " : "") + (type ? type->toString() : QString("<no type>"));

      DUChainReadLocker lock(DUChain::lock());
      return QString(isLValue() ? "lvalue " : "") + QString(instance ? "instance " : "") + (type ? type->toString() : QString("<no type>"));
    }

    typedef KSharedPtr<ExpressionEvaluationResult> Ptr;
};

/**
 * A class that simplifies the usage of CppExpressionVisitor by eventually parsing the expression and using CppExpressionVisitor to evaluate its type
 **/
class KDEVCPPDUCHAIN_EXPORT ExpressionParser {
  public:
     /**
     * @param strict When this is false, the expression-visitor tries to recover from problems. For example when it cannot find a matching function, it returns the first of the candidates.
     * @param debug Enables additional output
     * */

    explicit ExpressionParser( bool strict = false, bool debug = false );
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
     * @param exp The expression to evaluate
     * @param context the context within which the expression should be evaluated
     * @param forceExpression do not consider the expression to be a type-id
     * @param debug whether additional output to kdDebug should be issued
     *
     * 
    */
    ExpressionEvaluationResult evaluateType( const QByteArray& expression, DUContextPointer context, const KDevelop::ImportTrace& trace = KDevelop::ImportTrace(), bool forceExpression = false );

    /**
     * Same as evaluateType, except that it does not consider type-ids, only expressions.
     * Equivalent with calling evaluateType(.., .., true), but should be preferred for better overview.
     * */
    ExpressionEvaluationResult evaluateExpression( const QByteArray& expression, DUContextPointer context, const KDevelop::ImportTrace& trace = KDevelop::ImportTrace() );
    /**
     * Evaluates the type of an expression given as an AST.
     *
     * @param ast the AST. Its context must be built already, the context-member must be filled.
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult evaluateType( AST* ast, ParseSession* session, const KDevelop::ImportTrace& trace = KDevelop::ImportTrace() );

  private:
    bool m_strict;
    bool m_debug;
};

}
#endif
