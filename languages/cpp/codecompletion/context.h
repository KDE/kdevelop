/*
   Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include <language/duchain/duchainpointer.h>
#include "../cppduchain/typeconversion.h"
#include "../cppduchain/expressionparser.h"
#include "../cppduchain/viablefunctions.h"
#include "../cppduchain/overloadresolutionhelper.h"
#include "../cppduchain/expressionevaluationresult.h"
#include <language/util/includeitem.h>
#include "item.h"
#include <language/codecompletion/codecompletioncontext.h>

namespace KTextEditor {
  class View;
  class Cursor;
}

namespace KDevelop {
  class DUContext;
  class AbstractType;

  class CompletionTreeItem;
  typedef KSharedPtr<CompletionTreeItem> CompletionTreeItemPointer;
}

namespace Cpp {
  class OverloadResolutionFunction;
  class ImplementationHelperItem;

  /**
   * This class is responsible for finding out what kind of completion is needed, what expression should be evaluated for the container-class of the completion, what conversion will be applied to the result of the completion, etc.
   * */
  class CodeCompletionContext : public KDevelop::CodeCompletionContext {
    public:

      ///Computes the full set of completion items, using the information retrieved earlier.
      ///Should only be called on the first context, parent contexts are included in the computations.
      ///@param Abort is checked regularly, and if it is false, the computation is aborted.
      virtual QList<CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true);
      
      virtual QList< KSharedPtr< KDevelop::CompletionTreeElement > > ungroupedElements();

      typedef KSharedPtr<CodeCompletionContext> Ptr;

      typedef OverloadResolutionFunction Function;

      typedef QList<Function> FunctionList;

      enum MemberAccessOperation {
        NoMemberAccess,  ///With NoMemberAccess, a global completion should be done
        MemberAccess,      ///klass.
        ArrowMemberAccess, ///klass->
        StaticMemberChoose, /// Class::
        MemberChoose, /// klass->ParentClass::
        FunctionCallAccess,  ///"function(". Will never appear as initial access-operation, but as parentContext() access-operation.
        TemplateAccess,  ///bla<. Will never appear as initial access-operation, but as parentContext() access-operation.
        SignalAccess,  ///All signals from MemberAccessContainer should be listed
        SlotAccess,     ///All slots from MemberAccessContainer should be listed
        IncludeListAccess, ///A list of include-files should be presented. Get the list through includeItems()
        ReturnAccess,
        DeleteAccess   /// Any item which can be deleted or provide deletable results should be listed
      };
      
      /**
       * The first context created will never be a FunctionCallAccess
       * context. Instead it will at least be a NoMemberAccess. The result completion-list types/return-types should then be matched
       * against any parent FunctionCallAccess'es
       * */

      enum AdditionalContextType {
        Normal,
        FunctionCall,
        BinaryOperatorFunctionCall
      };

      /**
       * @param firstContext should be true for a context that has no parent. Such a context will never be a function-call context.
       * @param text the text to analyze. It usually is the text in the range starting at the beginning of the context, and ending at the position where completion should start
       * @warning The du-chain must be unlocked when this is called
       * @param knownArgumentExpressions has no effect when firstContext is set
       * @param line Optional line that will be used to filter the macros
       * */
      CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, const QString& followingText, const KDevelop::SimpleCursor& position, int depth = 0, const QStringList& knownArgumentExpressions = QStringList(), int line = -1 );
      ~CodeCompletionContext();

      AdditionalContextType additionalContextType() const;

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
       * (the code-completion should list its non-static content).
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
      ExpressionEvaluationResult memberAccessContainer() const;

      /**
       * Returns the internal context of memberAccessContainer, if any.
       *
       * When memberAccessOperation is StaticMemberChoose, this returns all
       * fitting namespace-contexts.
       * */
      QList<DUContext*> memberAccessContainers() const;

      /**
       * When memberAccessOperation is FunctionCallAccess,
       * this returns all functions available for matching, together with the argument-number that should be matched.
       *
       * Operators are treated as functions, but there is special-cases that need to be treated, especially operator=(..), because that operator competes with normal type-conversion.
       *
       * To also respect builtin operators, the types returned by additionalMatchTypes() must be respected.
       * */
      const FunctionList& functions() const;

      /// @return of the function name for this context. Also works for operators.
      QString functionName() const;

      /**
       * When memberAccessOperation is IncludeListAccess, then this contains all the files to be listed.
      * */
      QList<KDevelop::IncludeItem> includeItems() const;
      /**
       *
       * Returns additional potential match-types based on builtin operators(like the = operator)
       *
       * The other match-types are given by functions(), they are the argument-types at the offset Function::matchedArguments.
       *
       * All those types should used to highlight the best matching item in the list created.
       * */
//       QList<KDevelop::AbstractType::Ptr> additionalMatchTypes() const;

      int matchPosition() const;

      KDevelop::IndexedType applyPointerConversionForMatching(KDevelop::IndexedType type, bool fromLValue) const;
      
      QString followingText() const;
      
      void setFollowingText(QString str);
      
      bool isConstructorInitialization();
      
      ///If this is a function call context, this returns the arguments to the function that are already known
      QList<ExpressionEvaluationResult> knownArgumentTypes() const;
      
    private:
      
      enum OnlyShow {
        ShowAll,
        ShowTypes,
        ShowSignals,
        ShowSlots,
        ShowVariables,
        ShowImplementationHelpers
      };
      
      QList<CompletionTreeItemPointer> keywordCompletionItems();
      QList<CompletionTreeItemPointer> getImplementationHelpers();
      QList<CompletionTreeItemPointer> getImplementationHelpersInternal(QualifiedIdentifier minimumScope, DUContext* context);
      bool  filterDeclaration(Declaration* decl, DUContext* declarationContext = 0, bool dynamic = true, bool typeIsConst = false);
      bool  filterDeclaration(ClassMemberDeclaration* decl, DUContext* declarationContext = 0, bool typeIsConst = false);
      ///Replaces the member-access type at the current cursor position from "from" to "new", for example from "->" to "."
      void replaceCurrentAccess(QString old, QString _new);

      ///Computes the completion-items for the case that no special kind of access is used(just a list of all suitable items is needed)
      void standardAccessCompletionItems(QList<CompletionTreeItemPointer>& items);

      void processFunctionCallAccess();

      //Creates the group and adds it to m_storedUngroupedItems if items is not empty
      void eventuallyAddGroup(QString name, int priority, QList< KSharedPtr< KDevelop::CompletionTreeItem > > items);
      
      //Returns the required prefix that is needed in order to find the givne declaration from the current context.
      //In worst case, it is the scope prefix of the declaration.
      QualifiedIdentifier requiredPrefix(Declaration* decl) const;

      ///@param type The type of the argument the items are matched to.
      QList<CompletionTreeItemPointer> specialItemsForArgumentType(AbstractType::Ptr type);
      
      ///Returns whether the declaration is directly visible from within the current context
      bool visibleFromWithin(Declaration* decl, DUContext* currentContext);
      
      ///Returns whether the end of m_text is a valid completion-position
      bool isValidPosition();
      ///Returns whether this is a valid context for implementation helpers
      bool isImplementationHelperValid();
      ///Should preprocess the given text(replace macros with their body etc.)
      void preprocessText( int line );
      void processIncludeDirective(QString line);
      ///Returns whether the given strings ends with an overloaded operator that can form a parent-context
      bool endsWithOperator( const QString& str ) const;
      /**
       * Returns the operator used for an operator function that matches the given string end
       *
       * Example: For "bla[" it returns "[]", for "1 %" it returns "%"
       * */
      QString getEndFunctionOperator( const QString& str ) const;
      /**
       * Returns the exact end of the string that is an operator.
       * Example: For "bla[" it returns "["
       * */
      QString getEndOperator( const QString& str ) const;
      
      ///Does specialized code-completion for constructor-initializers
      ///Returns true if constructor completion is done, and no other completion should be applied
      bool doConstructorCompletion();
      ///Should map a position in m_text to a position in the underlying document
      MemberAccessOperation m_memberAccessOperation;
      QString m_expression;
      QString m_followingText;
      QString m_operator; //If this completion-context ends with a binary operator, this is the operator
      ExpressionEvaluationResult m_expressionResult;

      QList<KDevelop::IncludeItem> m_includeItems;

      QString m_functionName;

      //Here known argument-expressions and their types, that may have come from sub-contexts, are stored
      QStringList m_knownArgumentExpressions;
      QList<ExpressionEvaluationResult> m_knownArgumentTypes;

      AdditionalContextType m_contextType;

      QList<Function> m_functions;

      IndexedDeclaration m_connectedSignal;
      //If a signal/slot access is performed, and a slot is being connected to a signal, this contains the identifier and the signature
      Identifier m_connectedSignalIdentifier;
      QByteArray m_connectedSignalNormalizedSignature;

      DUContextPointer m_localClass;
      
      int m_pointerConversionsBeforeMatching; //0 = No conversion, +1, +2, .. = increase pointer level = &, -1, -2, .. = decrease pointer level = *
      
      QList<KDevelop::CompletionTreeElementPointer> m_storedUngroupedItems;
      
      QList<CompletionTreeItemPointer> m_storedItems; //Used to store pre-computed local completion-items.
      bool m_useStoredItems; //If this is true, m_storedItems will be used instead of computing items, no matter whether it is empty or not.
      OnlyShow m_onlyShow; //A specific completion item type to show, or ShowAll, see enum OnlyShow
      bool m_isDeclarationTypePrefix;//True if the expression is set to the type-part of a declaration like "int i"
      bool m_isConstructorCompletion;
      bool m_doAccessFiltering;

      friend class ImplementationHelperItem;
  };
}

#endif
