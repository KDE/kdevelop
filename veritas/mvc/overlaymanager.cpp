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

#include "overlaymanager.h"

#include "veritas/test.h"
#include "veritas/utils.h"

#include "runnermodel.h"
#include "overlaytoggle.h"

#include <KDebug>
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QModelIndex>
#include <QRect>

using Veritas::OverlayManager;
using Veritas::OverlayButton;
using Veritas::Test;
using Veritas::RunnerModel;

OverlayManager::OverlayManager(QAbstractItemView* parent) :
    QObject(parent),
    m_view(parent),
    m_toggle(0)
{
    Q_ASSERT(m_view);
}

void OverlayManager::setButton(OverlayButton* toggle)
{
    Q_ASSERT(toggle);
    m_toggle = toggle;
    m_toggle->setCheckable(true);
    m_toggle->hide();
}

void OverlayManager::reset()
{
    button()->reset();
}

OverlayButton* OverlayManager::button() const
{
    Q_ASSERT(m_toggle);
    return m_toggle;
}

void OverlayManager::makeConnections()
{
    Q_ASSERT(m_view);

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

OverlayManager::~OverlayManager()
{}

void OverlayManager::slotEntered(const QModelIndex& index)
{
    kDebug() << "";

    Q_ASSERT(m_toggle);
    m_toggle->hide();
    if (index.isValid() ){
        Test* t = index2Test(index);
        if (!m_toggle->shouldShow(t)) return;
        const QRect rect = m_view->visualRect(index);
        const int x = rect.right() - m_toggle->offset();
        const int y = rect.top();
        m_toggle->move(QPoint(x, y));
        m_toggle->show();
    }
    m_toggle->setIndex(index);
}

void OverlayManager::slotViewportEntered()
{
    kDebug() << "";
    Q_ASSERT(m_toggle);
    m_toggle->hide();
}

void OverlayManager::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent); Q_UNUSED(start); Q_UNUSED(end);
    m_toggle->hide();
}

void OverlayManager::slotSelectionChanged(const QItemSelection& selected,
                                          const QItemSelection& deselected)
{
    kDebug() << "";
    Q_UNUSED(deselected); Q_ASSERT(m_toggle);
    QModelIndexList indexes = selected.indexes();
    if (indexes.isEmpty()) {
        m_toggle->hide();
        return;
    }
    slotEntered(indexes.first());
}

Test* OverlayManager::index2Test(const QModelIndex& index) const
{
    Q_ASSERT(m_view); Q_ASSERT(m_view->model());
    QAbstractProxyModel* proxyModel = static_cast<QAbstractProxyModel*>(m_view->model());
    const QModelIndex runnerIndex = proxyModel->mapToSource(index);
    return static_cast<Test*>(runnerIndex.internalPointer());;
}

QAbstractItemView* OverlayManager::view() const
{
    Q_ASSERT(m_view);
    return m_view;
}

#include "overlaymanager.moc"
