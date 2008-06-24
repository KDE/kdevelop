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

#include "cppcodecompletionmodel.h"

#include <QIcon>
#include <QMetaType>
#include <QTextFormat>
#include <QBrush>
#include <QDir>
#include <kdebug.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kiconloader.h>
#include <khtmlview.h>


#include "cppduchain/cppduchain.h"
#include "cppduchain/typeutils.h"

#include "cppduchain/overloadresolutionhelper.h"

#include <declaration.h>
#include "cpptypes.h"
#include "typeutils.h"
#include <classfunctiondeclaration.h>
#include <ducontext.h>
#include <duchain.h>
#include <namespacealiasdeclaration.h>
#include <parsingenvironment.h>
#include <editorintegrator.h>
#include <duchainlock.h>
#include <duchainbase.h>
#include <topducontext.h>
#include "dumpchain.h"
#include "codecompletioncontext.h"
#include "navigationwidget.h"
#include "preprocessjob.h"
#include <duchainutils.h>
#include "cppcodecompletionworker.h"
#include "cpplanguagesupport.h"
#include <modificationrevision.h>

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

CppCodeCompletionModel::CppCodeCompletionModel( QObject * parent )
  : KDevelop::CodeCompletionModel(parent)
{
  setCompletionWorker(new CppCodeCompletionWorker(this));
}

CppCodeCompletionModel::~CppCodeCompletionModel()
{
}

void CppCodeCompletionModel::completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType, const KUrl& url)
{
  Q_UNUSED(invocationType)

  DUChainReadLocker lock(DUChain::lock(), 400);
  if( !lock.locked() ) {
    kDebug(9007) << "could not lock du-chain in time";
    return;
  }

  TopDUContext* top = CppLanguageSupport::self()->standardContext( url );
  if(!top || !top->parsingEnvironmentFile() || top->parsingEnvironmentFile()->type() != CppParsingEnvironment ) {
    kDebug(9007) << "no context or no parsingEnvironmentFile available, or the context is not a C++ context";
    return;
  }
  setCurrentTopContext(TopDUContextPointer(top));

  if (top) {
    kDebug(9007) << "completion invoked for context" << (DUContext*)top;

    if( top->parsingEnvironmentFile()->modificationRevision() != ModificationRevision::revisionForFile(IndexedString(url.pathOrUrl())) ) {
      kDebug(9007) << "Found context is not current. Its revision is " /*<< top->parsingEnvironmentFile()->modificationRevision() << " while the document-revision is " << ModificationRevision::revisionForFile(IndexedString(url.pathOrUrl()))*/;
    }

    DUContextPointer thisContext;
    {
      thisContext = top->findContextAt(SimpleCursor(range.start()));

       kDebug(9007) << "context is set to" << thisContext.data();
        if( thisContext ) {
/*          kDebug( 9007 ) << "================== duchain for the context =======================";
          DumpChain dump;
          dump.dump(thisContext.data());*/
        } else {
          kDebug( 9007 ) << "================== NO CONTEXT FOUND =======================";
          m_completionItems.clear();
          m_navigationWidgets.clear();
          reset();
          return;
        }
    }

    lock.unlock();

    emit completionsNeeded(thisContext, range.start(), view);
  } else {
    kDebug(9007) << "Completion invoked for unknown context. Document:" << url << ", Known documents:" << DUChain::self()->documents();
  }
}

#include "cppcodecompletionmodel.moc"
