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

using namespace KTextEditor;
using namespace KDevelop;
using namespace TypeUtils;

CppCodeCompletionModel::CppCodeCompletionModel( QObject * parent )
  : CodeCompletionModel2(parent)
  , m_mutex(new QMutex)
  , m_worker(new CodeCompletionWorker(this))
{
  qRegisterMetaType<QList<CompletionTreeElement> >("QList<KSharedPtr<CompletionTreeElement> >");
  qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");

  connect(this, SIGNAL(completionsNeeded(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), m_worker, SLOT(computeCompletions(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), Qt::QueuedConnection);

  //We connect directly, so we can do the pre-grouping within the background thread
  connect(m_worker, SIGNAL(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void*)), this, SLOT(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void*)), Qt::QueuedConnection);

  m_worker->start();
}

CppCodeCompletionModel::~CppCodeCompletionModel()
{
  // Let it leak...??
  m_worker->setParent(0L);
  m_worker->quit();

  delete m_mutex;
}

void CppCodeCompletionModel::addNavigationWidget(const CompletionTreeElement* element, Cpp::NavigationWidget* widget) const
{
  m_navigationWidgets[element] = widget;
}

void CppCodeCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType)
{
  Q_UNUSED(invocationType)

  m_navigationWidgets.clear();
  m_completionItems.clear();

  reset();

  m_worker->abortCurrentCompletion();

  KUrl url = view->document()->url();

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
  m_currentTopContext = TopDUContextPointer(top);

  if (top) {
    kDebug(9007) << "completion invoked for context" << (DUContext*)top;

    if( top->parsingEnvironmentFile()->modificationRevision() != EditorIntegrator::modificationRevision(url.prettyUrl()) ) {
      kDebug(9007) << "Found context is not current. Its revision is " << top->parsingEnvironmentFile()->modificationRevision() << " while the document-revision is " << EditorIntegrator::modificationRevision(url.prettyUrl());
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

void CppCodeCompletionModel::foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > items, void* completionContext)
{
  m_completionItems = items;
  m_completionContext = KSharedPtr<Cpp::CodeCompletionContext>((Cpp::CodeCompletionContext*)completionContext);
  reset();
  
/*  if (completionContext == m_completionContext.data()) {
    if( !m_completionItems.isEmpty() ) {
      beginInsertRows(QModelIndex(), m_completionItems.count(), m_completionItems.count() + items.count() - 1);
      m_completionItems += items;
      endInsertRows();
    } else {*/
/*    }
  }*/
  
}

void CppCodeCompletionModel::setCompletionContext(KSharedPtr<Cpp::CodeCompletionContext> completionContext)
{
  QMutexLocker lock(m_mutex);
  m_completionContext = completionContext;
}

KSharedPtr<Cpp::CodeCompletionContext> CppCodeCompletionModel::completionContext() const
{
  QMutexLocker lock(m_mutex);
  return m_completionContext;
}

///@todo move into subclass
void CppCodeCompletionModel::executeCompletionItem2(Document* document, const Range& word, const QModelIndex& index) const
{
  DUChainReadLocker lock(DUChain::lock(), 3000);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain for completion-item execution"; //Probably we prevented a deadlock
    return;
  }
  
  CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();
  if( !element || !element->asItem() )
    return;

  element->asItem()->execute(document, word);
}

QVariant CppCodeCompletionModel::data(const QModelIndex& index, int role) const
{
  CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();
  if( !element )
    return QVariant();

  CompletionTreeElement& treeElement(*element);

  if( role == CodeCompletionModel::GroupRole ) {
    if( treeElement.asNode() ) {
      return QVariant(treeElement.asNode()->role);
    }else {
      kDebug(9007) << "Requested group-role from leaf tree element";
      return QVariant();
    }
  }else{
    if( treeElement.asNode() ) {
      if( role == treeElement.asNode()->role ) {
        return treeElement.asNode()->roleValue;
      } else {
        kDebug(9007) << "Requested wrong role from non-leaf tree element";
        return QVariant();
      }
    }
  }

  if(!treeElement.asItem()) {
    kWarning(9007) << "Error in completion model";
    return QVariant();
  }

  //Navigation widget interaction is done here, the other stuff is done within the tree-elements
  switch (role) {
    case CodeCompletionModel::InheritanceDepth:
      return treeElement.asItem()->inheritanceDepth();
    case CodeCompletionModel::ArgumentHintDepth:
      return treeElement.asItem()->argumentHintDepth();
  
    case AccessibilityNext:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&treeElement];
      if( w )
        w->next();
    }
    break;
    case AccessibilityPrevious:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&treeElement];
      if( w )
        w->previous();
    }
    break;
    case AccessibilityAccept:
    {
      Cpp::NavigationWidget* w = m_navigationWidgets[&treeElement];
      if( w )
        w->accept();
    }
    break;
  }

  return treeElement.asItem()->data(index, role, this);
}

KDevelop::TopDUContextPointer CppCodeCompletionModel::currentTopContext() const
{
  return m_currentTopContext;
}

QModelIndex CppCodeCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
  if( parent.isValid() ) {
    CompletionTreeElement* element = (CompletionTreeElement*)parent.internalPointer();

    CompletionTreeNode* node = element->asNode();

    if( !node ) {
      kDebug(9007) << "Requested sub-index of leaf node";
      return QModelIndex();
    }

    if (row < 0 || row >= node->children.count() || column < 0 || column >= ColumnCount)
      return QModelIndex();

    return createIndex(row, column, node->children[row].data());
  } else {
    if (row < 0 || row >= m_completionItems.count() || column < 0 || column >= ColumnCount)
      return QModelIndex();

    return createIndex(row, column, const_cast<CompletionTreeElement*>(m_completionItems[row].data()));
  }
}

QModelIndex CppCodeCompletionModel::parent ( const QModelIndex & index ) const
{
  if(rowCount() == 0)
    return QModelIndex();

  if( index.isValid() ) {
    CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();

    if( element->parent() )
      return createIndex( element->rowInParent(), element->columnInParent(), element->parent() );
  }

  return QModelIndex();
}

int CppCodeCompletionModel::rowCount ( const QModelIndex & parent ) const
{
  if( parent.isValid() ) {
    CompletionTreeElement* element = (CompletionTreeElement*)parent.internalPointer();

    CompletionTreeNode* node = element->asNode();

    if( !node )
      return 0;

    return node->children.count();
  }else{
    return m_completionItems.count();
  }
}

#include "cppcodecompletionmodel.moc"
