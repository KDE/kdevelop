/*
 * KDevelop Class Browser
 *
 * Copyright 2007-2008 Hamish Rodda <rodda@kde.org>
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
#include "classmemberdeclaration.h"
#include "classfunctiondeclaration.h"
#include "parsingenvironment.h"
#include "duchain.h"
#include "duchainlock.h"
#include "duchainutils.h"

//#include "modeltest.h"

using namespace KTextEditor;
using namespace KDevelop;

ClassModel::ClassModel(ClassBrowserPlugin* parent)
  : QAbstractItemModel(parent)
  , m_topNode(0L)
  , m_filterDocument(0L)
  , m_filterProject(true)
{
  //new ModelTest(this);

  bool success = connect(DUChain::self()->notifier(), SIGNAL(branchAdded(KDevelop::DUContextPointer)), SLOT(branchAdded(KDevelop::DUContextPointer)), Qt::QueuedConnection);
  success &= connect(DUChain::self()->notifier(), SIGNAL(branchModified(KDevelop::DUContextPointer)), SLOT(branchModified(KDevelop::DUContextPointer)), Qt::QueuedConnection);
  Q_ASSERT(success);
}

ClassBrowserPlugin* ClassModel::plugin() const {
  return qobject_cast<ClassBrowserPlugin*>(QObject::parent());
}

ClassModel::~ClassModel()
{
  delete m_topNode;
  qDeleteAll(m_topLists);
}

void ClassModel::resetModel()
{
  delete m_topNode;
  m_topNode = 0L;
  qDeleteAll(m_topLists);
  m_topLists.clear();
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

  //If the range is empty, it was probably created from a macro like Q_OBJECT
  if(object->range().isEmpty() && dynamic_cast<Declaration*>(object))
    return true;

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
  if (!index.isValid())
    return topNode();

  return static_cast<Node*>(index.internalPointer());
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex & parentIndex) const
{
  if (row < 0 || column < 0 || column > 0)
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  Node* node;
  if (!parentIndex.isValid()) {
    node = topNode();

  } else {
    node = objectForIndex(parentIndex);
  }

  if (!node)
    return QModelIndex();

  if (row < discover(node)->children().count())
    return createIndex(row, column, node->children().at(row));

  return QModelIndex();
}

QModelIndex ClassModel::indexForObject(Node * node) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!node)
    return QModelIndex();

  if (!node->parent())
    return QModelIndex();

  if (node == m_topNode)
    return QModelIndex();

  int row = node->parent()->children().indexOf(node);
  Q_ASSERT(row != -1);

  return createIndex(row, 0, node);
}

bool ClassModel::hasChildren(const QModelIndex& parentIndex) const
{
  DUChainReadLocker readLock(DUChain::lock());

  Node* parent = objectForIndex(parentIndex);
  if (!parent)
    return false;

  if (parent->childrenDiscovered())
    return !parent->children().isEmpty();

  if (dynamic_cast<DUContext*>(parent->data()))
    return true;

  if (Declaration* definition = dynamic_cast<Declaration*>(parent->data())) {
    if (definition->kind() == Declaration::Instance)
      return false;
      
    AbstractType::Ptr type = definition->abstractType();
    if (type)
      if (type->whichType() == AbstractType::TypeStructure)
        // There may be, so be lazy and guess
        return true;
  }

  return false;
}

int ClassModel::rowCount(const QModelIndex & parentIndex) const
{
  DUChainReadLocker readLock(DUChain::lock());

  Node* parent = objectForIndex(parentIndex);
  if (!parent)
    return 0;

  return discover(parent)->children().count();
}

QModelIndex ClassModel::parent(const QModelIndex & index) const
{
  if (!index.isValid())
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  Node* base = objectForIndex(index);
  if (!base)
    return QModelIndex();

  return indexForObject(base->parent());
}

int declarationScore(Declaration* d)
{
  if (dynamic_cast<ClassFunctionDeclaration*>(d))
    return 1;

  if (dynamic_cast<ClassMemberDeclaration*>(d))
    return 2;

  if (dynamic_cast<AbstractFunctionDeclaration*>(d))
    return 1;
  
  return 0;
}

int typeScore(Declaration* d)
{
  int ret = 0;
  
  if (d->kind() == Declaration::Instance)
    return 4;

  switch (d->abstractType()->whichType()) {
    case AbstractType::TypeIntegral:
      ret += 2;
    case AbstractType::TypePointer:
      ret += 2;
    case AbstractType::TypeReference:
      ret += 2;
    case AbstractType::TypeFunction:
      ret += 1;
    case AbstractType::TypeStructure:
      ret += 0;
    case AbstractType::TypeArray:
      ret += 2;
    case AbstractType::TypeDelayed:
      ret += 3;
    case AbstractType::TypeForward:
      ret += 3;
    case AbstractType::TypeAbstract:
      ret += 3;
    default:
      ret += 5;
  }

  return ret;
}

bool ClassModel::orderItems(ClassModel::Node* p1, ClassModel::Node* p2)
{
  /*if (DUContext* d = dynamic_cast<DUContext*>(p1->data())) {
    if (dynamic_cast<Declaration*>(p2->data()))
      return true;

    if (DUContext* d2 = dynamic_cast<DUContext*>(p2->data())) {
      if (d2->type() != d->type())
        if (d->type() == DUContext::Namespace)
          return true;
        else
          return false;
    }

  } else*/
  
  if (Declaration* d = dynamic_cast<Declaration*>(p1->data())) {
    if (Declaration* d2 = dynamic_cast<Declaration*>(p2->data())) {
      if (d->kind() != d2->kind())
        if (d->kind() == Declaration::Instance)
          return false;
        else
          return true;

      int declarationScore1, declarationScore2;
      declarationScore1 = declarationScore( d );
      declarationScore2 = declarationScore( d2 );
      if (declarationScore1 < declarationScore2)
        return true;
      if (declarationScore1 > declarationScore2)
        return false;
      
      if (d->abstractType()) {
        if (d2->abstractType()) {
          if (d->abstractType() != d2->abstractType()) {              
            int typeScore1, typeScore2;
            typeScore1 = typeScore(d);
            typeScore2 = typeScore(d2);
            if (typeScore1 < typeScore2)
              return true;
            else if (typeScore1 > typeScore2)
              return false;
            // else fallthrough intended

          } // else fallthrough intended
        } else {
          return false;
        }
      } else {
        if (d2->abstractType())
          return false;
      }
    } else {
      return true;
    }
  }

  QString s1 = ClassModel::data(p1).toString();
  QString s2 = ClassModel::data(p2).toString();

  return QString::localeAwareCompare(s1, s2) < 0;
}

void ClassModel::refreshNode(Node* node, KDevelop::DUChainBase* base, QList<Node*>* resultChildren) const
{
  ENSURE_CHAIN_READ_LOCKED

  Q_ASSERT(node);

  bool childrenDiscovered = node->childrenDiscovered();
  node->setChildrenDiscovering();

  if (childrenDiscovered)
    node->resetEncounteredStatus();

  if (!base)
    base = node->data();

  if (DUContext* context = contextForBase(base)) {
    if (!filterObject(context)) {
      switch (context->type()) {
        default:
        case DUContext::Class:
          // We only add the definitions, not the contexts
          foreach (Declaration* declaration, context->localDeclarations()) {
            if (!declaration->isForwardDeclaration() && !filterObject(declaration)) {

              if (!m_filterDocument && declaration->declaration())
                // This is a definition, skip it
                continue;

              if (declaration->identifier().isEmpty())
                // Skip anonymous declarations
                continue;

              if (declaration->kind() == Declaration::NamespaceAlias)
                // Skip importing declarations
                continue;

              Node* newChild = createPointer(declaration, node);
              node->insertChild(newChild, childrenDiscovered ? this : 0);

              if (newChild->childrenDiscovered())
                refreshNode(newChild);

              if (resultChildren)
                resultChildren->append(newChild);
            }
          }
          break;

        /*case DUContext::Namespace: {
          Node* ns;
          if (m_namespaces.contains(child->scopeIdentifier())) {
            continue;

            // TODO figure out what's going on here
            ns = m_namespaces[child->scopeIdentifier()];

            if (node->children().contains(ns))
              break;

          } else {
            ns = createPointer(child, node);
          }

          ns->addNamespaceContext(DUContextPointer(child));

          node->insertChild(ns, childrenDiscovered ? this : 0);

          if (ns->childrenDiscovered())
            refreshNode(ns);

          kDebug() << "Adding namespace " << child->scopeIdentifier() << " to list";

          break;
        }*/
      }
    }
  }

  if (childrenDiscovered) {
    node->removeStaleItems(this);

  } else {
    if (!node->topNode())
      node->sortChildren();

    node->setChildrenDiscovered();
  }
}

DUContext* ClassModel::trueParent(DUContext* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

  while (parent) {
    switch (parent->type()) {
      case DUContext::Class:
        //return parent->owner();
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

  if (context) {
    branchAddedInternal(context.data());
  }
}

void ClassModel::branchAddedInternal(KDevelop::DUContext * context)
{
  Node* node = pointer(trueParent(context->parentContext()));

  if (!node) {
    QList<Node*>* topList = 0;

    if (!m_topLists.contains(context)) {
      topList = new QList<Node*>();
      m_topLists.insert(context, topList);
    } else {
      topList = m_topLists[context];
    }

    refreshNode(topNode(), context, topList);

  } else if (node->childrenDiscovered()) {
    refreshNode(node);
  }
  // Else, the parent node is not yet discovered, it will be figured out later if needed
}

void ClassModel::branchModified(KDevelop::DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context) {
    Node* node = pointer(trueParent(context->parentContext()));

    if (!node) {
      if (m_topLists.contains(context.data())) {
        QList<Node*>* topList = m_topLists[context.data()];
        QList<Node*> newTopList;

        refreshNode(topNode(), context.data(), &newTopList);

        foreach (Node* node, *topList) {
          if (!newTopList.contains(node)) {
            topNode()->removeChild(node, this);
          }
        }

        *topList = newTopList;

      } else {
        kWarning() << "Modified branch not previously encountered?";
        branchAdded(context);
      }

    } else if (node->childrenDiscovered()) {
      refreshNode(node);
    }
  }
}

void ClassModel::branchRemoved(DUContextPointer context, DUContextPointer parentContext)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context) {
    Node* parent = pointer(trueParent(parentContext.data()));

    if (!parent) {
      if (m_topLists.contains(context.data())) {
        QList<Node*>* topList = m_topLists[context.data()];
        foreach (Node* node, *topList) {
          int index = topList->indexOf(node);
          beginRemoveRows(QModelIndex(), index, index);
          topList->removeAt(index);
          endRemoveRows();
          delete node;
        }
        m_topLists.remove(context.data());
        delete topList;
      }
    }

    if (!parent->childrenDiscovered())
      // The parent node is not yet discovered, it will be figured out later if needed
      return;

    Q_ASSERT(parent->parent());
    parent->removeChild(parent, this);
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

  /*if (context->type() == DUContext::Namespace) {
    Q_ASSERT(!m_namespaces.contains(context->scopeIdentifier()));

    Node* n = createPointer(static_cast<DUChainBase*>(context), parent);
    m_namespaces.insert(context->scopeIdentifier(), n);

    return n;
  }*/

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
      case DUContext::Namespace:
        switch (role) {
          case Qt::DisplayRole:
            return context->localScopeIdentifier().toString();
          case Qt::DecorationRole:
            return KIcon("namespace");
        }

      default:
        break;
    }

  } else if (Declaration* dec = dynamic_cast<Declaration*>(base)) {
    switch (role) {
      case Qt::DisplayRole: {
        bool fullScope = false;
        if(dec->isDefinition() && dec->declaration()) {
          dec = dec->declaration();
          fullScope = true;
        }

        QString ret;
        if(!fullScope)
          ret = dec->identifier().toString();
        else
          ret = dec->qualifiedIdentifier().toString();
        
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

void ClassModel::Node::insertChild(Node * node, const ClassModel * model)
{
  if (m_children.contains(node)) {
    if (m_childrenEncountered)
      (*m_childrenEncountered)[m_children.indexOf(node)] = true;
    return;
  }

  int index = m_children.count();

  if (m_childrenDiscovered) {
    QList<Node*>::Iterator it = qUpperBound(m_children.begin(), m_children.end(), node, orderItems);

    if (it != m_children.end())
      index = m_children.indexOf(*it);
    else
      index = m_children.count();
  }

  if (model)
    const_cast<ClassModel*>(model)->beginInsertRows(model->indexForObject(node->parent()), index, index);

  m_children.insert(index, node);
  if (m_childrenEncountered)
    m_childrenEncountered->insert(index, true);

  if (model)
    const_cast<ClassModel*>(model)->endInsertRows();
}

void ClassModel::Node::removeChild(Node * node, const ClassModel * model)
{
  int index = m_children.indexOf(node);
  if (index == -1)
    return;

  if (model)
    const_cast<ClassModel*>(model)->beginRemoveRows(model->indexForObject(this), index, index);

  m_children.removeAt(index);

  if (model)
    const_cast<ClassModel*>(model)->endRemoveRows();
}

void ClassModel::Node::resetEncounteredStatus()
{
  // Top level node doesn't use this functionality, the class model coordinates this
  if (!m_parent)
    return;

  Q_ASSERT(!m_childrenEncountered);

  m_childrenEncountered = new QVector<bool>(m_children.count());
}

void ClassModel::Node::removeStaleItems(const ClassModel * model)
{
  // Top level node doesn't use this functionality, the class model coordinates this
  if (!m_parent)
    return;

  Q_ASSERT(m_childrenEncountered);

  for (int i = m_childrenEncountered->count() - 1; i >= 0; --i)
    if (!m_childrenEncountered->at(i))
      removeChild(m_children[i], model);

  delete m_childrenEncountered;
  m_childrenEncountered = 0;
}

void ClassModel::Node::setChildrenDiscovered(bool discovered)
{
  m_childrenDiscovered = discovered;
}

KDevelop::DUContext * ClassModel::contextForBase(KDevelop::DUChainBase * base) const
{
  if (DUContext* context = dynamic_cast<DUContext*>(base))
    return context;

  if (Declaration* declaration = dynamic_cast<Declaration*>(base))
    if (declaration->internalContext())
      if (declaration->internalContext()->type() == DUContext::Class)
        return declaration->internalContext();

  return 0;
}

ClassModel::Node::~ Node()
{
  qDeleteAll(m_children);
  delete m_childrenEncountered;
}

ClassModel::Node * ClassModel::topNode() const
{
  if (!m_topNode) {
    m_topNode = new Node(0, 0);

    foreach (TopDUContext* chain, DUChain::self()->allChains()) {
      m_topNode->setChildrenDiscovered(false);
      const_cast<ClassModel*>(this)->branchAddedInternal(chain);
    }

    m_topNode->sortChildren();
  }

  return m_topNode;
}

ClassModel::Node* ClassModel::discover(Node * node) const
{
  if (!node->childrenDiscovering())
    refreshNode(node);

  return node;
}


#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

ClassModel::Node * ClassModel::Node::findChild(KDevelop::DUChainBase * base) const
{
  foreach (Node* child, m_children)
    if (child->data() == base)
      return child;

  return 0;
}

void ClassModel::Node::hideChildren()
{
  m_childrenHidden = true;
}

void ClassModel::Node::showChildren()
{
  m_childrenHidden = false;
}

QList< ClassModel::Node * > ClassModel::Node::children() const
{
  if (m_childrenHidden)
    return QList<Node*>();

  return m_children;
}

ClassModel::Node::Node(KDevelop::DUChainBase * p, Node * parent)
  : KDevelop::DUChainBasePointer(p)
  , m_parent(parent)
  , m_childrenEncountered(0)
  , m_childrenDiscovering(false)
  , m_childrenDiscovered(false)
  , m_childrenHidden(false)
{
}

void ClassModel::Node::sortChildren()
{
  qSort(m_children.begin(), m_children.end(), orderItems);
}

QPair<ClassModel::Node*, DUChainBase*> ClassModel::firstKnownObjectForBranch(KDevelop::DUChainBase* base) const
{
  if (Node* n = pointer(base))
    return qMakePair(n, static_cast<DUChainBase*>(0));

  DUChainBase* oldBase = base;

  if (Declaration* declaration = dynamic_cast<Declaration*>(base))
    base = declaration->context();

  if (DUContext* context = dynamic_cast<DUContext*>(base)) {
    QualifiedIdentifier id = context->scopeIdentifier(true);

    while (id.count() > 1) {
      id.pop();

      QList<Declaration*> declarations = context->findDeclarations(id, base->range().start);
      foreach (Declaration* d, declarations)
        if (Node* n = pointer(d))
          return qMakePair(n, oldBase);

      QList<DUContext*> contexts = context->findContexts(DUContext::Class, id, base->range().start);
      foreach (DUContext* c, contexts)
        if (Node* n = pointer(c))
          return qMakePair(n, oldBase);

      contexts = context->findContexts(DUContext::Namespace, id, base->range().start);
      foreach (DUContext* c, contexts)
        if (Node* n = pointer(c))
          return qMakePair(n, oldBase);

      oldBase = context;
    }
  }

  return qMakePair(static_cast<Node*>(0), static_cast<DUChainBase*>(0));
}

