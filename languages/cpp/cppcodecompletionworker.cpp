/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "cppcodecompletionworker.h"

#include <kdebug.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include "cppduchain/cppduchain.h"
#include "cppduchain/typeutils.h"

#include "cppduchain/overloadresolutionhelper.h"

#include <declaration.h>
#include <ducontext.h>
#include "cpptypes.h"
#include <duchainlock.h>
#include <duchainbase.h>
#include "dumpchain.h"
#include "codecompletioncontext.h"
#include <duchainutils.h>

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

CodeCompletionWorker::CodeCompletionWorker(CppCodeCompletionModel* parent)
  : QThread(parent)
  , m_mutex(new QMutex)
{
}

CodeCompletionWorker::~CodeCompletionWorker()
{
  delete m_mutex;
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view)
{
  {
    QMutexLocker lock(m_mutex);
    m_abort = false;
  }

  //Compute the text we should complete on
  KTextEditor::Document* doc = view->document();
  if( !doc ) {
    kDebug(9007) << "No document for completion";
    return;
  }

  KTextEditor::Range range;
  QString text;
  {
    range = KTextEditor::Range(context->textRange().start(), position);
    text = doc->text(range);
  }

  if( text.isEmpty() ) {
    kDebug(9007) << "no text for context";
    return;
  }

  if( position.column() == 0 ) //Seems like when the cursor is a the beginning of a line, kate does not give the \n
    text += '\n';

  Cpp::CodeCompletionContext::Ptr completionContext( new Cpp::CodeCompletionContext( context, text ) );
  if (CppCodeCompletionModel* m = model())
    m->setCompletionContext(completionContext);

  typedef QPair<Declaration*, int> DeclarationDepthPair;

  if( completionContext->isValid() ) {
    DUChainReadLocker lock(DUChain::lock());

    if (!context) {
      kDebug(9007) << "Completion context disappeared before completions could be calculated";
      return;
    }

    if (m_abort)
      return;

    if( completionContext->memberAccessContainer().isValid() ||completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::StaticMemberChoose )
    {
      QList<DUContext*> containers = completionContext->memberAccessContainers();
      if( !containers.isEmpty() ) {
        foreach(DUContext* ctx, containers) {
          if (m_abort)
            return;

          foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations( ctx->allDeclarations(ctx->textRange().end(), context->topContext(), false ) ) )
            emit foundDeclaration(CppCodeCompletionModel::CompletionItem( DeclarationPointer(decl.first), completionContext, decl.second ), completionContext.data());
        }
      } else {
        kDebug(9007) << "CppCodeCompletionModel::setContext: no container-type";
      }
    } else if( completionContext->memberAccessOperation() == Cpp::CodeCompletionContext::IncludeListAccess ) {
      //Include-file completion
      int cnt = 0;
      QList<Cpp::IncludeItem> allIncludeItems = completionContext->includeItems();
      foreach(const Cpp::IncludeItem& includeItem, allIncludeItems) {
        if (m_abort)
          return;

        CppCodeCompletionModel::CompletionItem completionItem;
        completionItem.includeItem = includeItem;
        emit foundDeclaration(completionItem, completionContext.data());
        ++cnt;
      }
      kDebug(9007) << "Added " << cnt << " include-files to completion-list";
    } else {
      //Show all visible declarations

      foreach( const DeclarationDepthPair& decl, Cpp::hideOverloadedDeclarations( context->allDeclarations(context->type() == DUContext::Class ? context->textRange().end() : position, context->topContext()) ) ) {
        if (m_abort)
          return;
        emit foundDeclaration(CppCodeCompletionModel::CompletionItem( DeclarationPointer(decl.first), completionContext, decl.second ), completionContext.data());
      }

      kDebug(9007) << "CppCodeCompletionModel::setContext: using all declarations visible";
    }

    ///Find all recursive function-calls that should be shown as call-tips
    Cpp::CodeCompletionContext::Ptr parentContext = completionContext;
    do {
      if (m_abort)
        return;

      parentContext = parentContext->parentContext();
      if( parentContext ) {
        if( parentContext->memberAccessOperation() == Cpp::CodeCompletionContext::FunctionCallAccess ) {
          int num = 0;
          foreach( Cpp::CodeCompletionContext::Function function, parentContext->functions() ) {
            emit foundDeclaration(CppCodeCompletionModel::CompletionItem( function.function.declaration(), parentContext, 0, num ), completionContext.data());
            ++num;
          }
        } else {
          kDebug(9007) << "parent-context has non function-call access type";
        }
      }
    } while( parentContext );
  } else {
    kDebug(9007) << "CppCodeCompletionModel::setContext: Invalid code-completion context";
  }
}

CppCodeCompletionModel* CodeCompletionWorker::model() const
{
  return const_cast<CppCodeCompletionModel*>(static_cast<const CppCodeCompletionModel*>(parent()));
}

void CodeCompletionWorker::run()
{
  exec();
}

void CodeCompletionWorker::abortCurrentCompletion()
{
  QMutexLocker lock(m_mutex);
  m_abort = true;
}

#include "cppcodecompletionworker.moc"
