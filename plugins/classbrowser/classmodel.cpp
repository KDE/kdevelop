/*
 * KDevelop Class Browser
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "classmodel.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kprocess.h>

#include "idocument.h"
#include "icore.h"
#include "ilanguagecontroller.h"
#include "iprojectcontroller.h"
#include "iproject.h"

#include "backgroundparser/backgroundparser.h"
#include "backgroundparser/parsejob.h"

#include "classbrowserplugin.h"
#include "topducontext.h"
#include "declaration.h"
#include "parsingenvironment.h"
#include "duchain.h"
#include "duchainlock.h"
#include "duchainutils.h"

//#include "modeltest.h"

using namespace KTextEditor;
using namespace KDevelop;

ClassModel::ClassModel(ClassBrowserPlugin* parent)
  : QAbstractItemModel(parent)
  , m_topList(0L)
  , m_filterDocument(0L)
  , m_filterProject(true)
{
  //new ModelTest(this);

  bool success = connect(DUChain::self()->notifier(), SIGNAL(branchAdded(KDevelop::DUContextPointer)), SLOT(branchAdded(KDevelop::DUContextPointer)), Qt::QueuedConnection);
  Q_ASSERT(success);
}

ClassBrowserPlugin* ClassModel::plugin() const {
  return qobject_cast<ClassBrowserPlugin*>(QObject::parent());
}

ClassModel::~ClassModel()
{
  delete m_topList;
  qDeleteAll(m_lists);
  qDeleteAll(m_knownObjects);
}

void ClassModel::resetModel()
{
  delete m_topList;
  m_topList = 0L;
  qDeleteAll(m_lists);
  m_lists.clear();
  qDeleteAll(m_knownObjects);
  m_knownObjects.clear();
  m_namespaces.clear();

  reset();
}

void ClassModel::setFilterDocument(KDevelop::IDocument* document)
{
  if (m_filterDocument != document) {
    m_filterProject = false;
    m_filterDocument = document;
    resetModel();
  }
}

void ClassModel::setFilterByProject(bool filterByProject)
{
  if (m_filterProject != filterByProject) {
    m_filterProject = filterByProject;
    resetModel();
  }
}

bool ClassModel::filterObject(DUChainBase* object) const
{
  ENSURE_CHAIN_READ_LOCKED
  
  KUrl url(object->url().str());

  if (m_filterDocument)
    return m_filterDocument && !(url == m_filterDocument->url());

  if (m_filterProject) {
    if (m_inProject.contains(url))
      return m_inProject[url];

    bool ret = !plugin()->core()->projectController()->findProjectForUrl(url);

    if (ret)
      foreach (IProject* project,  plugin()->core()->projectController()->projects()) {
        if (project->folder().isParentOf(url)) {
          ret = false;
          break;
        }
      }

    //kDebug() << "Is file" << object->url().prettyUrl() << "in a project?" << !ret;
    m_inProject.insert(url, ret);
    return ret;
  }

  return false;
}

int ClassModel::columnCount(const QModelIndex & parent) const
{
  Q_UNUSED(parent);

  return 1;
}

ClassModel::Node* ClassModel::objectForIndex(const QModelIndex& index) const
{
  return static_cast<Node*>(index.internalPointer());
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex & parentIndex) const
{
  if (row < 0 || column < 0 || column > 0)
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  QList<Node*>* children;
  if (!parentIndex.isValid()) {
    children = childItems(0L);

  } else {
    Node* parent = objectForIndex(parentIndex);
    children = childItems(parent);
  }

  if (!children)
    return QModelIndex();

  if (row < children->count())
    return createIndex(row, column, children->at(row));

  return QModelIndex();
}

bool ClassModel::hasChildren(const QModelIndex& parentIndex) const
{
  DUChainReadLocker readLock(DUChain::lock());

  Node* parent = objectForIndex(parentIndex);
  if (!parent)
    if (QList<Node*>* children = childItems(parent))
      return children->count();
    else
      return false;

  if (!*parent)
    return false;

  DUContext* context = dynamic_cast<DUContext*>(parent->data());
  if (!context)
    return false;

  if (!context->localDeclarations().isEmpty())
    return true;

  if (context->childContexts().isEmpty())
    return false;

  if (QList<Node*>* children = childItems(parent))
    return children->count();
  else
    return false;
}

int ClassModel::rowCount(const QModelIndex & parentIndex) const
{
  DUChainReadLocker readLock(DUChain::lock());

  Node* parent = objectForIndex(parentIndex);
  if (parent && !*parent)
    return 0;

  QList<Node*>* children = childItems(parent);
  if (!children)
    return 0;

  return children->count();
}

QModelIndex ClassModel::parent(const QModelIndex & index) const
{
  if (!index.isValid())
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  Node* base = objectForIndex(index);
  if (!base)
    return QModelIndex();

  Node* parent = base->parent();

  if (!parent)
    return QModelIndex();

  Node* grandParent = parent->parent();

  QList<Node*>* list = childItems(grandParent);

  int i = list->indexOf(parent);
  Q_ASSERT(i != -1);

  return createIndex(i, 0, parent);
}

bool ClassModel::orderItems(ClassModel::Node* p1, ClassModel::Node* p2)
{
  if (DUContext* d = dynamic_cast<DUContext*>(p1->data())) {
    if (dynamic_cast<Declaration*>(p2->data()))
      return true;

    if (DUContext* d2 = dynamic_cast<DUContext*>(p2->data())) {
      if (d2->type() != d->type())
        if (d->type() == DUContext::Namespace)
          return true;
        else
          return false;
    }

  } else if (Declaration* d = dynamic_cast<Declaration*>(p1->data())) {
    if (DUContext* d = dynamic_cast<DUContext*>(p2->data()))
      return false;

  }

  QString s1 = ClassModel::data(p1).toString();
  QString s2 = ClassModel::data(p2).toString();

  return QString::localeAwareCompare(s1, s2) < 0;
}

QList<ClassModel::Node*>* ClassModel::childItems(Node* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!parent) {
    if (m_topList)
      return m_topList;

  } else if (m_lists.contains(parent)) {
    return m_lists[parent];
  }

  QList<Node*>* list = new QList<Node*>();

  if (parent) {
    if (DUContext* parentContext = dynamic_cast<DUContext*>(parent->data()))
      addTopLevelToList(parentContext, list, parent);

    foreach (DUContextPointer nsContext, parent->namespaceContexts())
      addTopLevelToList(nsContext.data(), list, parent);

  } else {
    foreach (TopDUContext* chain, DUChain::self()->allChains())
      addTopLevelToList(chain, list, parent);
  }

  qSort(list->begin(), list->end(), orderItems);

  if (!parent)
    m_topList = list;
  else
    m_lists.insert(parent, list);

  return list;
}

void ClassModel::addTopLevelToList(DUContext* context, QList<Node*>* list, Node* parent, bool first) const
{
  ENSURE_CHAIN_READ_LOCKED

  foreach (DUContext* child, context->childContexts()) {
    if (filterObject(child))
      continue;

    switch (child->type()) {
      case DUContext::Class:
        if (child->owner())
          list->append(createPointer(child, parent));
        break;

      case DUContext::Namespace: {
        Node* ns;
        if (m_namespaces.contains(child->scopeIdentifier())) {
          ns = m_namespaces[child->scopeIdentifier()];

          if (list->contains(ns))
            break;

        } else {
          ns = createPointer(child, parent);
        }

        // FIXME must emit changes here??
        ns->addNamespaceContext(DUContextPointer(child));

        if (!list->contains(ns))
          list->append(ns);

        break;
      }

      default:
        addTopLevelToList(child, list, parent, false);
        break;
    }
  }

  if (first) {
    foreach (Declaration* declaration, context->localDeclarations())
      if (declaration->isForwardDeclaration() && !filterObject(declaration))
        list->append(createPointer(declaration, parent));
  }
}

DUContext* ClassModel::trueParent(DUContext* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

  while (parent) {
    switch (parent->type()) {
      case DUContext::Class:
      case DUContext::Namespace:
        return parent;
      default:
        break;
    }

    parent = parent->parentContext();
  }

  return 0L;
}

void ClassModel::branchAdded(DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context)
    contextAdded(pointer(trueParent(context->parentContext())), context.data());
}

void ClassModel::contextAdded(Node* parent, DUContext* context)
{
  ENSURE_CHAIN_READ_LOCKED

  if (parent && !m_lists.contains(parent))
    // The parent node is not yet discovered, it will be figured out later if needed
    return;

  if (filterObject(context))
    return;

  if ((context->type() == DUContext::Class && context->owner()) || context->type() == DUContext::Namespace) {
    QList<Node*>* list = childItems(parent);
    if (!list) {
      kWarning() << "Could not find list of child objects for" << parent;
      return;
    }

    if (context->type() == DUContext::Namespace) {
      if (m_namespaces.contains(context->scopeIdentifier()))
        // This namespace is already known
        return;
    }

    Node* contextPointer = createPointer(context, parent);

    QList<Node*>::Iterator it = qUpperBound(list->begin(), list->end(), contextPointer, orderItems);

    int index = 0;
    if (it != list->end())
      index = list->indexOf(*it);
    else
      index = list->count();

    beginInsertRows(QModelIndex(), index, index);
    list->insert(index, contextPointer);
    endInsertRows();

  } else {
    foreach (DUContext* child, context->childContexts())
      contextAdded(parent, child);
  }
}

void ClassModel::branchRemoved(DUContextPointer context, DUContextPointer parent)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context)
    contextRemoved(pointer(trueParent(parent.data())), context.data());
}

void ClassModel::contextRemoved(Node* parent, DUContext* context)
{
  ENSURE_CHAIN_READ_LOCKED

  if (parent && !m_lists.contains(parent))
    // The parent node is not yet discovered, it will be figured out later if needed
    return;

  if (context->type() == DUContext::Class || context->type() == DUContext::Namespace) {
    QList<Node*>* list = childItems(parent);
    if (!list) {
      kWarning() << "Could not find list of child objects for" << parent;
      return;
    }

    Node* cn = pointer(context);
    if (!cn) {
      // Strange, not currently known
      kWarning() << "Unknown context removal requested - why is it unknown?";
      return;
    }

    int index = list->indexOf(cn);
    if (index == -1)
      return;

    beginRemoveRows(QModelIndex(), index, index);
    list->removeAt(index);
    endRemoveRows();
  }
}

ClassModel::Node* ClassModel::pointer(DUChainBase* object) const
{
  Node* ret = 0L;

  if (object &&m_knownObjects.contains(object))
    ret = m_knownObjects[object];

  return ret;
}

ClassModel::Node* ClassModel::createPointer(DUContext* context, Node* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (context->type() == DUContext::Namespace) {
    Q_ASSERT(!m_namespaces.contains(context->scopeIdentifier()));

    Node* n = createPointer(static_cast<DUChainBase*>(context), parent);
    m_namespaces.insert(context->scopeIdentifier(), n);

    return n;
  }

  return createPointer(static_cast<DUChainBase*>(context), parent);
}

ClassModel::Node* ClassModel::createPointer(DUChainBase* object, Node* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

  Node* ret;

  if (!m_knownObjects.contains(object)) {
    ret = new Node(object, parent);
    m_knownObjects.insert(object, ret);

  } else {
    ret = m_knownObjects[object];
  }

  return ret;
}

QVariant ClassModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  Node* basep = objectForIndex(index);
  return data(basep, role);
}

QVariant ClassModel::data(Node* node, int role)
{
  DUChainReadLocker readLock(DUChain::lock());

  DUChainBase* base = node->data();
  if (!base)
    return QVariant();

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    switch (context->type()) {
      case DUContext::Class:
        if (context->owner()) {
          if (Declaration* declaration = context->owner()) {
            if(context->owner()->isDefinition() && context->owner()->declaration())
              declaration = context->owner()->declaration();
            
            switch (role) {
              case Qt::DisplayRole:
                return declaration->identifier().toString();
              case Qt::DecorationRole:
                return DUChainUtils::iconForDeclaration(declaration);
            }
          }
        }

      case DUContext::Namespace:
        switch (role) {
          case Qt::DisplayRole:
            return context->localScopeIdentifier().toString();
          case Qt::DecorationRole:
            return KIcon("namespace");
        }
    }

  } else if (Declaration* dec = dynamic_cast<Declaration*>(base)) {
    switch (role) {
      case Qt::DisplayRole: {
        if(dec->isDefinition() && dec->declaration())
          dec = dec->declaration();
        
        QString ret = dec->identifier().toString();
        if (FunctionType::Ptr type = dec->type<FunctionType>())
          ret += type->partToString(FunctionType::SignatureArguments);
        return ret;
      }
      case Qt::DecorationRole:
        return DUChainUtils::iconForDeclaration(dec);
    }

  } else {
    switch (role) {
      case Qt::DisplayRole:
        return i18n("Unknown object!");
    }
  }

  return QVariant();
}


Declaration* ClassModel::declarationForObject(const DUChainBasePointer& pointer) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!pointer)
    return 0L;

  if (Declaration* declaration = dynamic_cast<Declaration*>(pointer.data())) {

    if(declaration->isDefinition() && declaration->declaration())
      return declaration->declaration();
    
    return declaration;

  } else if (DUContext* context = dynamic_cast<DUContext*>(pointer.data())) {
    if (context->owner())
      if(context->owner()->declaration())
        return context->owner()->declaration();
      else
        return context->owner();
  }

  return 0L;
}

Declaration* ClassModel::definitionForObject(const DUChainBasePointer& pointer) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!pointer)
    return 0L;

  if (Declaration* d = dynamic_cast<Declaration*>(pointer.data())) {
    if(!d->isDefinition()) {
      if(d->definition())
        return d->definition();
      else
        return 0L;
    }
    return d;
  } else if (DUContext* context = dynamic_cast<DUContext*>(pointer.data())) {
    if (context->owner()) {
      if(context->owner()->isDefinition())
        return context->owner();
      else
        if(context->owner()->definition())
          return context->owner()->definition();
    }
  }

  return 0L;
}

#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
