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
    connect(parent, SIGNAL(entered(const QModelIndex&)),
            this, SLOT(slotEntered(const QModelIndex&)));
    connect(parent, SIGNAL(viewportEntered()),
            this, SLOT(slotViewportEntered()));
    m_toggle = new VerboseToggle(m_view->viewport());
    m_toggle->setCheckable(true);
    m_toggle->hide();
    connect(m_toggle, SIGNAL(clicked(bool)),
            this, SLOT(emitOpenVerbose()));
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
        if (not t || not t->shouldRun()) return;
        m_toggle->setIndex(index);
        connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
        connect(m_view->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this,
                SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));

        const QRect rect = m_view->visualRect(index);
        const int x = rect.right() - 32;
        const int y = rect.top();
        m_toggle->move(QPoint(x, y));
        m_toggle->show();
    } else {
        m_toggle->setIndex(QModelIndex());
        disconnect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                   this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
        disconnect(m_view->selectionModel(),
                   SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                   this,
                   SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
    }
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
    const QModelIndex index = m_toggle->index();
    if (index.isValid()) {
        if (selected.contains(index)) {
            m_toggle->setChecked(true);
        }

        if (deselected.contains(index)) {
            m_toggle->setChecked(false);
        }
    }
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
    RunnerModel* runnerModel = static_cast<RunnerModel*>(proxyModel->sourceModel());
    const QModelIndex runnerIndex = proxyModel->mapToSource(index);
    return testFromIndex(runnerIndex);
}

#include "verbosemanager.moc"
