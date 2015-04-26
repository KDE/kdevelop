/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlinemodel.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>

#include "debug_outline.h"
#include "outlinenode.h"

using namespace KDevelop;

OutlineModel::OutlineModel(QObject* parent) : QAbstractItemModel(parent), m_lastDoc(nullptr)
{
    auto docController = ICore::self()->documentController();
    connect(docController, &IDocumentController::documentActivated,
            this, &OutlineModel::rebuildOutline);
    connect(docController, &IDocumentController::documentSaved,
            this, &OutlineModel::onDocumentSaved);
    rebuildOutline(docController->activeDocument());

    // TODO: void BackgroudParser::parseJobFinished(KDevelop::ParseJob* job)??

}

OutlineModel::~OutlineModel() {
    qDeleteAll(m_topLevelItems);
}

Qt::ItemFlags OutlineModel::flags(const QModelIndex& index) const {
    if(!index.isValid())
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int OutlineModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 1;
}

QVariant OutlineModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid())
        return QVariant();
    if(index.column() != 0)
        return QVariant();

    OutlineNode* node = static_cast<OutlineNode*>(index.internalPointer());
    Q_CHECK_PTR(node);
    if(role == Qt::DecorationRole) {
        return node->icon();
    }
    if(role == Qt::DisplayRole) {
        return node->text();
    }
    return QVariant();
}

bool OutlineModel::hasChildren(const QModelIndex& parent) const {
    if(!parent.isValid())
        return m_topLevelItems.size() > 0;

    if(parent.column() != 0)
        return false;

    OutlineNode* node = static_cast<OutlineNode*>(parent.internalPointer());
    return node->childCount() > 0;
}

int OutlineModel::rowCount(const QModelIndex& parent) const {
    if(!parent.isValid())
        return m_topLevelItems.size();

    if(parent.column() != 0)
        return 0;

    OutlineNode* node = static_cast<OutlineNode*>(parent.internalPointer());
    return node->childCount();
}

QModelIndex OutlineModel::index(int row, int column, const QModelIndex &parent) const {
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if(!parent.isValid()) {
        //topLevelItem
        if(row < m_topLevelItems.size()) {
            return createIndex(row, column, m_topLevelItems.at(row));
        }
        return QModelIndex();
    }
    else {
        if(parent.column() != 0) {
            return QModelIndex(); //only column 0 should have children
        }

        OutlineNode* node = static_cast<OutlineNode*>(parent.internalPointer());
        if(row < node->childCount()) {
            return createIndex(row, column, node->childAt(row));
        }
        return QModelIndex(); // out of range
    }
    return QModelIndex();
}

QModelIndex OutlineModel::parent(const QModelIndex& index) const
{
    if(!index.isValid()) {
        return QModelIndex();
    }

    OutlineNode* node = static_cast<OutlineNode*>(index.internalPointer());

    OutlineNode* parentNode = node->parent();

    if(!parentNode)
        return QModelIndex(); //node is a top level item

    OutlineNode* parentParentNode = parentNode->parent();
    //find the correct row if parents parent is null then we have to find the index in the top level items,
    //otherwise query the parents parent for an index
    const int row = parentParentNode ? parentParentNode->indexOf(parentNode) : m_topLevelItems.indexOf(parentNode);
    return createIndex(row, 0, parentNode);
}

void OutlineModel::onDocumentSaved(IDocument* doc)
{
    //rebuild outline whenever current document is saved (on every change is probably too expensive)
    if(m_lastDoc == doc)
        rebuildOutline(doc);
}

void OutlineModel::rebuildOutline(IDocument* doc)
{
    emit beginResetModel();
    qDeleteAll(m_topLevelItems);
    m_topLevelItems.clear();
    if (!doc) {
        return;
    }

    DUChainReadLocker lock(DUChain::lock());
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(doc->url());
    if (!topContext) {
        return;
    }
    foreach(Declaration* decl, topContext->localDeclarations()) {
        if (decl) {
            m_topLevelItems.append(new OutlineNode(decl, 0));
        }
    }
    lock.unlock(); //should not be locked anymore when signal is emitted
    m_lastDoc = doc;
    emit endResetModel();
}

void OutlineModel::activate(QModelIndex realIndex)
{
    if (!realIndex.isValid()) {
        qCDebug(PLUGIN_OUTLINE) << "attempting to activate invalid item!";
        return;
    }
    OutlineNode* node = static_cast<OutlineNode*>(realIndex.internalPointer());
    DUChainReadLocker lock(DUChain::lock());
    const Declaration* decl = node->declaration();
    if (!decl) {
        qCDebug(PLUGIN_OUTLINE) << "Declaration for node no longer exists:" << node->text();
        return;
    }
    //foreground thread == GUI thread? if so then we are fine
    KTextEditor::Range range = decl->rangeInCurrentRevision();
    lock.unlock();
    //outline view should ALWAYS correspond to currently active document
#ifndef QT_NO_DEBUG
    QUrl url = decl->url().toUrl();
    Q_ASSERT(url == ICore::self()->documentController()->activeDocument()->url());
#endif
    ICore::self()->documentController()->activateDocument(
        ICore::self()->documentController()->activeDocument(), range);
}
