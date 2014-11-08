/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
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

#include <QApplication>
#include <QDesktopWidget>

#include "treeview.h"
#include "treemodel.h"

using namespace KDevelop;

AsyncTreeView::AsyncTreeView(TreeModel* model, QWidget *parent = 0) 
: QTreeView(parent)
{
    setModel(model);
    connect (this, &AsyncTreeView::expanded,
             this, &AsyncTreeView::slotExpanded);
    connect (this, &AsyncTreeView::collapsed,
             this, &AsyncTreeView::slotCollapsed);
    connect (this, &AsyncTreeView::clicked,
             this, &AsyncTreeView::slotClicked);
    connect (model, &TreeModel::itemChildrenReady,
            this, &AsyncTreeView::slotExpandedDataReady);
}


void AsyncTreeView::slotExpanded(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->expanded(index);
}

void AsyncTreeView::slotCollapsed(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->collapsed(index);
    resizeColumns();
}

void AsyncTreeView::slotClicked(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->clicked(index);
    resizeColumns();
}

QSize AsyncTreeView::sizeHint() const
{
    //Assuming that columns are awlays resized to fit their contents, return a size that will fit all without a scrollbar
    QMargins margins = contentsMargins();
    int horizontalSize = margins.left() + margins.right();
    for (int i = 0; i < model()->columnCount(); ++i) {
        horizontalSize += columnWidth(i);
    }
    horizontalSize = qMin(horizontalSize, QApplication::desktop()->screenGeometry().width()*3/4);
    return QSize(horizontalSize, margins.top() + margins.bottom() + sizeHintForRow(0));
}

void AsyncTreeView::resizeColumns()
{
    for (int i = 0; i < model()->columnCount(); ++i) {
        this->resizeColumnToContents(i);
    }
    this->updateGeometry();
}

void AsyncTreeView::slotExpandedDataReady()
{
    resizeColumns();
}

