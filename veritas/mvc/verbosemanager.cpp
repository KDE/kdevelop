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

#include "veritas/mvc/verbosemanager.h"

#include "veritas/test.h"
#include "veritas/utils.h"

#include "veritas/mvc/runnermodel.h"
#include "veritas/mvc/verbosetoggle.h"

#include <KIconEffect>
#include <KDebug>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimeLine>

using Veritas::VerboseManager;
using Veritas::VerboseToggle;
using Veritas::Test;
using Veritas::RunnerModel;

VerboseManager::VerboseManager(QAbstractItemView* parent) :
    QObject(parent),
    m_view(parent),
    m_toggle(0)
{
    m_toggle = new VerboseToggle(m_view->viewport());
    m_toggle->setCheckable(true);
    m_toggle->hide();
    connect(m_toggle, SIGNAL(clicked(bool)),
            this, SLOT(emitOpenVerbose()));
}

void VerboseManager::makeConnections()
{
    m_view->disconnect(this);
    QItemSelectionModel* selectionModel = m_view->selectionModel();
    if (selectionModel) {
        selectionModel->disconnect(this);
    }
    QAbstractItemModel* model = m_view->model();
    if (model) model->disconnect(this);

    connect(m_view,
            SIGNAL(entered(QModelIndex)),
            SLOT(slotEntered(QModelIndex)));
    connect(m_view,
            SIGNAL(viewportEntered()),
            SLOT(slotViewportEntered()));

    connect(selectionModel,
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));

    connect(model,
            SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(slotRowsRemoved(QModelIndex,int,int)));
}

VerboseManager::~VerboseManager()
{}

void VerboseManager::reset()
{
    m_toggle->reset();
}

void VerboseManager::slotEntered(const QModelIndex& index)
{
    m_toggle->hide();
    if (index.isValid() ){
        Test* t = itemFromIndex(index);
        if (!t || !t->shouldRun()) return;
        const QRect rect = m_view->visualRect(index);
        const int x = rect.right() - 32;
        const int y = rect.top();
        m_toggle->move(QPoint(x, y));
        m_toggle->show();
    }
    m_toggle->setIndex(index);
}

void VerboseManager::slotViewportEntered()
{
    m_toggle->hide();
}

void VerboseManager::emitOpenVerbose()
{
    const QModelIndex index = m_toggle->index();
    if (index.isValid()) {
        Test* t = itemFromIndex(index);
        if (t) {
            emit openVerbose(t);
        }
    }
}

void VerboseManager::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    m_toggle->hide();
}

void VerboseManager::slotSelectionChanged(const QItemSelection& selected,
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

Test* VerboseManager::itemFromIndex(const QModelIndex& index) const
{
    QAbstractProxyModel* proxyModel = static_cast<QAbstractProxyModel*>(m_view->model());
    const QModelIndex runnerIndex = proxyModel->mapToSource(index);
    return testFromIndex(runnerIndex);
}

#include "verbosemanager.moc"
