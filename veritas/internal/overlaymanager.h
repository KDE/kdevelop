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

#ifndef VERITAS_OVERLAYMANAGER_H
#define VERITAS_OVERLAYMANAGER_H

#include <QObject>
#include "veritas/veritasexport.h"

class QAbstractItemView;
class QModelIndex;
class QAbstractButton;
class QItemSelection;

namespace Veritas
{
class Test;
class OverlayButton;

class VERITAS_EXPORT OverlayManager : public QObject
{
Q_OBJECT

public:
    OverlayManager(QAbstractItemView* parent);
    virtual ~OverlayManager();

    /*! Initialize connections with the view
        @note This must be re-invoked after every setModel() on parent */
    void makeConnections();

    /*! @note mandatory */
    void setButton(OverlayButton*);
    Test* index2Test(const QModelIndex&) const;

public slots:
    void reset();

protected:
    QAbstractItemView* view() const;
    OverlayButton* button() const;

protected slots:
    void slotEntered(const QModelIndex& index);

private slots:
    void slotViewportEntered();
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    QAbstractItemView* m_view;
    OverlayButton* m_toggle;
};

} // namespace Veritas

#endif // VERITAS_SELECTION_MANAGER_H

