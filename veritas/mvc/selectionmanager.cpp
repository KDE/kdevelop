/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *             modified by Manuel Breugelmans <mbr.nxi@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "veritas/mvc/selectionmanager.h"

#include "veritas/test.h"
#include "veritas/utils.h"

#include "veritas/mvc/runnermodel.h"
#include "veritas/mvc/selectiontoggle.h"

#include <KIconEffect>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimeLine>

using Veritas::SelectionManager;
using Veritas::SelectionToggle;
using Veritas::Test;
using Veritas::RunnerModel;

SelectionManager::SelectionManager(QAbstractItemView* parent) :
    QObject(parent),
    m_view(parent),
    m_toggle(0)
{
    m_toggle = new SelectionToggle(m_view->viewport());
    m_toggle->setCheckable(true);
    m_toggle->hide();
    connect(m_toggle, SIGNAL(clicked(bool)),
            this, SLOT(setItemSelected(bool)));
}

void SelectionManager::makeConnections()
{
    m_view->disconnect(this);
    QItemSelectionModel* selectionModel = m_view->selectionModel();
    if (selectionModel) {
        selectionModel->disconnect(this);
    }
    if (m_view->model()) {
        m_view->model()->disconnect(this);
    }

    connect(m_view, SIGNAL(entered(QModelIndex)),
            this, SLOT(slotEntered(QModelIndex)));
    connect(m_view, SIGNAL(viewportEntered()),
            this, SLOT(slotViewportEntered()));

    connect(selectionModel,
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));

    connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));

}

SelectionManager::~SelectionManager()
{}

void SelectionManager::reset()
{
    m_toggle->reset();
}

void SelectionManager::slotEntered(const QModelIndex& index)
{
    m_toggle->hide();
    if (index.isValid() ){
        m_toggle->setIndex(index);
        const QRect rect = m_view->visualRect(index);
        Test* t = itemFromIndex(index);
        if (!t) return;
        const int x = rect.right() - 16;
        const int y = rect.top();
        m_toggle->move(QPoint(x, y));

        m_toggle->setChecked(itemFromIndex(index)->selected());
        m_toggle->show();
    } else {
        m_toggle->setIndex(QModelIndex());
        disconnect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                   this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
    }
}

void SelectionManager::slotViewportEntered()
{
    m_toggle->hide();
}

void SelectionManager::setItemSelected(bool selected)
{
    const QModelIndex index = m_toggle->index();
    if (index.isValid()) {
        itemFromIndex(index)->setSelected(selected);
        QAbstractProxyModel* proxyModel = static_cast<QAbstractProxyModel*>(m_view->model());
        RunnerModel* runnerModel = static_cast<RunnerModel*>(proxyModel->sourceModel());
        runnerModel->updateView(proxyModel->mapToSource(index));
        m_view->viewport()->update();
    }
    emit selectionChanged();
}

void SelectionManager::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    m_toggle->hide();
}

void SelectionManager::slotSelectionChanged(const QItemSelection& selected,
                                            const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    QModelIndexList indexes = selected.indexes();
    if (indexes.isEmpty()) {
        m_toggle->hide();
        return;
    }
    slotEntered(indexes.first());
}

namespace
{
inline Test* testFromIndex(const QModelIndex& index)
{
    return static_cast<Test*>(index.internalPointer());
}
}

Test* SelectionManager::itemFromIndex(const QModelIndex& index) const
{
    QAbstractProxyModel* proxyModel = static_cast<QAbstractProxyModel*>(m_view->model());
    const QModelIndex runnerIndex = proxyModel->mapToSource(index);
    return testFromIndex(runnerIndex);
}

#include "selectionmanager.moc"
