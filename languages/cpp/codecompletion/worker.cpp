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

#include <kdebug.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <klocale.h>

#include "../cppduchain/cppduchain.h"
#include "../cppduchain/typeutils.h"

#include "../cppduchain/overloadresolutionhelper.h"

#include <language/duchain/declaration.h>
#include <language/duchain/ducontext.h>
#include "../cppduchain/cpptypes.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/dumpchain.h>
#include <language/codecompletion/codecompletioncontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/parsingenvironment.h>

using namespace KDevelop;
using namespace TypeUtils;

namespace Cpp {

CodeCompletionWorker::CodeCompletionWorker(CodeCompletionModel* parent)
  : KDevelop::CodeCompletionWorker(parent)
{
}
KDevelop::CodeCompletionContext* CodeCompletionWorker::createCompletionContext(KDevelop::DUContextPointer context, const QString &contextText, const QString &followingText) const
{
  return new Cpp::CodeCompletionContext( context, contextText, followingText );
}

void CodeCompletionWorker::computeCompletions(KDevelop::DUContextPointer context, const KTextEditor::Cursor& position, KTextEditor::View* view, const KTextEditor::Range& contextRange, const QString& contextText)
{
  TopDUContextPointer topContext;
  {
    DUChainReadLocker lock(DUChain::lock());
    if(context)
      topContext = TopDUContextPointer(context->topContext());
    if(!topContext)
      return;
    if(!topContext->parsingEnvironmentFile() || topContext->language() != IndexedString("C++")) {
      kDebug() << "top-context has wrong language:" << topContext->language().str();
      return;
    }
  }
  
  //We will have some caching in TopDUContext until this objects lifetime is over
  TopDUContext::Cache cache(topContext);
  Cpp::TypeConversionCacheEnabler enableConversionCache;

  KDevelop::CodeCompletionWorker::computeCompletions(context, position, view, contextRange, contextText);
}

CodeCompletionModel* CodeCompletionWorker::model() const
{
  return const_cast<CodeCompletionModel*>(static_cast<const CodeCompletionModel*>(parent()));
}

}

#include "worker.moc"
