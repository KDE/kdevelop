/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "multilevellistview.h"
#include <QHBoxLayout>
#include <QListView>

using namespace KDevelop;

class KDevelop::MultiLevelListViewPrivate
{
public:
    MultiLevelListViewPrivate(int levels);
    
    int levels;
    QList<QListView*> listViews;
};

MultiLevelListViewPrivate::MultiLevelListViewPrivate (int levels) : levels(levels)
{
    
}

MultiLevelListView::MultiLevelListView (int levels, QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f),
d(new MultiLevelListViewPrivate(levels))
{
    Q_ASSERT(levels > 1);
    QHBoxLayout* layout = new QHBoxLayout();
    
    // Create the list views
    QListView* lastView = 0;
    for (int i = 0; i < levels; ++i)
    {
        QListView* view = new QListView(this);
        if (lastView)
        {
            connect (lastView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), view, SLOT(setRootIndex(QModelIndex)));
        }
        layout->addWidget(view);
        d->listViews << view;
        lastView = view;
    }
    
    connect (lastView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));
    
    setLayout(layout);
}

MultiLevelListView::~MultiLevelListView()
{
    delete d;
}

void MultiLevelListView::setModel (QAbstractItemModel* model)
{
    foreach (QListView* view, d->listViews)
    {
        view->setModel(model);
    }
    foreach (QListView* view, d->listViews)
    {
        view->setCurrentIndex(view->rootIndex().child(0,0));
        view->setModel(model);
    }
}

QListView* MultiLevelListView::viewForLevel (int level)
{
    return d->listViews[level];
}

QModelIndex MultiLevelListView::currentIndex()
{
    int level = 0;
    QModelIndex index = d->listViews.first()->currentIndex();
    if (index.child(0,0).isValid())
    {
        ++level;
        index = d->listViews[level]->currentIndex();
    }
    return d->listViews.last()->currentIndex();
}

void MultiLevelListView::setCurrentIndex (const QModelIndex& index)
{
    QModelIndex idx(index);
    QModelIndexList indexes;
    while (idx.isValid() && indexes.size() < d->levels)
    {
        indexes.prepend(idx);
        idx = idx.parent();
    }
    
    Q_ASSERT(indexes.size() == d->levels);
    
    for (int i = 0; i < d->levels; ++i)
    {
        d->listViews[i]->setCurrentIndex(indexes[i]);
    }
}

void MultiLevelListView::setRootIndex (const QModelIndex& index)
{
    d->listViews.first()->setRootIndex(index);
}

