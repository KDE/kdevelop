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

#ifndef VERITAS_VERBOSEMANAGER_H
#define VERITAS_VERBOSEMANAGER_H

#include "veritas/test.h"
#include <QObject>
#include "overlaymanager.h"

class QAbstractItemView;
class QModelIndex;
class QAbstractButton;
class QItemSelection;

namespace Veritas
{

class VerboseToggle;
class Test;

/*! TODO should be removed completly */
class VerboseManager : public OverlayManager
{
Q_OBJECT

public:
    VerboseManager(QAbstractItemView* parent);
    virtual ~VerboseManager();
    virtual void setButton(OverlayButton*);

signals:
    /** Emitted when the toggle has been clicked. */
    void openVerbose(Veritas::Test*);

private slots:
    void emitOpenVerbose();

};

} // namespace Veritas

#endif // VERITAS_SELECTION_MANAGER_H

