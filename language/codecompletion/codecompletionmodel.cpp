/*
 * KDevelop Code Completion Support
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
#include <khtmlview.h>


#include <declaration.h>
#include "typesystem.h"
#include <classfunctiondeclaration.h>
#include <ducontext.h>
#include <duchain.h>
#include <namespacealiasdeclaration.h>
#include <parsingenvironment.h>
#include <editorintegrator.h>
#include <duchainlock.h>
#include <duchainbase.h>
#include <topducontext.h>
#include "codecompletioncontext.h"
#include <duchainutils.h>
#include "codecompletionworker.h"
#include "inavigationwidget.h"

using namespace KTextEditor;

namespace KDevelop {

CodeCompletionModel::CodeCompletionModel( QObject * parent )
  : CodeCompletionModel2(parent)
  , m_mutex(new QMutex)
  , m_worker(0)
{
  qRegisterMetaType<QList<CompletionTreeElement> >("QList<KSharedPtr<CompletionTreeElement> >");
  qRegisterMetaType<KTextEditor::Cursor>("KTextEditor::Cursor");
}

CodeCompletionModel::~CodeCompletionModel()
{
  // Let it leak...??
  m_worker->setParent(0L);
  m_worker->quit();

  delete m_mutex;
}

void CodeCompletionModel::setCompletionWorker(CodeCompletionWorker* worker)
{
  if (m_worker) {
    kWarning() << "Already have a current code completion worker!";
    return;
  }
  
  m_worker = worker;

  //We connect directly, so we can do the pre-grouping within the background thread
  connect(m_worker, SIGNAL(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void*)), this, SLOT(foundDeclarations(QList<KSharedPtr<CompletionTreeElement> >, void*)), Qt::QueuedConnection);

  connect(this, SIGNAL(completionsNeeded(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), m_worker, SLOT(computeCompletions(KDevelop::DUContextPointer, const KTextEditor::Cursor&, KTextEditor::View*)), Qt::QueuedConnection);

  m_worker->start();
}

void CodeCompletionModel::addNavigationWidget(const CompletionTreeElement* element, QWidget* widget) const
{
  Q_ASSERT(dynamic_cast<INavigationWidget*>(widget));
  m_navigationWidgets[element] = widget;
}

void CodeCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType)
{
  Q_UNUSED(invocationType)

  m_navigationWidgets.clear();
  m_completionItems.clear();

  reset();

  m_worker->abortCurrentCompletion();

  KUrl url = view->document()->url();

  completionInvokedInternal(view, range, invocationType, url);
}

void CodeCompletionModel::foundDeclarations(QList<KSharedPtr<CompletionTreeElement> > items, void* completionContext)
{
  m_completionItems = items;
  m_completionContext = KSharedPtr<CodeCompletionContext>((CodeCompletionContext*)completionContext);
  reset();
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
      INavigationWidget* w = dynamic_cast<INavigationWidget*>(m_navigationWidgets[&treeElement].data());
      if( w )
        w->next();
    }
    break;
    case AccessibilityPrevious:
    {
      INavigationWidget* w = dynamic_cast<INavigationWidget*>(m_navigationWidgets[&treeElement].data());
      if( w )
        w->previous();
    }
    break;
    case AccessibilityAccept:
    {
      INavigationWidget* w = dynamic_cast<INavigationWidget*>(m_navigationWidgets[&treeElement].data());
      if( w )
        w->accept();
    }
    break;
  }

  return treeElement.asItem()->data(index, role, this);
}

KDevelop::TopDUContextPointer CodeCompletionModel::currentTopContext() const
{
  return m_currentTopContext;
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
