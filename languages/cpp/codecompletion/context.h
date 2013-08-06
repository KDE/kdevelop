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
  typedef QPair<Declaration*, bool> DeclAccessPair;
}

namespace Cpp {
  class OverloadResolutionFunction;
  class ImplementationHelperItem;

  #ifdef TEST_COMPLETION
    int expressionBefore( const QString& _text, int index );
  #endif

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

      enum AccessType {
        NoMemberAccess,       /// With NoMemberAccess, a global completion should be done
        MemberAccess,         /// "Class."
        ArrowMemberAccess,    /// "Class->"
        StaticMemberChoose,   /// "Class::"
        MemberChoose,         /// "Class->ParentClass::"
        SignalAccess,         /// All signals from MemberAccessContainer should be listed
        SlotAccess,           /// All slots from MemberAccessContainer should be listed

        IncludeListAccess,    /// A list of include-files should be presented. Get the list through includeItems()

        /// The following will never appear as initial accessType, but as a parentContext()
        FunctionCallAccess,   /// "function("
        BinaryOpFunctionCallAccess, /// "var1 {somebinaryoperator} "
        TemplateAccess,       /// "bla<."
        ReturnAccess,         /// "return " -- Takes into account return type
        CaseAccess,           /// "case " -- Takes into account switch expression type
        NamespaceAccess
      };

      /**
       * @param firstContext should be true for a context that has no parent. Such a context will never be a function-call context.
       * @param text the text to analyze. It usually is the text in the range starting at the beginning of the context, and ending at the position where completion should start
       * @warning The du-chain must be unlocked when this is called
       * @param knownArgumentExpressions has no effect when firstContext is set
       * @param line Optional line that will be used to filter the macros
       * */
      CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, const QString& followingText, const KDevelop::CursorInRevision& position, int depth = 0, const QStringList& knownArgumentExpressions = QStringList(), int line = -1 );
      ~CodeCompletionContext();

      /**In the case of recursive argument-hints, there may be a chain of parent-contexts, each for the higher argument-matching
       * The parentContext() should always have the access-operation FunctionCallAccess.
       * When a completion-list is computed, the members of the list can be highlighted that match the corresponding parentContext()->functions() function-argument, or parentContext()->additionalMatchTypes()
       * */
      CodeCompletionContext* parentContext() const;

      ///@return the used access-operation
      AccessType accessType() const;
      ///@return the list of IndexedTypes that this CodeCompletionContext expects
      QList<IndexedType> matchTypes();

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
       * *DUChain must be locked*
       * */
      QSet<DUContext*> memberAccessContainers() const;

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

      int pointerConversions() const;
      
      QString followingText() const;
      
      void setFollowingText(QString str);
      
      bool isConstructorInitialization();
      
      ///If this is a function call context, this returns the arguments to the function that are already known
      QList<ExpressionEvaluationResult> knownArgumentTypes() const;

      ///Returns position of the argument being matched
      int matchPosition() const;

#ifndef TEST_COMPLETION
    private:
#endif
      enum OnlyShow {
        ShowAll,
        ShowIntegralConstants, // compile-time integral constants and related items (scopes, ...)
        ShowTypes,
        ShowSignals,
        ShowSlots,
        ShowVariables,
        ShowImplementationHelpers
      };
#ifdef TEST_COMPLETION
      OnlyShow onlyShow() { return m_onlyShow; };
    private:
#endif
      ///Preprocess m_text (replace macros with their body etc.)
      void preprocessText(int line);

      ///looks at @param str to determine current context
      ///*DUChain must be locked*
      AccessType findAccessType(const QString &str) const;

      ///Get local class from m_duContext, if available
      ///*DUChain must be locked*
      DUContextPointer findLocalClass() const;

      ///Find if this context should limit completions to certain kinds
      ///*DUChain must be locked*
      OnlyShow findOnlyShow(const QString &accessStr) const;

      ///Get the types for m_knownArgumentExpressions
      ///*DUChain must be locked*
      QList<ExpressionEvaluationResult> getKnownArgumentTypes() const;

      ///Looks in m_text to find @param expression, @param expressionPrefix,
      ///and whether the expression @param istypeprefix
      ///*DUChain must be locked*
      void findExpressionAndPrefix(QString &expression, QString &expressionPrefix, bool &isTypePrefix) const;

      ///Create and return a parent context for the given @param expressionPrefix
      KSharedPtr<KDevelop::CodeCompletionContext> getParentContext(const QString &expressionPrefix) const;

      ///Evaluate m_expression
      ExpressionEvaluationResult evaluateExpression() const;

      ///Remove unary operators from the end of m_text, setting m_pointerConversionsBeforeMatching accordingly
      void skipUnaryOperators(QString &str, int &pointerConversions) const;

      ///test if the context is valid for its accessType
      bool testContextValidity(const QString &expressionPrefix, const QString &accessStr) const;

      /**
       * Specialized processing for access types
       * *DUChain must be locked for these functions*
      **/
      void processArrowMemberAccess();
      void processFunctionCallAccess();
      void processAllMemberAccesses();

      ///Whether or not this context should add parent items
      bool shouldAddParentItems(bool fullCompletion);

      /**
      * Item creation functions for various completion types
      **/
      ///*DUChain must be locked*
      QList<CompletionTreeItemPointer> keywordCompletionItems();
      QList<CompletionTreeItemPointer> memberAccessCompletionItems(const bool& shouldAbort);
      QList<CompletionTreeItemPointer> returnAccessCompletionItems();
      QList<CompletionTreeItemPointer> caseAccessCompletionItems();
      QList<CompletionTreeItemPointer> templateAccessCompletionItems();
      QList<CompletionTreeItemPointer> functionAccessCompletionItems(bool fullCompletion);
      QList<CompletionTreeItemPointer> binaryFunctionAccessCompletionItems(bool fullCompletion);
      ///*DUChain must be locked*
      QList<CompletionTreeItemPointer> commonFunctionAccessCompletionItems(bool fullCompletion);
      QList<CompletionTreeItemPointer> includeListAccessCompletionItems(const bool& shouldAbort);
      QList<CompletionTreeItemPointer> signalSlotAccessCompletionItems();
      ///Computes the completion-items for the case that no special kind of access is used(just a list of all suitable items is needed)
      QList<CompletionTreeItemPointer> standardAccessCompletionItems();
      QList<CompletionTreeItemPointer> getImplementationHelpers();
      QList<CompletionTreeItemPointer> getImplementationHelpersInternal(const QualifiedIdentifier& minimumScope, DUContext* context);

      ///If @param forDecl is an instance of a class, find declarations in that class which match @param matchTypes
      ///@returns the list of matching declarations and whether or not you need the arrow operator (->) to access them
      QList<DeclAccessPair> getLookaheadMatches(Declaration* forDecl, const QList<IndexedType>& matchTypes) const;
      void addLookaheadMatches(const QList<CompletionTreeItemPointer> items);
      ///For a given @param container, find members which may potentially be used for lookahead matching
      ///@param isPointer specifies whether the container should be accessed with operator->
      ///@returns a list of declarations paired with whether or not they use "operator->"
      ///Note that a non-pointer container may declare an operator-> (ie, smart pointer)
      QList<DeclAccessPair> containedDeclarationsForLookahead(Declaration* decl, TopDUContext* top, bool isPointer) const;

      ///*DUChain must be locked*
      bool  filterDeclaration(Declaration* decl, DUContext* declarationContext = 0, bool dynamic = true) const;
      ///*DUChain must be locked*
      bool  filterDeclaration(ClassMemberDeclaration* decl, DUContext* declarationContext = 0) const;
      ///Replaces the member-access type at the current cursor position from "from" to "new", for example from "->" to "."
      ///*DUChain must be locked*
      void replaceCurrentAccess(QString old, QString _new);

      ///Creates the group and adds it to m_storedUngroupedItems if items is not empty
      void eventuallyAddGroup(QString name, int priority, QList< KSharedPtr< KDevelop::CompletionTreeItem > > items);
      
      ///@param type The type of the argument the items are matched to.
      ///*DUChain must be locked*
      void addSpecialItemsForArgumentType(AbstractType::Ptr type);
      
      ///Returns whether the declaration is directly visible from within the current context
      bool visibleFromWithin(Declaration* decl, DUContext* currentContext) const;

      ///Returns whether the declaration can be considered an integral constant
      ///@param acceptHelperItems whether we check for filtering (true) or for marking match quality (false)
      bool isIntegralConstant(Declaration* decl, bool acceptHelperItems) const;
      
      ///Returns whether the end of m_text is a valid completion-position
      bool isValidPosition();
      ///Returns whether this is a valid context for implementation helpers
      bool isImplementationHelperValid() const;

      /**
       * Group adding functions
       * *DUChain must be locked for these functions*
      **/
      void addOverridableItems();
      void addImplementationHelpers();
      void addCPPBuiltin();

      /**
       * Specialized completion functions, if these completion types are
       * valid, no need to continue searching for information about this context
       * *DUChain must be locked for these functions*
       **/
      ///Handle SIGNAL/SLOT in connect/disconnect, \returns true if valid
      bool doSignalSlotCompletion();
      ///Handle include path completion, \returns true if valid
      bool doIncludeCompletion();
      ///Handle code-completion for constructor-initializers, \returns true if valid
      bool doConstructorCompletion();

      AccessType m_accessType;
      QString m_expression;
      QString m_followingText;
      QString m_operator; //If this completion-context ends with a binary operator, this is the operator
      ExpressionEvaluationResult m_expressionResult;

      //Here known argument-expressions and their types, that may have come from sub-contexts, are stored
      QStringList m_knownArgumentExpressions;
      QList<ExpressionEvaluationResult> m_knownArgumentTypes;
      QString m_functionName;
      QList<Function> m_matchingFunctionOverloads;

      //If a signal/slot access is performed, and a slot is being connected to a signal, this contains the identifier and the signature
      Identifier m_connectedSignalIdentifier;
      QByteArray m_connectedSignalNormalizedSignature;
      IndexedDeclaration m_connectedSignal;

      //true if constructor completion is performed
      bool m_isConstructorCompletion;

      //include completion items for include completion
      QList<KDevelop::IncludeItem> m_includeItems;

      //0 = No conversion, +1, +2, .. = increase pointer level = &, -1, -2, .. = decrease pointer level = *
      int m_pointerConversionsBeforeMatching; 

      QList<KDevelop::CompletionTreeElementPointer> m_storedUngroupedItems;

      //A specific completion item type to show, or ShowAll, see enum OnlyShow
      OnlyShow m_onlyShow;
      //Expression is set to the type part in something like: {type}{varname}{initialization}
      bool m_expressionIsTypePrefix;
      bool m_doAccessFiltering;
      DUContextPointer m_localClass;

      QList<IndexedType> m_cachedMatchTypes;

      friend class ImplementationHelperItem;
  };
}

#endif
