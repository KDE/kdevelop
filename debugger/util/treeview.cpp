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

#include "treeview.h"
#include "treemodel.h"

using namespace KDevelop;

AsyncTreeView::AsyncTreeView(TreeModel* model, QWidget *parent = 0) 
: QTreeView(parent)
{
    setModel(model);
    connect (this, SIGNAL(expanded(const QModelIndex &)),
             this, SLOT(slotExpanded(const QModelIndex &)));
    connect (this, SIGNAL(collapsed(const QModelIndex &)),
             this, SLOT(slotCollapsed(const QModelIndex &)));
    connect (this, SIGNAL(clicked(const QModelIndex &)),
             this, SLOT(slotClicked(const QModelIndex &)));
}


void AsyncTreeView::slotExpanded(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->expanded(index);
}

void AsyncTreeView::slotCollapsed(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->collapsed(index);
}

void AsyncTreeView::slotClicked(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->clicked(index);
}

#include "treeview.moc"
