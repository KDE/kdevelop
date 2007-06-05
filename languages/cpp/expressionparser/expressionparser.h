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
#include "cppexpressionparserexport.h"

class TranslationUnitAST;
class AST;
namespace KDevelop {
class DUContext;
  
class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionEvaluationResult : public KShared {
  public:
    ExpressionEvaluationResult() : ast(0), isMacro(0) {
    }
    ~ExpressionEvaluationResult() {
      //delete ast;
    }

    AST* ast;
    bool isMacro;
    typedef KSharedPtr<ExpressionEvaluationResult> Pointer;
};

class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionParser {
  public:
    /**
     * Evaluates the type of an expression given as a string within a given context
     *
     * @param exp The expression to evaluate
     * @param context the context within which the expression should be evaluated
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult::Pointer evaluateType( const QByteArray& exp, DUContext* context, bool debug = true );
    /**
     * Evaluates the type of an expression given as an AST.
     *
     * @param ast the AST. It's context must be built already(context-member filled).
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult::Pointer evaluateType( AST* ast, bool debug = true );
};

}
#endif
