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
#include <QTimer>

#include "interfaces/idocument.h"
#include "interfaces/idocumentcontroller.h"
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
#include "language/duchain/types/structuretype.h"
#include "language/duchain/persistentsymboltable.h"

#include "classbrowserplugin.h"
#include <language/duchain/functiondefinition.h>

//#include "modeltest.h"

using namespace KTextEditor;
using namespace KDevelop;

class ClassModel::Node
{
  public:
    Node(const KDevelop::Identifier& id, Node* parent);
    Node(KDevelop::DUChainBase* p, Node* parent);
    ~Node();

    bool topNode() const { return !m_parent; }

    Node* parent() const { return m_parent; }
    int depth() const { return m_parent ? m_parent->depth() + 1 : 1; }

    bool childrenDiscovered() const { return m_childrenDiscovered; }
    bool childrenDiscovering() const { return m_childrenDiscovering; }
    void setChildrenDiscovering(bool discovering = true) { m_childrenDiscovering = discovering; }
    void setChildrenDiscovered(bool discovered = true);

    QList<Node*> children() const;
    void insertChild(Node* node, const ClassModel* model);
    void removeChild(Node* node, const ClassModel* model);
    // performs an initial sorting of child items, this is only to be performed when the node's children are being discovered for the first time.
    void sortChildren();

    void resetEncounteredStatus();
    void removeStaleItems(const ClassModel* model, const IndexedString& file);
    void removeItems(const ClassModel* model, const IndexedString& file);

    const QList<DUContextPointer>& namespaceContexts() const { return m_namespaceContexts; }
    void addNamespaceContext(const DUContextPointer& context) { m_namespaceContexts.append(context); }

    void addRelevantFile(const IndexedString& file);
    void removeRelevantFile(const IndexedString& file);
    const QSet<IndexedString>& relevantFiles() const;

    const Identifier& identifier() const;
    QualifiedIdentifier qualifiedIdentifier() const;
    void setIdentifier(const Identifier& identifier);

    CodeModelItem::Kind kind() const;
    void setKind(CodeModelItem::Kind kind);

    const DUChainBasePointer& duObject() const;
    void setDuObject(DUChainBase* object);

    bool isSpecialNode() const;
    void setSpecialNode();

  private:
    Node* m_parent;
    QList<Node*> m_children;
    QVector<bool>* m_childrenEncountered;

    // For Code Model derived items
    CodeModelItem::Kind m_kind;
    Identifier m_id;
    QSet<IndexedString> m_relevantFiles;

    // For duchain derived items
    DUChainBasePointer m_duobject;

    QList<DUContextPointer> m_namespaceContexts;
    bool m_childrenDiscovering : 1;
    bool m_childrenDiscovered : 1;
    bool m_specialNode: 1;
};

ClassModel::ClassModel(ClassBrowserPlugin* parent)
  : QAbstractItemModel(parent)
  , m_topNode(0L)
  , m_globalFunctions(0)
  , m_globalVariables(0)
  , m_updateTimer(new QTimer(this))
  , m_filterDocument(0L)
  , m_filterProject(true)
  , m_loading(false)
{
  //new ModelTest(this);

  // 2 second update interval
  m_updateTimer->setInterval(2000);

  connect(DUChain::self()->notifier(), SIGNAL(branchAdded(KDevelop::DUContextPointer)), SLOT(branchAdded(KDevelop::DUContextPointer)), Qt::QueuedConnection);
  connect(DUChain::self()->notifier(), SIGNAL(branchModified(KDevelop::DUContextPointer)), SLOT(branchModified(KDevelop::DUContextPointer)), Qt::QueuedConnection);

  connect (ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), this, SLOT(projectOpened(KDevelop::IProject*)));
  connect (ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), this, SLOT(projectClosing(KDevelop::IProject*)));

  connect (ICore::self()->documentController(), SIGNAL(documentOpened(KDevelop::IDocument*)), this, SLOT(documentOpened(KDevelop::IDocument*)));
  connect (ICore::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), this, SLOT(documentClosed(KDevelop::IDocument*)));

  resetModel();
}

void ClassModel::projectOpened(KDevelop::IProject* project)
{
  // TODO Need to connect to signals which notify of files added or removed.
  kDebug() << project->folder().prettyUrl();

  foreach(const IndexedString& file, project->fileSet())
    referenceFile(file);
}

void ClassModel::referenceFile(const IndexedString& file)
{
  m_displayedFiles[file]++;
  refreshNodes(file);
}

void ClassModel::dereferenceFile(const IndexedString& file)
{
  const int ref = m_displayedFiles[file];
  if (ref == 1) {
    m_displayedFiles.remove(file);
    m_topNode->removeItems(this, file);

  } else {
    --m_displayedFiles[file];
  }
}

void ClassModel::projectClosing(KDevelop::IProject* project)
{
  foreach (const IndexedString& file, project->fileSet())
    dereferenceFile(file);
}

void ClassModel::documentOpened(KDevelop::IDocument* doc)
{
  IndexedString file(doc->url());

  referenceFile(file);
}

void ClassModel::documentClosed(KDevelop::IDocument* doc)
{
  IndexedString file(doc->url());

  dereferenceFile(file);
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
  // Deletes all nodes
  delete m_topNode;

  m_objects.clear();

  startLoading();

  initialize();

  finishLoading();

  reset();
}

void ClassModel::initialize()
{
  foreach (KDevelop::IProject* project, ICore::self()->projectController()->projects())
    projectOpened(project);

  foreach (KDevelop::IDocument* doc, ICore::self()->documentController()->openDocuments())
    documentOpened( doc );
}

void ClassModel::startLoading()
{
  m_loading = true;

  m_topNode = new Node(0, 0);

  m_globalFunctions = new Node(Identifier(i18n("Global Functions")), m_topNode);
  m_globalFunctions->setKind(CodeModelItem::Function);
  m_globalFunctions->setSpecialNode();
  m_topNode->insertChild(m_globalFunctions, this);

  m_globalVariables = new Node(Identifier(i18n("Global Variables")), m_topNode);
  m_globalVariables->setKind(CodeModelItem::Variable);
  m_globalVariables->setSpecialNode();
  m_topNode->insertChild(m_globalVariables, this);
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
  if (m_objects.contains(identifier))
    return m_objects[identifier];

  return 0;
}

ClassModel::Node* ClassModel::objectForIdentifier(const KDevelop::QualifiedIdentifier& identifier) const
{
  if (identifier.isEmpty())
    return 0;

  IndexedQualifiedIdentifier id(identifier);

  if (!m_objects.contains(id))
    return 0;

  return m_objects[id];
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
  Node* parent = objectForIndex(parentIndex);
  if (!parent)
    return false;

  if (parent->childrenDiscovered())
    return !parent->children().isEmpty();

  if (parent->isSpecialNode() || parent->kind() == CodeModelItem::Class || parent->kind() == CodeModelItem::Namespace)
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

  if (codeModelScore1 < codeModelScore2)
    return true;
  if (codeModelScore1 > codeModelScore2)
    return false;

  QString s1 = ClassModel::data(p1).toString();
  QString s2 = ClassModel::data(p2).toString();

  return QString::localeAwareCompare(s1, s2) < 0;
}

void ClassModel::Node::addRelevantFile(const IndexedString& file)
{
  m_relevantFiles.insert(file);
}

void ClassModel::Node::removeRelevantFile(const IndexedString& file)
{
  m_relevantFiles.remove(file);
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

void ClassModel::Node::setIdentifier(const KDevelop::Identifier& identifier)
{
  m_id = identifier;
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

          n->insertChild( newChild, this);
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

void ClassModel::refreshNode(Node* const node, DUContext* context, QList<Node*>* resultChildren) const
{
  Q_ASSERT(node);

  bool childrenDiscovered = node->childrenDiscovered();
  node->setChildrenDiscovering();

  if (childrenDiscovered) {
    node->resetEncounteredStatus();
    if (node == m_topNode) {
      m_globalFunctions->resetEncounteredStatus();
      m_globalVariables->resetEncounteredStatus();
    }
  }

  // Load objects from the code model
  int level = node->depth();
  foreach (const IndexedString& file, node->relevantFiles()) {
    refreshNodes(file, level, node->qualifiedIdentifier());

    if (childrenDiscovered) {
      if (node == m_topNode) {
        node->removeStaleItems(this, file);
        m_globalFunctions->removeStaleItems(this, file);
        m_globalVariables->removeStaleItems(this, file);

      } else {
        node->removeStaleItems(this, IndexedString());
      }
    }
  }

  if (!childrenDiscovered) {
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

void ClassModel::branchAdded(DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context) {
    branchChanged(context.data());
  }
}

void ClassModel::branchChanged(KDevelop::DUContext * context)
{
  Q_ASSERT(context);
  m_updateFiles.insert(context->url());

  startUpdateTimer();
}

void ClassModel::startUpdateTimer()
{
  if (!m_updateTimer->isActive())
    m_updateTimer->start();
}

void ClassModel::branchModified(KDevelop::DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (context)
    branchChanged(context.data());
}

void ClassModel::branchRemoved(DUContextPointer context, DUContextPointer parentContext)
{
  DUChainReadLocker readLock(DUChain::lock());

  if (parentContext)
    branchChanged(parentContext.data());
}

ClassModel::Node* ClassModel::createPointer(const KDevelop::QualifiedIdentifier& id, Node* parent) const
{
  IndexedQualifiedIdentifier iid = id;

  if (m_objects.contains(iid))
    return m_objects[iid];

  Node* n = new Node(id.last(), parent);
  m_objects.insert(iid, n);
  return n;
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
            if(dynamic_cast<FunctionDefinition*>(dec)) {
              Declaration* decl = static_cast<FunctionDefinition*>(dec)->declaration();
              if(decl) {
                dec = decl;
              }
            }

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


void ClassModel::Node::insertChild(Node * node, const ClassModel * model)
{
  Q_ASSERT(node->parent() == this);

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

    const_cast<ClassModel*>(model)->beginInsertRows(model->indexForObject(this), index, index);
  }

  m_children.insert(index, node);
  if (m_childrenEncountered)
    m_childrenEncountered->insert(index, true);

  if (m_childrenDiscovered)
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

  if (model) {
    const_cast<ClassModel*>(model)->endRemoveRows();
    const_cast<ClassModel*>(model)->nodeDeleted(node);
  }

  delete node;
}

void ClassModel::nodeDeleted(Node* node)
{
  m_objects.remove(node->qualifiedIdentifier());
}

void ClassModel::Node::resetEncounteredStatus()
{
  Q_ASSERT(!m_childrenEncountered);

  m_childrenEncountered = new QVector<bool>(m_children.count());
}

void ClassModel::Node::removeStaleItems(const ClassModel * model, const IndexedString& file)
{
  Q_ASSERT(m_childrenEncountered);

  // Top level node must have a file provided, so only those with the same top contexts get removed
  Q_ASSERT(m_parent || !file.isEmpty());

  for (int i = m_childrenEncountered->count() - 1; i >= 0; --i) {
    if (!m_childrenEncountered->at(i)) {
      if (file.isEmpty()) {
        removeChild(m_children[i], model);
      } else if (!m_children[i]->relevantFiles().contains(file)) {
        if (m_children[i]->relevantFiles().count() == 1)
          removeChild(m_children[i], model);
        else
          m_children[i]->removeRelevantFile(file);
      }
    }
  }

  delete m_childrenEncountered;
  m_childrenEncountered = 0;
}

void ClassModel::Node::removeItems(const ClassModel* model, const IndexedString& file)
{
  for (int i = 0; i < m_children.count(); ++i) {
    Node* child = m_children.at(i);
    if (child->relevantFiles().contains(file))
      child->removeItems(model, file);
      if (child->children().isEmpty() && child->relevantFiles().count() == 1) {
        removeChild(child, model);
      } else {
        child->removeRelevantFile( file );
      }
  }
}

void ClassModel::Node::setChildrenDiscovered(bool discovered)
{
  m_childrenDiscovered = discovered;
}

ClassModel::Node::~Node()
{
  qDeleteAll(m_children);
  delete m_childrenEncountered;
}

ClassModel::Node* ClassModel::discover(Node * node) const
{
  // Don't auto-discover the top node
  if (node == m_topNode)
    return node;

  if (!node->childrenDiscovering()) {
    DUChainReadLocker readLock(DUChain::lock());

    refreshNode(node);
  }

  return node;
}

QList< ClassModel::Node * > ClassModel::Node::children() const
{
  return m_children;
}

ClassModel::Node::Node(const KDevelop::Identifier& id, Node* parent)
  : m_parent(parent)
  , m_childrenEncountered(0)
  , m_kind(CodeModelItem::Unknown)
  , m_id(id)
  , m_childrenDiscovering(false)
  , m_childrenDiscovered(false)
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

#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
