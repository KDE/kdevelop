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

#include <KDebug>

#include <QHBoxLayout>
#include <QListView>
#include <QLayout>
#include <QLabel>
#include <QSignalMapper>

using namespace KDevelop;

class KDevelop::MultiLevelListViewPrivate
{
public:
    MultiLevelListViewPrivate();
    ~MultiLevelListViewPrivate();

    void currentChanged(int i);
    void connectSignals(MultiLevelListView* parent);

    int levels;
    QList<QListView*> listViews;
    QList<QVBoxLayout*> layouts;
    QList<QLabel*> labels;

    QSignalMapper* mapper;
    QAbstractItemModel* model;
};

MultiLevelListViewPrivate::MultiLevelListViewPrivate()
{
    levels = 0;
    mapper = new QSignalMapper;
    model = 0;
}

MultiLevelListViewPrivate::~MultiLevelListViewPrivate()
{
    delete mapper;
}

void MultiLevelListViewPrivate::currentChanged(int i)
{
    kDebug() << i;
    Q_ASSERT(i < levels - 1);
    QModelIndex index = listViews[i]->currentIndex();

    listViews[i+1]->setRootIndex(index);
    listViews[i+1]->setCurrentIndex(index.child(0, 0));
}

void MultiLevelListViewPrivate::connectSignals(MultiLevelListView* parent)
{
    for (int i = 0; i < levels; ++i)
    {
        QListView* view = listViews[i];
        mapper->setMapping(view->selectionModel(), i);
        if (i == levels - 1)
        {
            QObject::connect (view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    parent, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)));
        }
        else
        {
            QObject::connect (view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), mapper, SLOT(map()));
        }
    }
}

MultiLevelListView::MultiLevelListView(QWidget* parent, Qt::WindowFlags f, int levels)
: QWidget(parent, f)
, d(new MultiLevelListViewPrivate)
{
    Q_ASSERT(levels > 1);

    delete layout();
    setLayout(new QHBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);

    connect(d->mapper, SIGNAL(mapped(int)), SLOT(currentChanged(int)));
}

int MultiLevelListView::levels() const
{
    return d->levels;
}

void MultiLevelListView::setLevels (int levels)
{
    qDeleteAll(d->listViews);
    qDeleteAll(d->layouts);
    qDeleteAll(d->labels);
    d->listViews.clear();
    d->layouts.clear();
    d->labels.clear();

    d->levels = levels;

    for (int i = 0; i < d->levels; ++i)
    {
        QVBoxLayout* levelLayout = new QVBoxLayout();

        QListView* view = new QListView(this);
        view->setContentsMargins(0, 0, 0, 0);

        QLabel* label = new QLabel(this);
        levelLayout->addWidget(label);

        levelLayout->addWidget(view);
        layout()->addItem(levelLayout);

        d->labels << label;
        d->layouts << levelLayout;
        d->listViews << view;
    }

    setModel(d->model);
}

MultiLevelListView::~MultiLevelListView()
{
    delete d;
}

QAbstractItemModel* MultiLevelListView::model() const
{
    return d->model;
}

void MultiLevelListView::setModel(QAbstractItemModel* model)
{
    d->model = model;

    foreach (QListView* view, d->listViews)
    {
        view->setModel(model);
    }

    d->connectSignals(this);

    if (model && !d->listViews.isEmpty())
    {
        d->listViews.first()->setCurrentIndex(model->index(0,0));
    }
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

void MultiLevelListView::setHeaderLabels (const QStringList& labels)
{
    int n = qMin(d->levels, labels.size());
    for (int i = 0; i < n; ++i)
    {
        d->labels[i]->setText(labels[i]);
    }
}


#include "multilevellistview.moc"
