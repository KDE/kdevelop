/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "expandingtree.h"

#include <QAbstractProxyModel>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include "expandingwidgetmodel.h"

#include <util/path.h>
#include <util/widgetcolorizer.h>

using namespace KDevelop;

ExpandingTree::ExpandingTree(QWidget* parent) : QTreeView(parent)
{
    m_drawText.documentLayout()->setPaintDevice(this);
    setUniformRowHeights(false);
}

void ExpandingTree::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(!model || qobject_cast<const ExpandingWidgetModel*>(
                 qobject_cast<const QAbstractProxyModel*>(model)->sourceModel())
             );
    QTreeView::setModel(model);
}

void ExpandingTree::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QTreeView::drawRow(painter, option, index);

    const auto* eModel = qobject_cast<const ExpandingWidgetModel*>(qobject_cast<const QAbstractProxyModel*>(model())->sourceModel());
    Q_ASSERT(eModel);
    const QModelIndex sourceIndex = eModel->mapToSource(index);
    if (eModel->isPartiallyExpanded(sourceIndex) != ExpandingWidgetModel::ExpansionType::NotExpanded) {
        QRect rect = eModel->partialExpandRect(sourceIndex);
        if (rect.isValid()) {
            painter->fillRect(rect, QBrush(0xffffffff));

            QAbstractTextDocumentLayout::PaintContext ctx;
            // since arbitrary HTML can be shown use a black on white color scheme here
            ctx.palette = QPalette(Qt::black, Qt::white);
            ctx.clip = QRectF(0, 0, rect.width(), rect.height());
            painter->setViewTransformEnabled(true);
            painter->translate(rect.left(), rect.top());

            m_drawText.setHtml(eModel->partialExpandText(sourceIndex));
            m_drawText.setPageSize(QSizeF(rect.width(), rect.height()));
            m_drawText.documentLayout()->draw(painter, ctx);

            painter->translate(-rect.left(), -rect.top());
        }
    }
}

int ExpandingTree::sizeHintForColumn(int column) const
{
    return columnWidth(column);
}

void ExpandingTree::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    const auto& path = index.data(ProjectPathRole).value<Path>();
    if (path.isValid()) {
        const auto color = WidgetColorizer::colorForId(qHash(path), palette(), true);
        WidgetColorizer::drawBranches(this, painter, rect, index, color);
    }
    QTreeView::drawBranches(painter, rect, index);
}
