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
#include <QSignalMapper>

using namespace KDevelop;

class KDevelop::MultiLevelListViewPrivate
{
public:
    MultiLevelListViewPrivate(int levels);
    ~MultiLevelListViewPrivate();

    void currentChanged(int i);

    int levels;
    QList<QListView*> listViews;
    QList<QVBoxLayout*> layouts;
    QSignalMapper* mapper;
};

MultiLevelListViewPrivate::MultiLevelListViewPrivate(int levels)
: levels(levels)
{
    mapper = new QSignalMapper;
}

MultiLevelListViewPrivate::~MultiLevelListViewPrivate()
{
    delete mapper;
}

void MultiLevelListViewPrivate::currentChanged(int i)
{
    Q_ASSERT(i < levels - 1);
    QModelIndex index = listViews[i]->currentIndex();
    listViews[i+1]->setRootIndex(index);
    listViews[i+1]->setCurrentIndex(index.child(0, 0));
}

MultiLevelListView::MultiLevelListView(QWidget* parent, Qt::WindowFlags f, int levels)
: QWidget(parent, f)
, d(new MultiLevelListViewPrivate(levels))
{
    Q_ASSERT(levels > 1);

    connect(d->mapper, SIGNAL(mapped(int)), SLOT(currentChanged(int)));

    QHBoxLayout* layout = new QHBoxLayout();
    for (int i = 0; i < d->levels; ++i)
    {
        QVBoxLayout* levelLayout = new QVBoxLayout();

        QListView* view = new QListView(this);
        view->setContentsMargins(0, 0, 0, 0);
        levelLayout->addWidget(view);
        layout->addItem(levelLayout);

        d->layouts << levelLayout;
        d->listViews << view;
    }
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

MultiLevelListView::~MultiLevelListView()
{
    delete d;
}

void MultiLevelListView::setModel(QAbstractItemModel* model)
{
    QListView* lastView = 0;
    for (int i = 0; i < d->levels; ++i)
    {
        QListView* view = d->listViews[i];
        view->setModel(model);
        if (i == d->levels - 1)
        {
            connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));
        }
        else
        {
            d->mapper->setMapping(view->selectionModel(), i);
            connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    d->mapper, SLOT(map()));
        }
        lastView = view;
    }

    connect(lastView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));

    d->listViews.first()->setCurrentIndex(model->index(0,0));
}

QListView* MultiLevelListView::viewForLevel(int level) const
{
    return d->listViews[level];
}

QModelIndex MultiLevelListView::currentIndex() const
{
    return d->listViews.last()->currentIndex();
}

void MultiLevelListView::addWidget(int level, QWidget* widget)
{
    Q_ASSERT(level < d->levels);
    d->layouts[level]->addWidget(widget);
}

void MultiLevelListView::setCurrentIndex(const QModelIndex& index)
{
    QModelIndex previous = currentIndex();

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

    emit currentIndexChanged(index, previous);
}

void MultiLevelListView::setRootIndex(const QModelIndex& index)
{
    d->listViews.first()->setRootIndex(index);
}

#include "multilevellistview.moc"
