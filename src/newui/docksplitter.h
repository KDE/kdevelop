/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef IDEALDOCKSPLITTER_H
#define IDEALDOCKSPLITTER_H

#include <qsplitter.h>
#include <qlist.h>

namespace Ideal {

class DockWidget;

/**
@short Splitter for docks
*/
class DockSplitter: public QSplitter {
    Q_OBJECT
public:
    DockSplitter(Qt::Orientation orientation, QWidget *parent = 0, const char *name = 0);
    ~DockSplitter();

    void addDock(int row, int col, QWidget *dock);
    void removeDock(int row, int col, bool alsoDelete = false);

    QPair<int, int> indexOf(QWidget *dock);

    int numRows() const;
    int numCols(int row) const;

protected:
    void appendSplitter();
    void createSplitters(int index);
    void shiftWidgets(QSplitter *splitter, int row, int fromCol);

    bool isRowEmpty(int row);

private:
    Qt::Orientation m_orientation;
    QList<QSplitter*> m_splitters;
    QList<QList<QWidget*> > m_docks;
};

}

#endif
