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
#include "visitor.h"
#include "cppexpressionparserexport.h"
#include "expressionvisitor.h"

class TranslationUnitAST;
class AST;
namespace KDevelop {
class Declaration;
class DUContext;
}

namespace Cpp  {
using namespace KDevelop;

class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionEvaluationResult {
  public:
    ExpressionEvaluationResult() {
    }

    AbstractType::Ptr type; ///Type the expression evaluated to, may be zero when the expression failed to evaluate
    ExpressionVisitor::Instance instance; ///If the expression evaluates to an instance, this contains a pointer to the instance's declaration(@see CppExpressionVisitor::expressionType())

    QList<Declaration*> allDeclarations; ///If type is a function-type, this may contain the declarations of all found overloaded functions

    ///@return whether the result is an lvalue
    bool isLValue() const;

    ///@return whether this result is valid(has a type)
    bool isValid() const;

    ///it does not matter whether du-chain is locked or not
    QString toString() const;

    typedef KSharedPtr<ExpressionEvaluationResult> Ptr;
};

/**
 * A class that simplifies the usage of CppExpressionVisitor by eventually parsing the expression and using CppExpressionVisitor to evaluate it's type
 **/
class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionParser {
  public:
     /**
     * @param strict When this is false, the expression-visitor tries to recover from problems. For example when it cannot find a matching function, it returns the first of the candidates.
     * @param debug Enables additional output
     * */

    explicit ExpressionParser( bool strict = false, bool debug = true );
    /**
     * Evaluates the type of an expression given as a string within a given context
     *
     * @param exp The expression to evaluate
     * @param context the context within which the expression should be evaluated
     * @param statement whether the text should be parsed as a statement. If this is false, the text will need to be a valid translation-unit.
     * @param debug whether additional output to kdDebug should be issued
     *
     * Note: Even expressions that create sub-contexts should work,
     * Example:
     * "int i; i += 5;"
    */
    ExpressionEvaluationResult evaluateType( const QByteArray& expression, DUContextPointer context, bool statement = true );
    /**
     * Evaluates the type of an expression given as an AST.
     *
     * @param ast the AST. It's context must be built already(context-member filled).
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult evaluateType( AST* ast, ParseSession* session );

  private:
    bool m_strict;
    bool m_debug;
};

}
#endif
