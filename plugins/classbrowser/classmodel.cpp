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

#include <QMap>

#include "interfaces/idocument.h"
#include "interfaces/icore.h"
#include "interfaces/ilanguagecontroller.h"
#include "interfaces/iprojectcontroller.h"
#include "interfaces/iproject.h"

#include "language/backgroundparser/backgroundparser.h"
#include "language/backgroundparser/parsejob.h"

#include "language/duchain/topducontext.h"
#include "language/duchain/classmemberdeclaration.h"
#include "language/duchain/classfunctiondeclaration.h"
#include "language/duchain/parsingenvironment.h"
#include "language/duchain/duchain.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/duchainutils.h"
#include "language/duchain/codemodel.h"
#include "language/duchain/types/functiontype.h"
#include "language/duchain/persistentsymboltable.h"

#include "classbrowserplugin.h"
#include <language/duchain/functiondefinition.h>

//#include "modeltest.h"

using namespace KTextEditor;
using namespace KDevelop;

ClassModel::ClassModel(ClassBrowserPlugin* parent)
  : QAbstractItemModel(parent)
  , m_topNode(0L)
  , m_globalFunctions(0)
  , m_globalVariables(0)
  , m_filterDocument(0L)
  , m_filterProject(true)
  , m_loading(false)
{
  //new ModelTest(this);

  bool success = connect(DUChain::self()->notifier(), SIGNAL(branchAdded(KDevelop::DUContextPointer)), SLOT(branchAdded(KDevelop::DUContextPointer)), Qt::QueuedConnection);
  success &= connect(DUChain::self()->notifier(), SIGNAL(branchModified(KDevelop::DUContextPointer)), SLOT(branchModified(KDevelop::DUContextPointer)), Qt::QueuedConnection);

  success &= connect (ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), this, SLOT(projectOpened(KDevelop::IProject*)));
  success &= connect (ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), this, SLOT(projectClosing(KDevelop::IProject*)));

  resetModel();

  Q_ASSERT(success);

  kDebug() << "Class model initialized" << success;
}

void ClassModel::projectOpened(KDevelop::IProject* project)
{
  // TODO Need to connect to signals which notify of files added or removed.
  kDebug() << project->folder().prettyUrl();

  foreach(const IndexedString& file, project->fileSet()) {
    refreshNodes(file);
  }
}

void ClassModel::projectClosing(KDevelop::IProject* project)
{
  foreach(const IndexedString& file, project->fileSet()) {
    //TODO
    //fileRemoved(file);
  }
}

ClassBrowserPlugin* ClassModel::plugin() const {
  return qobject_cast<ClassBrowserPlugin*>(QObject::parent());
}

ClassModel::~ClassModel()
{
  delete m_topNode;
}

void ClassModel::resetModel()
{
  delete m_topNode;
  delete m_globalVariables;
  delete m_globalFunctions;
  m_codeModelObjects.clear();
  m_knownObjects.clear();

  startLoading();

  initialize();

  finishLoading();

  reset();
}

void ClassModel::initialize()
{
  foreach (KDevelop::IProject* project, ICore::self()->projectController()->projects())
    projectOpened(project);
}

void ClassModel::startLoading()
{
  m_loading = true;

  m_topNode = new Node(0, 0);

  m_globalFunctions = new Node(Identifier(i18n("Global Functions")), m_topNode);
  m_globalFunctions->setKind(CodeModelItem::Function);
  m_globalFunctions->setSpecialNode();
  m_topNode->insertChild(m_globalFunctions, 0);

  m_globalVariables = new Node(Identifier(i18n("Global Variables")), m_topNode);
  m_globalVariables->setKind(CodeModelItem::Variable);
  m_globalVariables->setSpecialNode();
  m_topNode->insertChild(m_globalVariables, 0);
}

void ClassModel::finishLoading()
{
  m_loading = false;

  m_topNode->sortChildren();
  m_topNode->setChildrenDiscovered();
}

void ClassModel::setFilterDocument(KDevelop::IDocument* document)
{
  if (m_filterDocument != document) {
    m_filterProject = false;
    m_filterDocument = document;
    resetModel();
  }
}

void ClassModel::searchStringChanged(const QString& string)
{
  //TODO improve efficiency
#if 0
  if (m_searchString != string) {
    m_searchString = string;
    resetModel();
  }
#endif
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

  if (!m_searchString.isEmpty())
    if (Declaration* declaration = dynamic_cast<Declaration*>(object))
      // TODO regexp?
      if (!declaration->identifier().toString().contains(m_searchString, Qt::CaseInsensitive))
        return true;

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
    return m_topNode;

  return static_cast<Node*>(index.internalPointer());
}

KDevelop::DUChainBasePointer ClassModel::duObjectForIndex(const QModelIndex& index) const
{
  Node* n = objectForIndex(index);

  if (n) {
    getDuObject( n );
    return n->duObject();
  }

  return KDevelop::DUChainBasePointer();
}

ClassModel::Node* ClassModel::objectForIdentifier(const KDevelop::IndexedQualifiedIdentifier& identifier) const
{
  if (m_codeModelObjects.contains(identifier))
    return m_codeModelObjects[identifier];

  return 0;
}

ClassModel::Node* ClassModel::objectForIdentifier(const KDevelop::QualifiedIdentifier& identifier) const
{
  if (identifier.isEmpty())
    return 0;

  IndexedQualifiedIdentifier id(identifier);

  if (!m_codeModelObjects.contains(id))
    return 0;

  return m_codeModelObjects[id];
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex & parentIndex) const
{
  if (row < 0 || column < 0 || column > 0)
    return QModelIndex();

  DUChainReadLocker readLock(DUChain::lock());

  Node* node;
  if (!parentIndex.isValid()) {
    node = m_topNode;

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

  if (parent->kind() == CodeModelItem::Class || parent->kind() == CodeModelItem::Namespace)
    return true;

  return false;
}

int ClassModel::rowCount(const QModelIndex & parentIndex) const
{
  Node* parent = objectForIndex(parentIndex);
  if (!parent)
    return 0;

  return discover(parent)->children().count();
}

QModelIndex ClassModel::parent(const QModelIndex & index) const
{
  if (!index.isValid())
    return QModelIndex();

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
    case AbstractType::TypeAbstract:
      ret += 3;
    default:
      ret += 5;
  }

  return ret;
}

int codeModelScore(ClassModel::Node* n)
{
  switch (n->kind()) {
    case CodeModelItem::Namespace:
      return 0;
    case CodeModelItem::Class:
      return 1;
    case CodeModelItem::Function:
      return 2;
    case CodeModelItem::Variable:
      return 3;
    default:
      return 4;
  }
}

bool ClassModel::orderItems(ClassModel::Node* p1, ClassModel::Node* p2)
{
  int codeModelScore1 = codeModelScore(p1);
  int codeModelScore2 = codeModelScore(p2);

  //kDebug() << p1->qualifiedIdentifier().toString() << p2->qualifiedIdentifier().toString() << codeModelScore1 << codeModelScore2;

  if (codeModelScore1 < codeModelScore2)
    return true;
  if (codeModelScore1 > codeModelScore2)
    return false;

  if (codeModelScore1 == codeModelScore2) {
    switch (p1->kind()) {
      case CodeModelItem::Namespace:
      case CodeModelItem::Class:
        goto compareNames;
      default:
        // Look up duchain info to see if they should be sorted on the basis of that first
        break;
    }
  }

  /*if (Declaration* d = dynamic_cast<Declaration*>(p1->duObject().data())) {
    if (Declaration* d2 = dynamic_cast<Declaration*>(p2->duObject().data())) {
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
  }*/

  compareNames:

  QString s1 = ClassModel::data(p1).toString();
  QString s2 = ClassModel::data(p2).toString();

  int result = QString::localeAwareCompare(s1, s2);

  //kDebug() << "String compare" << result << (result < 0);

  return result < 0;
}

void ClassModel::Node::addRelevantFile(const IndexedString& file)
{
  m_relevantFiles.insert(file);
}

const QSet<KDevelop::IndexedString>& ClassModel::Node::relevantFiles() const
{
  return m_relevantFiles;
}

const KDevelop::Identifier& ClassModel::Node::identifier() const
{
  return m_id;
}

KDevelop::QualifiedIdentifier ClassModel::Node::qualifiedIdentifier() const
{
  if (!parent() || isSpecialNode())
    return KDevelop::QualifiedIdentifier();

  KDevelop::QualifiedIdentifier id = parent() ? parent()->qualifiedIdentifier() : KDevelop::QualifiedIdentifier();
  id.push(identifier());
  return id;
}

KDevelop::CodeModelItem::Kind ClassModel::Node::kind() const
{
  return m_kind;
}

void ClassModel::Node::setKind(KDevelop::CodeModelItem::Kind kind)
{
  m_kind = kind;
}

const KDevelop::DUChainBasePointer& ClassModel::Node::duObject() const
{
  return m_duobject;
}

void ClassModel::Node::setDuObject(KDevelop::DUChainBase* object)
{
  m_duobject = object;
}

void ClassModel::refreshNodes(const IndexedString& file, int level, const QualifiedIdentifier& from) const
{
  //kDebug() << file.str() << level << from.toString();

  uint itemCount = 0;
  const CodeModelItem* items;

  CodeModel::self().items(file, itemCount, items);

  for(uint a = 0; a < itemCount; ++a) {
    // Don't insert unknown or forward declarations into the class browser
    if (items[a].kind == CodeModelItem::Unknown || items[a].kind == CodeModelItem::ForwardDeclaration)
      continue;

    IndexedQualifiedIdentifier id(items[a].id);
    Node* topParent = m_topNode;

    switch (items[a].kind) {
      case CodeModelItem::Function:
        if (level == 1)
          topParent = m_globalFunctions;
        break;
      case CodeModelItem::Variable:
        if (level == 1)
          topParent = m_globalVariables;
        break;
      default:
        break;
    }

    // Check if we already have this node
    Node* n = objectForIdentifier(id);

    if (n) {
      // We already have the relevant node.
      //kDebug() << "Found preexisting node " << n->qualifiedIdentifier().toString();
      continue;

    } else {
      QualifiedIdentifier qi = id.identifier();
      if (level > qi.count()) {
        //kDebug() << "Greater level than id count";
        continue;
      }

      if (!from.isEmpty() && !qi.beginsWith(from)) {
        //kDebug() << "Does not begin with identifier of interest";
        continue;
      }

      bool isSubIdentifier = false;
      if (level != qi.count()) {
        //kDebug() << "Sub-identifier " << qi.left(level).toString();
        isSubIdentifier = true;

        n = objectForIdentifier(qi.left(level));
      }

      if (!n) {
        int newLevel = level;
        for (int i = level - 1; i > 0; --i) {
          n = objectForIdentifier(qi.left(i));
          if (n) {
            newLevel = i + 1;
            break;
          }
        }

        if (!n) {
          n = topParent;
          newLevel = 1;
        }

        // Found it, generate nodes up to this level
        for (int i = newLevel; i <= level; ++i) {
          Node* newChild = createPointer(qi.left(i), n);
          if (i == level)
            newChild->setKind(items[a].kind);

          n->insertChild( newChild, m_loading ? 0 : this);
          n = newChild;
          //kDebug() << "Created node " << n->qualifiedIdentifier().toString();
        }

      } else {
        //kDebug() << "Found sub-identifier node";
      }

      if (!n->relevantFiles().contains(file)) {
        n->addRelevantFile( file );
        if (n->childrenDiscovered())
          // TODO may lead to too much refreshing?
          refreshNode(n);
      }
    }
  }
}

void ClassModel::refreshNode(Node* node, KDevelop::DUChainBase* base, QList<Node*>* resultChildren) const
{
  ENSURE_CHAIN_READ_LOCKED

  Q_ASSERT(node);

  bool childrenDiscovered = node->childrenDiscovered();
  node->setChildrenDiscovering();

  if (childrenDiscovered)
    node->resetEncounteredStatus();

  if (!base) {
    // Load objects from the code model
    int level = node->depth();
    foreach (const IndexedString& file, node->relevantFiles()) {
      refreshNodes(file, level, node->qualifiedIdentifier());
    }

  } else {
    if (DUContext* context = contextForBase(base)) {
      if (!filterObject(context)) {
        switch (context->type()) {
          default:
          case DUContext::Class:
            // We only add the definitions, not the contexts
            foreach (Declaration* declaration, context->localDeclarations())
              declaration->identifier().isEmpty();
            foreach (Declaration* declaration, context->localDeclarations()) {
              if (!declaration->isForwardDeclaration() && !filterObject(declaration)) {

                if (!m_filterDocument && dynamic_cast<FunctionDefinition*>(declaration))
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
  }

  if (childrenDiscovered) {
    node->removeStaleItems(this);

  } else {
    node->sortChildren();
    node->setChildrenDiscovered();
  }
}

void ClassModel::getDuObject(Node* node)
{
  if (node->duObject())
    return;

  // Find the object from the symbol table
  uint count;
  const IndexedDeclaration* declarations;
  PersistentSymbolTable::self().declarations(node->qualifiedIdentifier(), count, declarations);

  for (uint i = 0; i < count; ++i) {
    if(node->relevantFiles().contains(declarations[i].indexedTopContext().url())) {
      if(Declaration* d = declarations[i].declaration()) {
        node->setDuObject(d);
        return;
      }
    }
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

  if (context) {
    branchAddedInternal(context.data());
  }
}

void ClassModel::branchAddedInternal(KDevelop::DUContext * context)
{
  Node* node = pointer(trueParent(context->parentContext()));

  if (!node)
    node = objectForIdentifier(context->scopeIdentifier(true));

  if (node && node->childrenDiscovered())
    refreshNode(node);
  // Else, the parent node is not yet discovered, it will be figured out later if needed
}

void ClassModel::branchModified(KDevelop::DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context) {
    Node* node = pointer(trueParent(context->parentContext()));

    if (!node)
      node = objectForIdentifier(context->scopeIdentifier(true));

    if (node && node->childrenDiscovered())
      refreshNode(node);
  }
}

void ClassModel::branchRemoved(DUContextPointer context, DUContextPointer parentContext)
{
#if 0
  DUChainReadLocker readLock(DUChain::lock());

  if (context) {
    Node* parent = pointer(trueParent(parentContext.data()));

    if (!parent)
      parent = objectForIdentifier(parentContext->scopeIdentifier(true));

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
#endif
}

ClassModel::Node* ClassModel::pointer(DUChainBase* object) const
{
  Node* ret = 0L;

  if (object &&m_knownObjects.contains(object))
    ret = m_knownObjects[object];

  return ret;
}

ClassModel::Node* ClassModel::createPointer(const KDevelop::QualifiedIdentifier& id, Node* parent) const
{
  IndexedQualifiedIdentifier iid = id;

  if (m_codeModelObjects.contains(iid))
    return m_codeModelObjects[iid];

  Node* n = new Node(id.last(), parent);
  m_codeModelObjects.insert(iid, n);
  return n;
}

ClassModel::Node* ClassModel::createPointer(DUContext* context, Node* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

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
  // Don't retrieve the duchains where not required
  if (node->duObject() || (node->kind() != CodeModelItem::Class || node->kind() != CodeModelItem::Namespace)) {
    DUChainReadLocker readLock(DUChain::lock());

    getDuObject(node);
    DUChainBase* base = node->duObject().data();

    if (base) {
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
            if(dynamic_cast<FunctionDefinition*>(dec)) {
              Declaration* decl = static_cast<FunctionDefinition*>(dec)->declaration();
              if(decl) {
                dec = decl;
                fullScope = true;
              }
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
    }
  }

  switch (role) {
    case Qt::DisplayRole:
      return node->identifier().toString();
    case Qt::DecorationRole:
      switch (node->kind()) {
        case CodeModelItem::Namespace:
          return KIcon("namespace");
        case CodeModelItem::Class:
          return KIcon("class");
        default:
          break;
      }
    default:
      break;
  }


  return QVariant();
}


Declaration* ClassModel::declarationForObject(const DUChainBasePointer& pointer) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!pointer)
    return 0L;

  if (Declaration* declaration = dynamic_cast<Declaration*>(pointer.data())) {

    if(FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(declaration))
      if(def->declaration())
        return def->declaration();

    return declaration;

  } else if (DUContext* context = dynamic_cast<DUContext*>(pointer.data())) {
    if (context->owner())
      if(dynamic_cast<FunctionDefinition*>(context->owner()) && static_cast<FunctionDefinition*>(context->owner())->declaration())
        return static_cast<FunctionDefinition*>(context->owner())->declaration();
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
    if(!dynamic_cast<FunctionDefinition*>(d)) {
      if(FunctionDefinition::definition(d))
        return FunctionDefinition::definition(d);
      else
        return 0L;
    }
    return d;
  } else if (DUContext* context = dynamic_cast<DUContext*>(pointer.data())) {
    if (context->owner()) {
      if(context->owner()->isDefinition())
        return context->owner();
      else
        if(FunctionDefinition::definition(context->owner()))
          return FunctionDefinition::definition(context->owner());
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
  }

  if (model)
    const_cast<ClassModel*>(model)->beginInsertRows(model->indexForObject(this), index, index);

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

ClassModel::Node* ClassModel::discover(Node * node) const
{
  if (!node->childrenDiscovering()) {
    DUChainReadLocker readLock(DUChain::lock());

    refreshNode(node);
  }

  return node;
}


#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

ClassModel::Node * ClassModel::Node::findChild(KDevelop::DUChainBase * base) const
{
  foreach (Node* child, m_children)
    if (child->duObject().data() == base)
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

ClassModel::Node::Node(const KDevelop::Identifier& id, Node* parent)
  : m_parent(parent)
  , m_childrenEncountered(0)
  , m_kind(CodeModelItem::Unknown)
  , m_id(id)
  , m_childrenDiscovering(false)
  , m_childrenDiscovered(false)
  , m_childrenHidden(false)
  , m_specialNode(false)
{
}

ClassModel::Node::Node(KDevelop::DUChainBase * p, Node * parent)
  : m_parent(parent)
  , m_childrenEncountered(0)
  , m_kind(CodeModelItem::Unknown)
  , m_duobject(p)
  , m_childrenDiscovering(false)
  , m_childrenDiscovered(false)
  , m_childrenHidden(false)
  , m_specialNode(false)
{
}

bool ClassModel::Node::isSpecialNode() const
{
  return m_specialNode;
}

void ClassModel::Node::setSpecialNode()
{
  m_specialNode = true;
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

