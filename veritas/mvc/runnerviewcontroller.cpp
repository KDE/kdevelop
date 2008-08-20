/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
 
/*!
 * \file  runnerviewcontroller.cpp
 *
 * \brief Implements class RunnerViewController.
 */

#include "veritas/mvc/runnerviewcontroller.h"
#include "veritas/mvc/runnerproxymodel.h"
#include "veritas/mvc/runnermodel.h"

#include "veritas/test.h"
#include "veritas/utils.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <KDebug>

using Veritas::RunnerViewController;
using Veritas::RunnerProxyModel;
using Veritas::RunnerModel;
using Veritas::Test;

RunnerViewController::RunnerViewController(QObject* parent, QTreeView* view)
        : ViewControllerCommon(parent,view)
{
    // Intercept tree view events for key press handling.
    view->installEventFilter(static_cast<RunnerViewController*>(this));
}

RunnerViewController::~RunnerViewController()
{}

RunnerModel* RunnerViewController::runnerModel() const
{
    return static_cast<RunnerModel*>(model());
}

RunnerProxyModel* RunnerViewController::runnerProxyModel() const
{
    return static_cast<RunnerProxyModel*>(proxyModel());
}

void RunnerViewController::selectAll() const
{
    selectAllItems(true);
}

void RunnerViewController::unselectAll() const
{
    selectAllItems(false);
}

namespace
{
inline Test* testFromIndex(const QModelIndex& index)
{
    return static_cast<Test*>(index.internalPointer());
}
}

void RunnerViewController::selectAllItems(bool select) const
{
    QModelIndex i = proxyModel()->index(0,0);
    while (i.isValid()) { // Walk all top level indices
        QModelIndex s = proxyModel()->mapToSource(i);
        Test* t = testFromIndex(s);
        t->setSelected(select);
        runnerModel()->updateView(s); // <- emits dataChanged
        i = i.sibling(i.row() + 1, 0);
    }
    view()->viewport()->update();
    runnerModel()->countItems(); // Update counters.
    
}

bool RunnerViewController::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() != QEvent::KeyPress) {
        // Pass the event on to the parent class.
        return QObject::eventFilter(obj, event);
    }

    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    int key = keyEvent->key();

    // Tree view relevant keys make the currently highlighted row visible.
    if (key == Qt::Key_Minus || key == Qt::Key_Plus  ||
            key == Qt::Key_Left  || key == Qt::Key_Right ||
            key == Qt::Key_Up    || key == Qt::Key_Down  ||
            key == Qt::Key_Space) {
        if (highlightedRow().isValid()) {
            view()->scrollTo(highlightedRow());
        }
    }

    // Support +/- for expanding/collapsing branches.
    if (key == Qt::Key_Minus || key == Qt::Key_Plus) {
        if (key == Qt::Key_Minus) {
            key = Qt::Key_Left;
        } else {
            key = Qt::Key_Right;
        }

        // Create and send key to the tree view.
        QKeyEvent* newKeyEvent = new QKeyEvent(QEvent::KeyPress, key,
                                               keyEvent->modifiers());
        QCoreApplication::postEvent(view(), newKeyEvent);

        return true;
    }

    if (key != Qt::Key_Space) {
        return QObject::eventFilter(obj, event);
    }

    // Mimick a click of the user.
    QModelIndex testItemIndex;
    testItemIndex = Utils::modelIndexFromProxy(proxyModel(), highlightedRow());
    Test* item = testFromIndex(testItemIndex);
    bool checked = item->selected();
    item->setSelected(!checked);

    return true;
}
