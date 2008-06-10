/* 
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

#ifndef CODECOMPLETIONCONTEXT_H
#define CODECOMPLETIONCONTEXT_H

#include <ktexteditor/cursor.h>
#include <ksharedptr.h>

#include "duchain/duchainpointer.h"
#include "completionitem.h"
#include "../languageexport.h"

namespace KTextEditor {
  class View;
  class Cursor;
}

namespace KDevelop {
  class DUContext;
  class AbstractType;


  class CompletionTreeItem;
  typedef KSharedPtr<CompletionTreeItem> CompletionTreeItemPointer;

  /**
   * This class is responsible for finding out what kind of completion is needed, what expression should be evaluated for the container-class of the completion, what conversion will be applied to the result of the completion, etc.
   * */
  class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionContext : public KShared {
    public:

      ///Computes the full set of completion items, using the information retrieved earlier.
      ///Should only be called on the first context, parent contexts are included in the computations.
      ///@param Abort is checked regularly, and if it is false, the computation is aborted.
      QList<CompletionTreeItemPointer> completionItems(const KDevelop::SimpleCursor& position, bool& abort);

      typedef KSharedPtr<CodeCompletionContext> Ptr;

      typedef QList<Function> FunctionList;
      
      /**
       * @param firstContext should be true for a context that has no parent. Such a context will never be a function-call context.
       * @param text the text to analyze. It usually is the text in the range starting at the beginning of the context, and ending at the position where completion should start
       * @warning The du-chain must be unlocked when this is called
       * @param knownArgumentExpressions has no effect when firstContext is set
       * @param line Optional line that will be used to filter the macros
       * */
      CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, int depth = 0, const QStringList& knownArgumentExpressions = QStringList(), int line = -1 );
      ~CodeCompletionContext();

      ///@return whether this context is valid for code-completion
      virtual bool isValid() const = 0;

      ///@return depth of the context. The basic completion-context has depth 0, its parent 1, and so on..
      int depth() const;
      
      /**In the case of recursive argument-hints, there may be a chain of parent-contexts, each for the higher argument-matching
       * The parentContext() should always have the access-operation FunctionCallAccess.
       * When a completion-list is computed, the members of the list can be highlighted that match the corresponding parentContext()->functions() function-argument, or parentContext()->additionalMatchTypes()
       * */
      CodeCompletionContext* parentContext();

    protected:
      void log( const QString& str ) const;
      
      static QString extractLastLine(const QString& str);
      
      QString m_text;
      int m_depth;

      KDevelop::DUContextPointer m_duContext;

      KSharedPtr<CodeCompletionContext> m_parentContext;
  };
}

#endif
