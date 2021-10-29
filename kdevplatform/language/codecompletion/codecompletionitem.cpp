/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecompletionitem.h"

#include <QModelIndex>

#include <KLocalizedString>

#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/Range>
#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <debug.h>
#include "../duchain/declaration.h"
#include "../duchain/duchainutils.h"

using namespace KTextEditor;
namespace KDevelop {
///Intermediate nodes
struct CompletionTreeNode;
///Leaf items
class CompletionTreeItem;

CompletionTreeElement::CompletionTreeElement() : m_parent(nullptr)
    , m_rowInParent(0)
{
}

CompletionTreeElement::~CompletionTreeElement()
{
}

CompletionTreeElement* CompletionTreeElement::parent() const
{
    return m_parent;
}

void CompletionTreeElement::setParent(CompletionTreeElement* parent)
{
    Q_ASSERT(m_parent == nullptr);

    m_parent = parent;
    auto node = parent ? parent->asNode() : nullptr;
    if (node) {
        m_rowInParent = node->children.count();
    }
}

void CompletionTreeNode::appendChildren(
    const QList<QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement>>& children)
{
    for (const auto& child : children) {
        appendChild(child);
    }
}

void CompletionTreeNode::appendChildren(
    const QList<QExplicitlySharedDataPointer<KDevelop::CompletionTreeItem>>& children)
{
    for (const auto& child : children) {
        appendChild(CompletionTreeElementPointer(child.data()));
    }
}

void CompletionTreeNode::appendChild(QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement> child)
{
    child->setParent(this);
    children << child;
}

int CompletionTreeElement::columnInParent() const
{
    return 0;
}

CompletionTreeNode::CompletionTreeNode() : CompletionTreeElement()
    , role(( KTextEditor::CodeCompletionModel::ExtraItemDataRoles )Qt::DisplayRole)
{}

CompletionTreeNode::~CompletionTreeNode()
{
}

CompletionTreeNode* CompletionTreeElement::asNode()
{
    return dynamic_cast<CompletionTreeNode*>(this);
}

CompletionTreeItem* CompletionTreeElement::asItem()
{
    return dynamic_cast<CompletionTreeItem*>(this);
}

const CompletionTreeNode* CompletionTreeElement::asNode() const
{
    return dynamic_cast<const CompletionTreeNode*>(this);
}

const CompletionTreeItem* CompletionTreeElement::asItem() const
{
    return dynamic_cast<const CompletionTreeItem*>(this);
}

int CompletionTreeElement::rowInParent() const
{
    return m_rowInParent;
}

void CompletionTreeItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
    Q_UNUSED(view)
    Q_UNUSED(word)
    qCWarning(LANGUAGE) << "doing nothing";
}

QVariant CompletionTreeItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
    Q_UNUSED(index)
    Q_UNUSED(model)
    if (role == Qt::DisplayRole)
        return i18n("not implemented");
    return QVariant();
}

int CompletionTreeItem::inheritanceDepth() const
{
    return 0;
}

int CompletionTreeItem::argumentHintDepth() const
{
    return 0;
}

KTextEditor::CodeCompletionModel::CompletionProperties CompletionTreeItem::completionProperties() const
{
    Declaration* dec = declaration().data();
    if (!dec) {
        return {};
    }

    return DUChainUtils::completionProperties(dec);
}

DeclarationPointer CompletionTreeItem::declaration() const
{
    return DeclarationPointer();
}

QList<IndexedType> CompletionTreeItem::typeForArgumentMatching() const
{
    return QList<IndexedType>();
}

CompletionCustomGroupNode::CompletionCustomGroupNode(const QString& groupName, int _inheritanceDepth)
{
    role = ( KTextEditor::CodeCompletionModel::ExtraItemDataRoles )Qt::DisplayRole;
    roleValue = groupName;
    inheritanceDepth = _inheritanceDepth;
}

bool CompletionTreeItem::dataChangedWithInput() const
{
    return false;
}
}
