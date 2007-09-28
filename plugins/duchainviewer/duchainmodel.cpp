/*
 * KDevelop DUChain viewer
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include "duchainmodel.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kprocess.h>

#include "idocument.h"
#include "icore.h"
#include "ilanguagecontroller.h"
#include "backgroundparser/backgroundparser.h"
#include "backgroundparser/parsejob.h"

#include "duchainview_part.h"
#include "dumpdotgraph.h"
#include "topducontext.h"
#include "declaration.h"
#include "definition.h"
#include "parsingenvironment.h"
#include "use.h"
#include "duchain.h"
#include "duchainlock.h"
#include "duchainpointer.h"

using namespace KTextEditor;
using namespace KDevelop;

ProxyObject::ProxyObject(DUChainBase* _parent, DUChainBase* _object)
  : DUChainBase(_object->textRangePtr())
  , parent(_parent)
  , object(_object)
{
}

DUChainModel::DUChainModel(DUChainViewPart* parent)
  : QAbstractItemModel(parent)
  , m_chain(0)
{
  DUChainWriteLocker writeLock(DUChain::lock());
  DUChain::self()->addObserver(this);
  connect( part()->core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

DUChainViewPart* DUChainModel::part() const {
  return qobject_cast<DUChainViewPart*>(QObject::parent());
}

DUChainModel::~DUChainModel()
{
}

void DUChainModel::parseJobFinished(KDevelop::ParseJob* job)
{
  QMutexLocker lock(&m_mutex);
  if( job->document() == m_document && job->duChain() ) {
    lock.unlock();
    setTopContext(job->duChain());
  }
  
}

void DUChainModel::documentActivated(KDevelop::IDocument* document)
{
  if (document) {
    TopDUContext* chain = DUChain::self()->chainForDocument(document->url());
    if (chain)
      setTopContext(chain);
    else {
      QMutexLocker lock(&m_mutex);
      m_document = document->url();
    }
  }
}

void DUChainModel::setTopContext(TopDUContext* context)
{
  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  if( context )
    m_document = context->url();
  else
    m_document = KUrl();
  
  if (m_chain != context)
    m_chain = context;

  qDeleteAll(m_proxyObjects.values());
  m_proxyObjects.clear();

  m_objectLists.clear();
  m_modelRow.clear();

  reset();
}

int DUChainModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);

  return 1;
}

DUChainBase* DUChainModel::objectForIndex(const QModelIndex& index) const
{
  return static_cast<DUChainBase*>(index.internalPointer());
}

QModelIndex DUChainModel::index(int row, int column, const QModelIndex & parent) const
{
  if (row < 0 || column < 0 || column > 0 || !m_chain)
    return QModelIndex();

  if (!parent.isValid()) {
    if (row > 0)
      return QModelIndex();

    return createIndex(row, column, m_chain);
  }

  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  DUChainBase* base = objectForIndex(parent);
  if (!base)
    return QModelIndex();

  QList<DUChainBase*>* items = childItems(base);

  if (!items)
    return QModelIndex();

  if (row >= items->count())
    return QModelIndex();

  return createIndex(row, column, items->at(row));
}

QModelIndex DUChainModel::parent(const QModelIndex & index) const
{
  if (!index.isValid())
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  DUChainBase* base = objectForIndex(index);
  if (!base)
    return QModelIndex();

  if (ProxyObject* proxy = dynamic_cast<ProxyObject*>(base))
    return createParentIndex(proxy->parent);

  if (DUContext* context = dynamic_cast<DUContext*>(base))
    if (context && context->parentContext())
      return createParentIndex(context->parentContext());
    else
      return QModelIndex();

  if (Declaration* dec = dynamic_cast<Declaration*>(base))
    return createParentIndex(dec->context());

  if (Definition* def = dynamic_cast<Definition*>(base))
    return createParentIndex(def->declaration());

  if (Use* use = dynamic_cast<Use*>(base))
    return createParentIndex(use->declaration());

  // Shouldn't really hit this
  //Q_ASSERT(false);
  return QModelIndex();
}

QVariant DUChainModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  DUChainBase* base = objectForIndex(index);
  if (!base)
    return QVariant();

  ProxyObject* proxy = dynamic_cast<ProxyObject*>(base);
  if (proxy)
    base = proxy->object;

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        if (proxy)
          return i18n("Imported Context: %1", context->localScopeIdentifier().toString());
        else if (context == m_chain)
          return i18n("Top level context");
        else
          return i18n("Context: %1", context->localScopeIdentifier().toString());
    }

  } else if (Declaration* dec = dynamic_cast<Declaration*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Declaration: %1", dec->identifier().toString());
    }

  } else if (Definition* def = dynamic_cast<Definition*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Definition: %1", def->declaration()->identifier().toString());
    }

  } else if (Use* use = dynamic_cast<Use*>(base)) {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Use: %1", use->declaration() ? use->declaration()->identifier().toString() : i18n("[No definition found]"));
    }

  } else {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Unknown object!");
    }
  }

  return QVariant();
}

int DUChainModel::rowCount(const QModelIndex & parent) const
{
  if (!parent.isValid())
    return 1;

  DUChainReadLocker readLock(DUChain::lock());
  QMutexLocker lock(&m_mutex);

  DUChainBase* base = objectForIndex(parent);
  if (!base)
    return 0;

  QList<DUChainBase*>* items = childItems(base);
  if (!items)
    return 0;

  return items->count();
}

#define TEST_NEXT(iterator, index) \
      if (!first.isValid()) { \
        current = nextItem(iterator, firstInit); \
        if (current.isValid() && (current < first || !first.isValid())) { \
          first = current; \
          found = index; \
        } \
      }

QList< DUChainBase * >* DUChainModel::childItems(DUChainBase * parent) const
{
  if (m_objectLists.contains(parent))
    return m_objectLists[parent];

  ProxyObject* proxy = dynamic_cast<ProxyObject*>(parent);
  if (proxy)
    parent = proxy->object;

  QList<DUChainBase*>* list = 0;

  if (DUContext* context = dynamic_cast<DUContext*>(parent)) {
    
    QList<DUContext*> importedParentContextsData;
    foreach( DUContextPointer p, context->importedParentContexts() )
      if( p.data() )
        importedParentContextsData << p.data();

    QListIterator<DUContext*> contexts = context->childContexts();
    QListIterator<DUContext*> importedParentContexts = importedParentContextsData;
    QListIterator<Declaration*> declarations = context->localDeclarations();
    QListIterator<Definition*> definitions = context->localDefinitions();
    QListIterator<Use*> uses = context->uses();


    bool firstInit = true;
    forever {
      DUChainBase* currentItem = 0;
      Cursor first = Cursor::invalid(), current;
      int found = 0;

      TEST_NEXT(contexts, 1)
      TEST_NEXT(importedParentContexts, 2)
      TEST_NEXT(declarations, 3)
      TEST_NEXT(definitions, 4)
      TEST_NEXT(uses, 5)

      if (first.isValid()) {
        switch (found) {
          case 1:
            currentItem = item(contexts);
            break;
          case 2:
            currentItem = proxyItem(context, importedParentContexts);
            break;
          case 3:
            currentItem = item(declarations);
            break;
          case 4:
            currentItem = proxyItem(context, definitions);
            break;
          case 5:
            currentItem = proxyItem(context, uses);
            break;
          default:
            Q_ASSERT(false);
            break;
        }

        if (!list)
          list = new QList<DUChainBase*>();

        m_modelRow[currentItem] = list->count();
        list->append(currentItem);

        first = Cursor::invalid();
      } else {
        break;
      }

      firstInit = false;
    }

  } else if (Declaration* dec = dynamic_cast<Declaration*>(parent)) {
    if (dec->definition()) {
      list = new QList<DUChainBase*>();
      list->append(static_cast<DUChainBase*>(dec->definition()));
    }

    foreach (Use* use, dec->uses()) {
      if (!list)
        list = new QList<DUChainBase*>();

      list->append(static_cast<DUChainBase*>(use));
    }

  } else {
    // No child items for definitions or uses
    //kDebug(9500) << "No child items for definitions or uses";
  }

  m_objectLists.insert(parent, list);

  return list;
}

/*bool DUChainModel::hasChildren(const QModelIndex & parent) const
{
  if (!parent.isValid())
    return true;

  QMutexLocker lock(&m_mutex);
  DUChainReadLocker readLock(DUChain::lock());

  DUChainBase* base = objectForIndex(parent);
  if (!base)
    return false;

  if (m_objectLists.contains(base))
    return !m_objectLists[base]->isEmpty();

  ProxyObject* proxy = dynamic_cast<ProxyObject*>(base);
  if (proxy)
    base = proxy->object;

  if (DUContext* context = dynamic_cast<DUContext*>(base))
    return !context->childContexts().isEmpty() || !context->importedParentContexts().isEmpty() || !context->localDeclarations().isEmpty() || !context->localDefinitions().isEmpty() || !context->uses().isEmpty();

  else if (Declaration* dec = dynamic_cast<Declaration*>(base))
    return dec->definition() || !dec->uses().isEmpty();

  return false;
}*/

void DUChainModel::contextChanged(DUContext * context, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  QMutexLocker lock(&m_mutex);

  if (!m_objectLists.contains(context) || !m_modelRow.contains(context))
    return;

  QList<DUChainBase*>* list = m_objectLists[context];

  switch (relationship) {
    case ChildContexts:
      switch (change) {
        case Deletion:
        case Removal: {
          if (m_objectLists.contains(context))
            m_objectLists.remove(context);
          // fallthrough
        }
        if( context == m_chain ) {
          //Top-context was deleted
          setTopContext(0);
          return;
        }
          

        case Change: {
          int index = list->indexOf(relatedObject);
          Q_ASSERT(index != -1);
          beginRemoveRows(createIndex(m_modelRow[context], 0, context), index, index);
          list->removeAt(index);
          endRemoveRows();
          if (change == Removal || change == Deletion)
            break;
          // else fallthrough
        }

        case Addition: {
          int index = findInsertIndex(*list, relatedObject);
          beginInsertRows(createIndex(m_modelRow[context], 0, context), index, index);
          list->insert(index, relatedObject);
          endInsertRows();
          break;
        }
      }
      break;

    default:
      break;
  }
}

void DUChainModel::declarationChanged(Declaration * declaration, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(declaration);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void DUChainModel::definitionChanged(Definition * definition, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(definition);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

void DUChainModel::useChanged(Use * use, Modification change, Relationship relationship, DUChainBase * relatedObject)
{
  Q_UNUSED(use);
  Q_UNUSED(change);
  Q_UNUSED(relationship);
  Q_UNUSED(relatedObject);
}

int DUChainModel::findInsertIndex(QList<DUChainBase*>& list, DUChainBase* object) const
{
  for (int i = 0; i < list.count(); ++i)
    if (list.at(i)->textRange().start() > object->textRange().start())
      return i;

  return list.count();
}

void DUChainModel::doubleClicked ( const QModelIndex & index ) {
  if( index.isValid() ) {
    DUChainBase* base = objectForIndex(index);
    DUContext* ctx = dynamic_cast<DUContext*>(base);
    if( base && !ctx && dynamic_cast<Declaration*>(base) )
      ctx = static_cast<Declaration*>(base)->internalContext();
    
    if(ctx) {
      KTemporaryFile tempFile;

      {
        DUChainReadLocker readLock(DUChain::lock());
        QMutexLocker lock(&m_mutex);


        QString suffix = (dynamic_cast<TopDUContext*>(ctx) && static_cast<TopDUContext*>(ctx)->parsingEnvironmentFile()) ?
                            static_cast<TopDUContext*>(ctx)->parsingEnvironmentFile()->identity().toString()
                            : ctx->localScopeIdentifier().toString();
        suffix = suffix.replace('/', '_');
        suffix = suffix.replace(':', '.');
        suffix = suffix.replace(' ', '_');
        suffix += ".temp.dot";
        tempFile.setSuffix( suffix );
      
      
        if( tempFile.open() ) {
          DumpDotGraph dump;
          tempFile.write( dump.dotGraph( ctx ).toLocal8Bit() ); //Shorten if it is a top-context, because it would become too much output
        } else {
          readLock.unlock();
          lock.unlock();
          KMessageBox::error(0, i18n("Cannot create temporary file \"%1\" with suffix \"%2\"", tempFile.fileName(), suffix));
        }
      }
      tempFile.setAutoRemove(false);
      QString fileName = tempFile.fileName();
      tempFile.close();
      kDebug(9500) << "Wrote dot-graph of context " << ctx << " into " << fileName << endl;
      KProcess proc; ///@todo this is a simple hack. Maybe do it with mime-types etc.
      proc << "dotty" << fileName;
      if( !proc.startDetached() ) {
        KProcess proc2;
        proc2 << "kgraphviewer" << fileName;
        if( !proc2.startDetached() ) {
          KMessageBox::error(0, i18n("Could not open %1 with kgraphviewer or dotty.", fileName));
        }
      }
      
    }
  }
}

#include "duchainmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
