/*
 * KDevelop Generic Code Completion Support
 *
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

#include "codecompletionitem.h"

#include <QModelIndex>

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include "../duchain/declaration.h"
#include "../duchain/duchainutils.h"


using namespace KTextEditor;
namespace KDevelop {

///Intermediate nodes
struct CompletionTreeNode;
///Leaf items
class CompletionTreeItem;

CompletionTreeElement::CompletionTreeElement() : m_parent(0), m_rowInParent(0) {
}

CompletionTreeElement::~CompletionTreeElement() {
}

CompletionTreeElement* CompletionTreeElement::parent() const {
  return m_parent;
}

void CompletionTreeElement::setParent(CompletionTreeElement* parent) {
    Q_ASSERT(m_parent == 0);

    m_parent = parent;
    CompletionTreeNode* node = parent->asNode();
    if( node ) {
      m_rowInParent = node->children.count();
    }
}

void CompletionTreeNode::appendChildren(QList< QExplicitlySharedDataPointer< KDevelop::CompletionTreeElement > > children) {
  foreach (const auto& child, children) {
    appendChild(child);
  }
}

void CompletionTreeNode::appendChildren(QList< QExplicitlySharedDataPointer< KDevelop::CompletionTreeItem > > children) {
  foreach (auto child, children) {
    appendChild(CompletionTreeElementPointer(child.data()));
  }
}

void CompletionTreeNode::appendChild(QExplicitlySharedDataPointer< KDevelop::CompletionTreeElement > child) {
  child->setParent(this);
  children << child;
}

int CompletionTreeElement::columnInParent() const {
  return 0;
}

CompletionTreeNode::CompletionTreeNode() : CompletionTreeElement() {
}
CompletionTreeNode::~CompletionTreeNode() {
}

CompletionTreeNode* CompletionTreeElement::asNode() {
  return dynamic_cast<CompletionTreeNode*>(this);
}

CompletionTreeItem* CompletionTreeElement::asItem() {
  return dynamic_cast<CompletionTreeItem*>(this);
}

const CompletionTreeNode* CompletionTreeElement::asNode() const {
  return dynamic_cast<const CompletionTreeNode*>(this);
}

const CompletionTreeItem* CompletionTreeElement::asItem() const {
  return dynamic_cast<const CompletionTreeItem*>(this);
}

int CompletionTreeElement::rowInParent() const {
  return m_rowInParent;
}

void CompletionTreeItem::execute(KTextEditor::View* view, const KTextEditor::Range& word) {
  Q_UNUSED(view)
  Q_UNUSED(word)
  kDebug(9700) << "doing nothing";
}

QVariant CompletionTreeItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const {
  Q_UNUSED(index)
  Q_UNUSED(model)
  if(role == Qt::DisplayRole)
    return QString("not implemented");
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

KTextEditor::CodeCompletionModel::CompletionProperties CompletionTreeItem::completionProperties() const {
  Declaration* dec = declaration().data();
  if(!dec) {
    return {};
  }

  return DUChainUtils::completionProperties(dec);
}

DeclarationPointer CompletionTreeItem::declaration() const {
  return DeclarationPointer();
}

QList<IndexedType> CompletionTreeItem::typeForArgumentMatching() const {
  return QList<IndexedType>();
}

CompletionCustomGroupNode::CompletionCustomGroupNode(QString groupName, int _inheritanceDepth) {
  role = (KTextEditor::CodeCompletionModel::ExtraItemDataRoles)Qt::DisplayRole;
  roleValue = groupName;
  inheritanceDepth = _inheritanceDepth;
}

bool CompletionTreeItem::dataChangedWithInput() const {
  return false;
}

}
