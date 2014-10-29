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

#ifndef KDEVPLATFORM_CODECOMPLETIONCONTEXT_H
#define KDEVPLATFORM_CODECOMPLETIONCONTEXT_H

#include "../duchain/duchainpointer.h"
#include <language/languageexport.h>
#include "../editor/cursorinrevision.h"
#include "codecompletionitem.h"

namespace KTextEditor {
  class View;
  class Cursor;
}

namespace KDevelop {
  class CursorInRevision;

  class CompletionTreeItem;
  class CompletionTreeElement;
  typedef QExplicitlySharedDataPointer<CompletionTreeItem> CompletionTreeItemPointer;
  typedef QExplicitlySharedDataPointer<CompletionTreeElement> CompletionTreeElementPointer;

  /**
   * This class is responsible for finding out what kind of completion is needed, what expression should be evaluated for the container-class of the completion, what conversion will be applied to the result of the completion, etc.
   * */
  class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionContext : public QSharedData {
    public:
      typedef QExplicitlySharedDataPointer<CodeCompletionContext> Ptr;

      /**
       * @param text the text to analyze. It usually is the text in the range starting at the beginning of the context,
       *    and ending at the position where completion should start
       *
       * @warning The du-chain must be unlocked when this is called
       * */
      CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, const KDevelop::CursorInRevision& position, int depth = 0);
      virtual ~CodeCompletionContext();

      /**
       * @return Whether this context is valid for code-completion
       */
      bool isValid() const;

      /**
       * @return Depth of the context. The basic completion-context has depth 0, its parent 1, and so on..
       */
      int depth() const;

      /**
       * Computes the full set of completion items, using the information retrieved earlier.
       * Should only be called on the first context, parent contexts are included in the computations.
       *
       * @param abort Checked regularly, and if false, the computation is aborted.
       *
       * @warning Please check @p abort and @p isValid when reimplementing this method
       */
      virtual QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true) = 0;

      /**
       * After completionItems(..) has been called, this may return completion-elements that are already grouped,
       * for example using custom grouping(@see CompletionCustomGroupNode
       */
      virtual QList<KDevelop::CompletionTreeElementPointer> ungroupedElements();
      
      /**
       * In the case of recursive argument-hints, there may be a chain of parent-contexts, each for the higher argument-matching
       * The parentContext() should always have the access-operation FunctionCallAccess.
       * When a completion-list is computed, the members of the list can be highlighted that match
       * the corresponding parentContext()->functions() function-argument, or parentContext()->additionalMatchTypes()
       */
      CodeCompletionContext* parentContext();

      ///Sets the new parent context, and also updates the depth
      void setParentContext(QExplicitlySharedDataPointer<CodeCompletionContext> newParent);
      
      DUContext* duContext() const;
      
    protected:
      static QString extractLastLine(const QString& str);
      
      QString m_text;
      int m_depth;
      bool m_valid;
      KDevelop::CursorInRevision m_position;

      KDevelop::DUContextPointer m_duContext;

      QExplicitlySharedDataPointer<CodeCompletionContext> m_parentContext;
  };
}

#endif
