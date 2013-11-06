/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "worker.h"

#include "context.h"

#include <kdebug.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/parsingenvironment.h>

using namespace KDevelop;

namespace Cpp {

CodeCompletionWorker::CodeCompletionWorker(CodeCompletionModel* model)
  : KDevelop::CodeCompletionWorker(model)
{
}
KDevelop::CodeCompletionContext* CodeCompletionWorker::createCompletionContext(KDevelop::DUContextPointer context, const QString &contextText, const QString &followingText, const KDevelop::CursorInRevision& position) const
{
  return new Cpp::CodeCompletionContext( context, contextText, followingText, position );
}

void CodeCompletionWorker::updateContextRange(KTextEditor::Range& contextRange, KTextEditor::View* view, DUContextPointer context) const
{
  if(context && context->owner() && context->owner()->type<FunctionType>()) {
    if(!context->owner()->type<FunctionType>()->returnType()) {
      //For constructor completion, we need some more context
      contextRange.start().setLine(contextRange.start().line() > 30 ? contextRange.start().line()-30 : 0);
      contextRange.start().setColumn(0);
    }
  }
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, QString followingText, const KTextEditor::Range& _contextRange, const QString& _contextText)
{
  KTextEditor::Range contextRange(_contextRange);
  QString contextText(_contextText);
  
  TopDUContextPointer topContext;
  {
    DUChainReadLocker lock(DUChain::lock());
    if(context)
      topContext = TopDUContextPointer(context->topContext());
    if(!topContext)
      return;
    if(!topContext->parsingEnvironmentFile() || topContext->parsingEnvironmentFile()->language() != IndexedString("C++")) {
      kDebug() << "top-context has wrong language:";
      return;
    }
  }
  
  Cpp::TypeConversionCacheEnabler enableConversionCache;

  KDevelop::CodeCompletionWorker::computeCompletions(context, position, followingText, contextRange, contextText);
}

}

#include "worker.moc"
