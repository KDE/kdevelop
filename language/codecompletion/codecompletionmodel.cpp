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

#include "codecompletionmodel.h"

#include <QIcon>
#include <QMetaType>
#include <QTextFormat>
#include <QBrush>
#include <QDir>
#include <kdebug.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kiconloader.h>


#include "../duchain/declaration.h"
#include "../duchain/classfunctiondeclaration.h"
#include "../duchain/ducontext.h"
#include "../duchain/duchain.h"
#include "../duchain/namespacealiasdeclaration.h"
#include "../duchain/parsingenvironment.h"
#include "../editor/editorintegrator.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchainbase.h"
#include "../duchain/topducontext.h"
#include "../duchain/duchainutils.h"
#include "../interfaces/quickopendataprovider.h"
#include "../interfaces/icore.h"
#include "../interfaces/ilanguagecontroller.h"
#include "../interfaces/icompletionsettings.h"

#include "codecompletionworker.h"
#include "codecompletioncontext.h"
#include <duchain/specializationstore.h>

using namespace KTextEditor;

//Multi-threaded completion creates some multi-threading related crashes, and sometimes shows the completions in the wrong position if the cursor was moved
// #define SINGLE_THREADED_COMPLETION

namespace KDevelop {

struct CompletionWorkerThread : public QThread {
   CompletionWorkerThread(CodeCompletionModel* model) : QThread(model), m_model(model), m_worker(0) {
   }
   ~CompletionWorkerThread() {
     delete m_worker;
   }
   
   virtual void run () {
     if(!m_worker)
      m_worker = m_model->createCompletionWorker();
     
     //We connect directly, so we can do the pre-grouping within the background thread
     connect(m_worker, SIGNAL(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, KSharedPtr<CodeCompletionContext>)), m_model, SLOT(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, KSharedPtr<CodeCompletionContext>)), Qt::QueuedConnection);

     connect(m_model, SIGNAL(completionsNeeded(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), m_worker, SLOT(computeCompletions(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), Qt::QueuedConnection);
     connect(m_model, SIGNAL(doSpecialProcessingInBackground(uint)), m_worker, SLOT(doSpecialProcessing(uint)));
     exec();
   }
   
   CodeCompletionModel* m_model;
   CodeCompletionWorker* m_worker;
};

CodeCompletionModel::CodeCompletionModel( QObject * parent )
  : CodeCompletionModel2(parent)
  , m_fullCompletion(true)
  , m_mutex(new QMutex)
  , m_thread(0)
{
  qRegisterMetaType<QList<CompletionTreeElement> >("QList<KSharedPtr<CompletionTreeElement> >");
  qRegisterMetaType<KSharedPtr<CodeCompletionContext> >("KSharedPtr<CodeCompletionContext>");
  qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");
}

void CodeCompletionModel::initialize() {
  if(!m_thread) {
    m_thread = new CompletionWorkerThread(this);
#ifdef SINGLE_THREADED_COMPLETION
    m_thread->m_worker = createCompletionWorker();
#endif
    m_thread->start();
  }
}

CodeCompletionModel::~CodeCompletionModel()
{
  if(m_thread->m_worker)
    m_thread->m_worker->abortCurrentCompletion();
  m_thread->quit();
  m_thread->wait();
  
  delete m_thread;
  delete m_mutex;
}

void CodeCompletionModel::addNavigationWidget(const CompletionTreeElement* element, QWidget* widget) const
{
  Q_ASSERT(dynamic_cast<QuickOpenEmbeddedWidgetInterface*>(widget));
  m_navigationWidgets[element] = widget;
}


bool CodeCompletionModel::fullCompletion() const
{
  return m_fullCompletion;
}

KDevelop::CodeCompletionWorker* CodeCompletionModel::worker() const {
  return m_thread->m_worker;
}

void CodeCompletionModel::clear() {
  m_completionItems.clear();
  m_navigationWidgets.clear();
  m_completionContext.clear();
  reset();
}

void CodeCompletionModel::completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType, const KUrl& url)
{
  Q_UNUSED(invocationType)

  DUChainReadLocker lock(DUChain::lock(), 400);
  if( !lock.locked() ) {
    kDebug(9007) << "could not lock du-chain in time";
    return;
  }

  TopDUContext* top = DUChainUtils::standardContextForUrl( url );
  if(!top) {
    return;
  }
  setCurrentTopContext(TopDUContextPointer(top));

  if (top) {
    kDebug(9007) << "completion invoked for context" << (DUContext*)top;

    if( top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->modificationRevision() != ModificationRevision::revisionForFile(IndexedString(url.pathOrUrl())) ) {
      kDebug(9007) << "Found context is not current. Its revision is " /*<< top->parsingEnvironmentFile()->modificationRevision() << " while the document-revision is " << ModificationRevision::revisionForFile(IndexedString(url.pathOrUrl()))*/;
    }

    DUContextPointer thisContext;
    {
      thisContext = SpecializationStore::self().applySpecialization(top->findContextAt(SimpleCursor(range.start())), top);

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
void CodeCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType)
{
  //If this triggers, initialize() has not been called after creation.
  Q_ASSERT(m_thread);
  
  KDevelop::ICompletionSettings::CompletionLevel level = KDevelop::ICore::self()->languageController()->completionSettings()->completionLevel();
  if(level == KDevelop::ICompletionSettings::AlwaysFull || (invocationType != AutomaticInvocation && level == KDevelop::ICompletionSettings::MinimalWhenAutomatic))
    m_fullCompletion = true;
  else
    m_fullCompletion = false;
  
  //Only use grouping in full completion mode
  setHasGroups(m_fullCompletion);
  
  Q_UNUSED(invocationType)

  if (!worker())
    kWarning() << "Completion invoked on a completion model which has no code completion worker assigned!";

  m_navigationWidgets.clear();
  m_completionItems.clear();

  reset();

  worker()->abortCurrentCompletion();
  worker()->setFullCompletion(m_fullCompletion);

  KUrl url = view->document()->url();

  completionInvokedInternal(view, range, invocationType, url);
}

void CodeCompletionModel::foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > items, KSharedPtr<CodeCompletionContext> completionContext)
{
  m_completionItems = items;
  m_completionContext = completionContext;

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

void CodeCompletionModel::setCompletionContext(KSharedPtr<CodeCompletionContext> completionContext)
{
  QMutexLocker lock(m_mutex);
  m_completionContext = completionContext;
}

KSharedPtr<CodeCompletionContext> CodeCompletionModel::completionContext() const
{
  QMutexLocker lock(m_mutex);
  return m_completionContext;
}

///@todo move into subclass
void CodeCompletionModel::executeCompletionItem2(Document* document, const Range& word, const QModelIndex& index) const
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

KSharedPtr< KDevelop::CompletionTreeElement > CodeCompletionModel::itemForIndex(QModelIndex index) const {
  CompletionTreeElement* element = (CompletionTreeElement*)index.internalPointer();
  return KSharedPtr< KDevelop::CompletionTreeElement >(element);
}

QVariant CodeCompletionModel::data(const QModelIndex& index, int role) const
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
      if( role == CodeCompletionModel::InheritanceDepth ) {
        CompletionCustomGroupNode* customGroupNode = dynamic_cast<CompletionCustomGroupNode*>(element);
        if(customGroupNode)
          return QVariant(customGroupNode->inheritanceDepth);
      }
      if( role == treeElement.asNode()->role ) {
        return treeElement.asNode()->roleValue;
      } else {
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

    case CodeCompletionModel::ItemSelected: {
      DeclarationPointer decl = treeElement.asItem()->declaration();
      if(decl) {
        DUChain::self()->emitDeclarationSelected(decl);
      }
      break;
    }
  }
  
  //In minimal completion mode, hide all columns except the "name" one
  if(!m_fullCompletion && role == Qt::DisplayRole && index.column() != Name)
    return QVariant();
  
  QVariant ret = treeElement.asItem()->data(index, role, this);

  //In reduced completion mode, don't show information text with the selected items
  if(!m_fullCompletion && role == ItemSelected)
    return QVariant();
  
  return ret;
}

KDevelop::TopDUContextPointer CodeCompletionModel::currentTopContext() const
{
  return m_currentTopContext;
}

void CodeCompletionModel::setCurrentTopContext(KDevelop::TopDUContextPointer topContext)
{
  m_currentTopContext = topContext;
}

QModelIndex CodeCompletionModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex CodeCompletionModel::parent ( const QModelIndex & index ) const
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

int CodeCompletionModel::rowCount ( const QModelIndex & parent ) const
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

}

#include "codecompletionmodel.moc"
