/* 
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

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

#ifndef CODECOMPLETIONCONTEXT_H
#define CODECOMPLETIONCONTEXT_H

#include <ktexteditor/cursor.h>
#include <ksharedptr.h>
#include "expressionparser/typeconversion.h"
#include "expressionparser/expressionparser.h"
#include "expressionparser/viablefunctions.h"
#include <typesystem.h>

namespace KTextEditor {
  class View;
  class Cursor;
}

namespace KDevelop {
  class DUContext;
  class AbstractType;
}

namespace Cpp {

  /**
   * This class is responsible for finding out what kind of completion is needed, what expression should be evaluated for the container-class of the completion, what conversion will be applied to the result of the completion, etc.
   * */
  class CodeCompletionContext : public KShared {
    public:

      struct Function {
        Function();
        Function( int _matchedArguments, const ViableFunction& _viable );

        /**The count of arguments that are already matched in this function. The argument of the function at this offset is the next one to be matched.
         *This is variable, and needed so global binary overloaded operators and member binary
         *operators can be treated same(global operators take 2 arguments, local
         *operators only 1)
         * */
        int matchedArguments;

        ///Result of matching the function to available arguments. Also contains the declaration.
        ViableFunction function;
      };

      typedef QList<Function> FunctionList;
      
      enum MemberAccessOperation {
        NoMemberAccess,  ///With NoMemberAccess, a global completion should be done
        MemberAccess,      ///klass.
        ArrowMemberAccess, ///klass->
        StaticMemberChoose, /// Class::
        MemberChoose, /// klass->ParentClass::
        FunctionCallAccess,  ///"function(". Will never appear is initial access-operation, but as parentContext() access-operation.
        SignalAccess,  ///All signals from MemberAccessContainer should be listed
        SlotAccess     ///All slots from MemberAccessContainer should be listed
      };
      /**
       * The first context created will never be a FunctionCallAccess
       * context. Instead it will at least be a NoMemberAccess. The result completion-list types/return-types should then be matched
       * against any parent FunctionCallAccess'es
       * */

      enum SpecificContextType {
        Normal,
        FunctionCall,
        BinaryOperatorFunctionCall
      };

      /**
       * @param firstContext should be true for a context that has no parent. Such a context will never be a function-call context.
       * @param text the text to analyze. It usually is the text in the range starting at the beginning of the context, and ending at the position where completion should start
       * @param knownArgumentExpressions has no effect when firstContext is set
       * */
      CodeCompletionContext(KDevelop::DUContext * context, const QString& text, bool firstContext = true, const QStringList& knownArgumentExpressions = QStringList() );
      ~CodeCompletionContext();

      ///@return whether this context is valid for code-completion
      operator bool() const;

      /**In the case of recursive argument-hints, there may be a chain of parent-contexts, each for the higher argument-matching
       * The parentContext() should always have the access-operation FunctionCallAccess.
       * When a completion-list is computed, the members of the list can be highlighted that match the corresponding parentContext()->functions() function-argument, or parentContext()->additionalMatchTypes()
       * */
      CodeCompletionContext* parentContext();

      ///@return the used access-operation
      MemberAccessOperation memberAccessOperation() const;
      
      /**
       * When the access-operation is a MemberAccess or ArrowMemberAccess, this
       * is the container that completion should happen in
       * (the code-completion should list it's non-static content).
       *
       * When memberAccessOperation is StaticMemberChoose, the code-completion
       * should list all static members of this container.
       *
       * When memberAccessOperation is MemberChoose, it should be treated equivalently to MemberAccess.
       * 
       * The type does not respect the member-access-operation, so
       * the code-completion may check whether the arrow-access was used correctly
       * and maybe do automatic correction.
       * @return the type of the container that should be completed in.
       * */
      ExpressionEvaluationResult::Ptr memberAccessContainer() const;

      /**
       * When memberAccessOperation is FunctionCallAccess,
       * this returns all functions available for matching.
       *
       * Operators are treated as functions, but there is special-cases that need to be treated, especially operator=(..), because that operator competes with normal type-conversion.
       *
       * So when completing, the builtin operator= using type-conversion should be respected from outside.
       * */
      const FunctionList& functions() const;

      /**
       *
       * Returns additional potential match-types based on builtin operators(like the = operator)
       *
       * The other match-types are given by functions(), they are the argument-types at the offset Function::matchedArguments.
       *
       * All those types should used to highlight the best matching item in the list created.
       * */
      QList<KDevelop::AbstractType::Ptr> additionalMatchTypes() const;
    private:
      ///@return the extracted expression, without any access-operation
      QString expression() const;

      void processFunctionCallAccess();
      
      ///Returns whether the end of m_text is a valid completion-position
      bool isValidPosition();
      ///Should preprocess the given text(replace macros with their body etc.)
      void preprocessText();
      void log( const QString& str ) const;
      ///Returns whether the given strings ends with an overloaded operator that can form a parent-context
      bool endsWithOperator( const QString& str ) const;
      QString getEndOperator( const QString& str ) const;
      ///Should map a position in m_text to a position in the underlying document
      MemberAccessOperation m_memberAccessOperation;
      bool m_valid;
      QString m_expression;
      QString m_operator; //If this completion-context ends with a binary operator, this is the operator
      ExpressionEvaluationResult::Ptr m_expressionResult;

      QString m_text;

      //Here known argument-expressions and their types, that may have come from sub-context's, are stored
      QStringList m_knownArgumentExpressions;
      QList<ExpressionEvaluationResult::Ptr> m_knownArgumentTypes;
      
      KDevelop::DUContext* m_duContext;
      SpecificContextType m_contextType;

      QList<Function> m_functions;

      KSharedPtr<CodeCompletionContext> m_parentContext;
  };
}

#endif
