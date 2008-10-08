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

#ifndef VERITAS_SELECTIONMANAGER_H
#define VERITAS_SELECTIONMANAGER_H

#include <QObject>
#include "overlaymanager.h"

class QAbstractItemView;
class QModelIndex;

namespace Veritas
{

/*! Whenever a test-item is hovered by the mouse in the runner tree,
 *  a toggle button is shown. This allows the user to select/deselect that
 *  specific test and it's children. A deselected test is excluded from the
 *  test run. Individual Veritas::Test's must set the needSelectionToggle property
 *  if they want such a button. */
class SelectionManager : public OverlayManager
{
    Q_OBJECT

public:
    SelectionManager(QAbstractItemView* parent);
    virtual ~SelectionManager();
    virtual void setButton(OverlayButton*);

protected slots:
    void slotEntered(const QModelIndex&);

private slots:
    void setItemSelected(bool);

signals:
    /** Is emitted if the selection has been changed by the toggle button. */
    void selectionChanged();
};

} // namespace Veritas

#endif // VERITAS_SELECTION_MANAGER_H

