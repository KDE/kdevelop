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
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>

#include "debug_outline.h"
#include "outlinenode.h"

using namespace KDevelop;

OutlineModel::OutlineModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    auto docController = ICore::self()->documentController();
    m_lastDoc = docController->activeDocument();
    if (m_lastDoc) {
        m_lastUrl = IndexedString(m_lastDoc->url());
    }
    // we want to rebuild the outline whenever the current document has been reparsed
    connect(ICore::self()->languageController()->backgroundParser(),
            &BackgroundParser::parseJobFinished, this, &OutlineModel::onParseJobFinished);
    // and also when we switch the current document
    connect(docController, &IDocumentController::documentActivated,
            this, &OutlineModel::rebuildOutline);
    connect(docController, &IDocumentController::documentUrlChanged, this, [this](IDocument* doc) {
        if (doc == m_lastDoc) {
            m_lastUrl = IndexedString(doc->url());
        }
    });
    // build the initial outline now
    rebuildOutline(docController->activeDocument());
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

    OutlineNode* node = static_cast<OutlineNode*>(index.internalPointer());
    Q_CHECK_PTR(node);
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
    if (!parent.isValid()) {
        return m_topLevelItems.size() > 0;
    }
    if (parent.column() != 0) {
        return false;
    }

    OutlineNode* node = static_cast<OutlineNode*>(parent.internalPointer());
    return node->childCount() > 0;
}

int OutlineModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_topLevelItems.size();
    }

    if (parent.column() != 0) {
        return 0;
    }

    const OutlineNode* node = static_cast<const OutlineNode*>(parent.internalPointer());
    return node->childCount();
}

QModelIndex OutlineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        // topLevelItem
        if ((size_t)row < m_topLevelItems.size()) {
            // ! using the address is only safe since we never modify the vector once it has been created
            return createIndex(row, column, const_cast<OutlineNode*>(&m_topLevelItems.at(row)));
        }
        return QModelIndex();
    } else {
        if (parent.column() != 0) {
            return QModelIndex(); //only column 0 should have children
        }
        OutlineNode* node = static_cast<OutlineNode*>(parent.internalPointer());
        if (row < node->childCount()) {
            // ! using the address is only safe since we never modify the vector once it has been created
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

    const OutlineNode* node = static_cast<const OutlineNode*>(index.internalPointer());

    const OutlineNode* parentNode = node->parent();

    if (!parentNode) {
        return QModelIndex(); //node is a top level item
    }

    const OutlineNode* parentParentNode = parentNode->parent();
    //find the correct row if parents parent is null then we have to find the index in the top level items,
    //otherwise query the parents parent for an index
    const int row = parentParentNode ? parentParentNode->indexOf(parentNode) : OutlineNode::findNode(m_topLevelItems, parentNode);
    // ! using the address is only safe since we never modify the vector once it has been created
    return createIndex(row, 0, const_cast<OutlineNode*>(parentNode));
}

void OutlineModel::onParseJobFinished(KDevelop::ParseJob* job)
{
    if (job->document() == m_lastUrl) {
        rebuildOutline(m_lastDoc);
    }
}

void OutlineModel::rebuildOutline(IDocument* doc)
{
    emit beginResetModel();
    m_topLevelItems.clear();
    if (!doc) {
        return;
    }

    {
        DUChainReadLocker lock;
        TopDUContext* topContext = DUChainUtils::standardContextForUrl(doc->url());
        if (!topContext) {
            return;
        }
        foreach (Declaration* decl, topContext->localDeclarations()) {
            if (decl) {
                m_topLevelItems.emplace_back(decl, nullptr);
            }
        }
        // ! m_topLevelItems mustn't be modified anywhere other than this function
    } // DUChain should not be locked anymore when signal is emitted
    if (doc != m_lastDoc) {
        m_lastDoc = doc;
        m_lastUrl = IndexedString(doc->url());
    }
    emit endResetModel();
}

void OutlineModel::activate(const QModelIndex& realIndex)
{
    if (!realIndex.isValid()) {
        qCDebug(PLUGIN_OUTLINE) << "attempting to activate invalid item!";
        return;
    }
    OutlineNode* node = static_cast<OutlineNode*>(realIndex.internalPointer());
    KTextEditor::Range range;
    {
        DUChainReadLocker lock;
        const Declaration* decl = node->declaration();
        if (!decl) {
            qCDebug(PLUGIN_OUTLINE) << "Declaration for node no longer exists:" << node->text();
            return;
        }
        //foreground thread == GUI thread? if so then we are fine
        range = decl->rangeInCurrentRevision();
        //outline view should ALWAYS correspond to currently active document
        Q_ASSERT(decl->url().toUrl() == ICore::self()->documentController()->activeDocument()->url());
    }
    ICore::self()->documentController()->activateDocument(m_lastDoc, range);
}
