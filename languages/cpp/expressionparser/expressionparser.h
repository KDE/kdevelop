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

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <ksharedptr.h>
#include <typesystem.h>
#include "visitor.h"
#include "cppexpressionparserexport.h"

class TranslationUnitAST;
class AST;
namespace KDevelop {
class Declaration;
class DUContext;
};

namespace Cpp  {
using namespace KDevelop;

class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionEvaluationResult : public KShared {
  public:
    ExpressionEvaluationResult() : instanceDeclaration(0) {
    }

    AbstractType::Ptr type; ///Type the expression evaluated to, may be zero when the expression failed to evaluate
    Declaration* instanceDeclaration; ///If the expression evaluates to an instance, this contains a pointer to the instance's declaration(@see CppExpressionVisitor::expressionType())
    
    typedef KSharedPtr<ExpressionEvaluationResult> Ptr;
};

/**
 * A class that simplifies the usage of CppExpressionVisitor by eventually parsing the expression and using CppExpressionVisitor to evaluate it's type
 **/
class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionParser {
  public:
    /**
     * Evaluates the type of an expression given as a string within a given context
     *
     * @param exp The expression to evaluate
     * @param context the context within which the expression should be evaluated
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult::Ptr evaluateType( const QByteArray& exp, DUContext* context, bool debug = true );
    /**
     * Evaluates the type of an expression given as an AST.
     *
     * @param ast the AST. It's context must be built already(context-member filled).
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult::Ptr evaluateType( AST* ast, ParseSession* session, bool debug = true );
};

}
#endif
