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

#include "selectionmanager.h"

#include "../test.h"
#include "test_p.h"
#include "overlaytoggle.h"

#include <QAbstractItemView>
#include <QModelIndex>

using Veritas::SelectionManager;
using Veritas::Test;
using Veritas::OverlayManager;
using Veritas::OverlayButton;

SelectionManager::SelectionManager(QAbstractItemView* parent) :
    OverlayManager(parent)
{}

void SelectionManager::setButton(OverlayButton* button)
{
    connect(button, SIGNAL(clicked(bool)), SLOT(setItemSelected(bool)));
    OverlayManager::setButton(button);
}

SelectionManager::~SelectionManager()
{}


void SelectionManager::slotEntered(const QModelIndex& index)
{
    if (index.isValid()) {
        Test* t = index2Test(index);
        Q_ASSERT(t);
        button()->setChecked(t->internal()->isChecked());
    }
    OverlayManager::slotEntered(index);
}


void SelectionManager::setItemSelected(bool selected)
{
    const QModelIndex index = button()->index();
    if (index.isValid()) {
        selected ? index2Test(index)->internal()->check() : index2Test(index)->internal()->unCheck();
        view()->viewport()->update();
    }
    emit selectionChanged();
}

#include "selectionmanager.moc"
