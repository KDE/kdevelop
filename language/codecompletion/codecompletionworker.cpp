/*
 * KDevelop Generic Code Completion Support
 *
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "codecompletionworker.h"

#include <kdebug.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <klocale.h>

#include "../duchain/ducontext.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchain.h"
#include "codecompletion.h"
#include "codecompletionitem.h"
#include "codecompletionmodel.h"
#include "codecompletionitemgrouper.h"
#include <interfaces/foregroundlock.h>

using namespace KTextEditor;
using namespace KDevelop;

CodeCompletionWorker::CodeCompletionWorker(KDevelop::CodeCompletionModel* model) :
  m_hasFoundDeclarations(false)
  , m_mutex(new QMutex())
  , m_abort(false)
  , m_fullCompletion(true)
  , m_model(model)
{
}

CodeCompletionWorker::~CodeCompletionWorker()
{
  delete m_mutex;
}

void CodeCompletionWorker::setFullCompletion(bool full) {
  m_fullCompletion = full;
}

bool CodeCompletionWorker::fullCompletion() const {
  return m_fullCompletion;
}

void CodeCompletionWorker::failed() {
    foundDeclarations( QList<KSharedPtr<CompletionTreeElement> >(), KSharedPtr<KDevelop::CodeCompletionContext>() );
}

void KDevelop::CodeCompletionWorker::foundDeclarations(QList< KSharedPtr< KDevelop::CompletionTreeElement > >  items, KSharedPtr< KDevelop::CodeCompletionContext > completionContext)
{
    m_hasFoundDeclarations = true;
    emit foundDeclarationsReal(items, completionContext);
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view)
{
  {
    QMutexLocker lock(m_mutex);
    m_abort = false;
  }

  ///@todo It's not entirely safe to pass KTextEditor::View* through a queued connection
  // We access the view/document which is not thread-safe, so we need the foreground lock
  ForegroundLock foreground;
  
  //Compute the text we should complete on
  KTextEditor::Document* doc = view->document();
  if( !doc ) {
    kDebug() << "No document for completion";
    failed();
    return;
  }

  KTextEditor::Range range;
  QString text;
  {
    QMutexLocker lock(m_mutex);
    DUChainReadLocker lockDUChain;
    
    if(context) {
      kDebug() << context->localScopeIdentifier().toString();
      range = KTextEditor::Range(context->rangeInCurrentRevision().start.textCursor(), position);
    }
    
    else
      range = KTextEditor::Range(KTextEditor::Cursor(position.line(), 0), position);

    updateContextRange(range, view, context);
    
    text = doc->text(range);
  }

  if( position.column() == 0 ) //Seems like when the cursor is a the beginning of a line, kate does not give the \n
    text += '\n';

  if (aborting()) {
    failed();
    return;
  }
  m_hasFoundDeclarations = false;

  KTextEditor::Cursor cursorPosition = view->cursorPosition();
  QString followingText; //followingText may contain additional text that stands for the current item. For example in the case "QString|", QString is in addedText.
  if(position < cursorPosition)
    followingText = view->document()->text( KTextEditor::Range( position, cursorPosition ) );
  
  foreground.unlock();
  
  computeCompletions(context, position, followingText, range, text);
  
  if(!m_hasFoundDeclarations)
    failed();
}

void KDevelop::CodeCompletionWorker::doSpecialProcessing(uint) {

}

CodeCompletionContext* CodeCompletionWorker::createCompletionContext(KDevelop::DUContextPointer context, const QString& contextText, const QString& followingText, const KDevelop::CursorInRevision& position) const {
  Q_UNUSED(context);
  Q_UNUSED(contextText);
  Q_UNUSED(followingText);
  Q_UNUSED(position);
  return 0;
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, QString followingText, const KTextEditor::Range& contextRange, const QString& contextText)
{
  Q_UNUSED(contextRange);
  
  kDebug() << "added text:" << followingText;
  
  CodeCompletionContext::Ptr completionContext( createCompletionContext( context, contextText, followingText, CursorInRevision::castFromSimpleCursor(SimpleCursor(position)) ) );
  if (KDevelop::CodeCompletionModel* m = model())
    m->setCompletionContext(KDevelop::CodeCompletionContext::Ptr::staticCast(completionContext));

  if( completionContext && completionContext->isValid() ) {
    {
      DUChainReadLocker lock(DUChain::lock());

      if (!context) {
        failed();
        kDebug() << "Completion context disappeared before completions could be calculated";
        return;
      }
    }
    QList<CompletionTreeItemPointer> items = completionContext->completionItems(aborting(), fullCompletion());

    if (aborting()) {
      failed();
      return;
    }
    
    QList<KSharedPtr<CompletionTreeElement> > tree = computeGroups( items, completionContext );

    if(aborting()) {
      failed();
      return;
    }
    
    tree += completionContext->ungroupedElements();

    foundDeclarations( tree, KSharedPtr<KDevelop::CodeCompletionContext>::staticCast(completionContext) );

  } else {
    kDebug() << "setContext: Invalid code-completion context";
  }
}

QList<KSharedPtr<CompletionTreeElement> > CodeCompletionWorker::computeGroups(QList<CompletionTreeItemPointer> items, KSharedPtr<CodeCompletionContext> completionContext)
{
  Q_UNUSED(completionContext);
  QList<KSharedPtr<CompletionTreeElement> > tree;
  /**
   * 1. Group by argument-hint depth
   * 2. Group by inheritance depth
   * 3. Group by simplified attributes
   * */
  CodeCompletionItemGrouper<ArgumentHintDepthExtractor, CodeCompletionItemGrouper<InheritanceDepthExtractor, CodeCompletionItemGrouper<SimplifiedAttributesExtractor> > > argumentHintDepthGrouper(tree, 0, items);
  return tree;
}

void CodeCompletionWorker::abortCurrentCompletion()
{
  QMutexLocker lock(m_mutex);
  m_abort = true;
}

bool& CodeCompletionWorker::aborting()
{
  return m_abort;
}

KDevelop::CodeCompletionModel* CodeCompletionWorker::model() const
{
  return m_model;
}

void CodeCompletionWorker::updateContextRange(Range& contextRange, View* view, DUContextPointer context) const
{
  Q_UNUSED(contextRange);
  Q_UNUSED(view);
  Q_UNUSED(context);
}
