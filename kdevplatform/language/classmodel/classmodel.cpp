/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classmodel.h"
#include "classmodelnode.h"
#include "allclassesfolder.h"
#include "projectfolder.h"
#include "../duchain/declaration.h"
#include <typeinfo>

#include "../../interfaces/icore.h"
#include "../../interfaces/iproject.h"
#include "../../interfaces/iprojectcontroller.h"

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

NodesModelInterface::~NodesModelInterface()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ClassModel::ClassModel()
    : m_features(NodesModelInterface::AllProjectsClasses |
        NodesModelInterface::BaseAndDerivedClasses |
        NodesModelInterface::ClassInternals)
{
    m_topNode = new FolderNode(QStringLiteral("Top Node"), this);

    if (features().testFlag(NodesModelInterface::AllProjectsClasses)) {
        m_allClassesNode = new FilteredAllClassesFolder(this);
        m_topNode->addNode(m_allClassesNode);
    }

    connect(ICore::self()->projectController(), &IProjectController::projectClosing,
            this, &ClassModel::removeProjectNode);
    connect(ICore::self()->projectController(), &IProjectController::projectOpened,
            this, &ClassModel::addProjectNode);

    const auto projects = ICore::self()->projectController()->projects();
    for (IProject* project : projects) {
        addProjectNode(project);
    }
}

ClassModel::~ClassModel()
{
    delete m_topNode;
}

void ClassModel::updateFilterString(const QString& a_newFilterString)
{
    m_allClassesNode->updateFilterString(a_newFilterString);
    for (ClassModelNodes::FilteredProjectFolder* folder : std::as_const(m_projectNodes)) {
        folder->updateFilterString(a_newFilterString);
    }
}

void ClassModel::collapsed(const QModelIndex& index)
{
    Node* node = static_cast<Node*>(index.internalPointer());

    node->collapse();
}

void ClassModel::expanded(const QModelIndex& index)
{
    Node* node = static_cast<Node*>(index.internalPointer());

    node->expand();
}

QFlags<Qt::ItemFlag> ClassModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int ClassModel::rowCount(const QModelIndex& parent) const
{
    Node* node = m_topNode;

    if (parent.isValid())
        node = static_cast<Node*>(parent.internalPointer());

    return node->children().size();
}

QVariant ClassModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node* node = static_cast<Node*>(index.internalPointer());

    if (role == Qt::DisplayRole)
        return node->displayName();

    if (role == Qt::DecorationRole) {
        QIcon icon = node->cachedIcon();
        return icon.isNull() ? QVariant() : icon;
    }

    return QVariant();
}

QVariant ClassModel::headerData(int, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole)
        return QStringLiteral("Class");

    return QVariant();
}

int ClassModel::columnCount(const QModelIndex&) const
{
    return 1;
}

bool ClassModel::hasChildren(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return true;

    Node* node = static_cast<Node*>(parent.internalPointer());

    return node->hasChildren();
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    Node* node = m_topNode;
    if (parent.isValid())
        node = static_cast<Node*>(parent.internalPointer());

    if (row >= node->children().size())
        return QModelIndex();

    return index(node->children()[row]);
}

QModelIndex ClassModel::parent(const QModelIndex& childIndex) const
{
    if (!childIndex.isValid())
        return QModelIndex();

    Node* childNode = static_cast<Node*>(childIndex.internalPointer());

    if (childNode->parent() == m_topNode)
        return QModelIndex();

    return index(childNode->parent());
}

QModelIndex ClassModel::index(ClassModelNodes::Node* a_node) const
{
    if (!a_node) {
        return QModelIndex();
    }

    // If no parent exists, we have an invalid index (root node or not part of a model).
    if (a_node->parent() == nullptr)
        return QModelIndex();

    return createIndex(a_node->row(), 0, a_node);
}

KDevelop::DUChainBase* ClassModel::duObjectForIndex(const QModelIndex& a_index)
{
    if (!a_index.isValid())
        return nullptr;

    Node* node = static_cast<Node*>(a_index.internalPointer());

    if (auto* identifierNode = dynamic_cast<IdentifierNode*>(node))
        return identifierNode->declaration();

    // Non was found.
    return nullptr;
}

QModelIndex ClassModel::indexForIdentifier(const KDevelop::IndexedQualifiedIdentifier& a_id)
{
    ClassNode* node = m_allClassesNode->findClassNode(a_id);
    if (node == nullptr)
        return QModelIndex();

    return index(node);
}

void ClassModel::nodesLayoutAboutToBeChanged(ClassModelNodes::Node*)
{
    emit layoutAboutToBeChanged();
}

void ClassModel::nodesLayoutChanged(ClassModelNodes::Node*)
{
    const QModelIndexList oldIndexList = persistentIndexList();
    QModelIndexList newIndexList;

    newIndexList.reserve(oldIndexList.size());
    for (const QModelIndex& oldIndex : oldIndexList) {
        Node* node = static_cast<Node*>(oldIndex.internalPointer());
        if (node) {
            // Re-map the index.
            newIndexList << createIndex(node->row(), 0, node);
        } else
            newIndexList << oldIndex;
    }

    changePersistentIndexList(oldIndexList, newIndexList);

    emit layoutChanged();
}

void ClassModel::nodesAboutToBeRemoved(ClassModelNodes::Node* a_parent, int a_first, int a_last)
{
    beginRemoveRows(index(a_parent), a_first, a_last);
}

void ClassModel::nodesRemoved(ClassModelNodes::Node*)
{
    endRemoveRows();
}

void ClassModel::nodesAboutToBeAdded(ClassModelNodes::Node* a_parent, int a_pos, int a_size)
{
    beginInsertRows(index(a_parent), a_pos, a_pos + a_size - 1);
}

void ClassModel::nodesAdded(ClassModelNodes::Node*)
{
    endInsertRows();
}

void ClassModel::addProjectNode(IProject* project)
{
    m_projectNodes[project] = new ClassModelNodes::FilteredProjectFolder(this, project);
    nodesLayoutAboutToBeChanged(m_projectNodes[project]);
    m_topNode->addNode(m_projectNodes[project]);
    nodesLayoutChanged(m_projectNodes[project]);
}

void ClassModel::removeProjectNode(IProject* project)
{
    m_topNode->removeNode(m_projectNodes[project]);
    m_projectNodes.remove(project);
}

#include "moc_classmodel.cpp"
