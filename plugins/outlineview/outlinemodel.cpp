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

#include <debug.h>
#include "outlinenode.h"

using namespace KDevelop;

OutlineModel::OutlineModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_lastDoc(nullptr)
{
    auto docController = ICore::self()->documentController();
    // build the initial outline now
    rebuildOutline(docController->activeDocument());
    // we must always have a valid root node
    Q_ASSERT(m_rootNode);

    // we want to rebuild the outline whenever the current document has been reparsed
    connect(DUChain::self(), &DUChain::updateReady,
            this, [this] (const IndexedString& document, const ReferencedTopDUContext& /*topContext*/) {
                if (document == m_lastUrl) {
                    rebuildOutline(m_lastDoc);
                }
            });
    // and also when we switch the current document
    connect(docController, &IDocumentController::documentActivated,
            this, &OutlineModel::rebuildOutline);
    connect(docController, &IDocumentController::documentClosed,
            this, [this](IDocument* doc) {
        if (doc == m_lastDoc) {
            m_lastDoc = nullptr;
            m_lastUrl = IndexedString();
            rebuildOutline(nullptr);
        }
    });
    connect(docController, &IDocumentController::documentUrlChanged,
            this, [this](IDocument* doc) {
        if (doc == m_lastDoc) {
            m_lastUrl = IndexedString(doc->url());
        }
    });

}

OutlineModel::~OutlineModel()
{
}

Qt::ItemFlags OutlineModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

int OutlineModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant OutlineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.column() != 0) {
        return QVariant();
    }

    auto* node = static_cast<OutlineNode*>(index.internalPointer());
    Q_ASSERT(node);
    if (role == Qt::DecorationRole) {
        return node->icon();
    }
    if (role == Qt::DisplayRole) {
        return node->text();
    }
    return QVariant();
}

bool OutlineModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int OutlineModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        Q_ASSERT(m_rootNode);
        return m_rootNode->childCount();
    } else if (parent.column() != 0) {
        return 0;
    } else {
        const auto* node = static_cast<const OutlineNode*>(parent.internalPointer());
        return node->childCount();
    }
}

QModelIndex OutlineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        // topLevelItem
        if (row < m_rootNode->childCount()) {
            return createIndex(row, column, const_cast<OutlineNode*>(m_rootNode->childAt(row)));
        }
        return QModelIndex();
    } else {
        if (parent.column() != 0) {
            return QModelIndex(); //only column 0 should have children
        }
        auto* node = static_cast<OutlineNode*>(parent.internalPointer());
        if (row < node->childCount()) {
            return createIndex(row, column, const_cast<OutlineNode*>(node->childAt(row)));
        }
        return QModelIndex(); // out of range
    }
    return QModelIndex();
}

QModelIndex OutlineModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    const auto* node = static_cast<const OutlineNode*>(index.internalPointer());

    const OutlineNode* parentNode = node->parent();
    Q_ASSERT(parentNode);
    if (parentNode == m_rootNode.get()) {
        return QModelIndex(); //node is a top level item
    }

    // parent node was not m_rootNode -> parent() must be valid
    const OutlineNode* parentParentNode = parentNode->parent();
    Q_ASSERT(parentNode);
    const int row = parentParentNode->indexOf(parentNode);
    return createIndex(row, 0, const_cast<OutlineNode*>(parentNode));
}

void OutlineModel::rebuildOutline(IDocument* doc)
{
    beginResetModel();
    if (!doc) {
        m_rootNode = OutlineNode::dummyNode();
    } else {
        // TODO: do this in a separate thread? Might take a while for large documents
        // and we really shouldn't be blocking the GUI thread!
        DUChainReadLocker lock;
        TopDUContext* topContext = DUChainUtils::standardContextForUrl(doc->url());
        if (topContext) {
            m_rootNode = OutlineNode::fromTopContext(topContext);
        } else {
            m_rootNode = OutlineNode::dummyNode();
        }
    }
    if (doc != m_lastDoc) {
        m_lastUrl = doc ? IndexedString(doc->url()) : IndexedString();
        m_lastDoc = doc;
    }
    endResetModel();
}

void OutlineModel::activate(const QModelIndex& realIndex)
{
    if (!realIndex.isValid()) {
        qCWarning(PLUGIN_OUTLINE) << "attempting to activate invalid item!";
        return;
    }
    auto* node = static_cast<OutlineNode*>(realIndex.internalPointer());
    KTextEditor::Range range;
    {
        DUChainReadLocker lock;
        const DUChainBase* dcb = node->duChainObject();
        if (!dcb) {
            qCDebug(PLUGIN_OUTLINE) << "No declaration exists for node:" << node->text();
            return;
        }
        //foreground thread == GUI thread? if so then we are fine
        range = dcb->rangeInCurrentRevision();
        //outline view should ALWAYS correspond to currently active document
        Q_ASSERT(dcb->url().toUrl() == ICore::self()->documentController()->activeDocument()->url());
        // lock should be released before activating the document
    }
    ICore::self()->documentController()->activateDocument(m_lastDoc, range);
}
