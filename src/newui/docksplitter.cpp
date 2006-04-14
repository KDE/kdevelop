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
#include "docksplitter.h"

#include <kdebug.h>

namespace Ideal {

DockSplitter::DockSplitter(Qt::Orientation orientation, QWidget *parent)
    :QSplitter(parent), m_orientation(orientation)
{
    switch (m_orientation)
    {
        case Qt::Horizontal:
            setOrientation(Qt::Vertical);
            break;
        case Qt::Vertical:
            setOrientation(Qt::Horizontal);
            break;
    }
    setOpaqueResize(true);
    appendSplitter();
}

DockSplitter::~DockSplitter()
{
}

void DockSplitter::addDock(int row, int col, QWidget *dock)
{
    if (m_docks.count() <= row)
        for (int i = m_docks.count(); i <= row ; ++i)
            m_docks.append(QList<QWidget*>());

    if (m_docks.at(row).count() <= col)
    {
        for (int i = m_docks.at(row).count(); i <= col ; ++i)
            m_docks[row].append(0);
        m_docks[row][col] = dock;
    }
    else if (m_docks.at(row).at(col) == 0)
        m_docks[row][col] = dock;
    else
        m_docks[row].insert(col, dock);

    if (m_splitters.count() <= row)
        createSplitters(row);
    QSplitter *splitter = m_splitters.at(row);

    if (col < m_docks.at(row).count()-1)
        shiftWidgets(splitter, row, col+1);
}

void DockSplitter::appendSplitter()
{
    QSplitter *splitter = new QSplitter(m_orientation, this);
    splitter->setOpaqueResize(true);
    splitter->show();
    m_splitters.append(splitter);
}

void DockSplitter::createSplitters(int index)
{
    kDebug() << "DockSplitter::createSplitters index = " << index << " count = " << m_splitters.count() << endl;
    for (int i = m_splitters.count(); i <= index; ++i)
    {
        kDebug() << "    appendSplitter..." << endl;
        appendSplitter();
    }
}

void DockSplitter::removeDock(int row, int col, bool alsoDelete)
{
    if ((row >= m_docks.count()) || (col >= m_docks[row].count()))
        return;

    QWidget *w = m_docks.at(row).at(col);
    m_docks[row].removeAll(m_docks.at(row).at(col));

    if (alsoDelete)
    {
        delete w;
        w = 0;
    }
    else
    {
        w->hide();
    }

    QSplitter *splitter = m_splitters.at(row);
    splitter->setMinimumSize(splitter->minimumSizeHint());

    if (isRowEmpty(row))
    {
        m_docks.removeAt(row);
        delete m_splitters.takeAt(row);
    }
}

bool DockSplitter::isRowEmpty(int row)
{
    for (int i = 0; i < m_docks.at(row).count(); ++i) {
        if (m_docks.at(row).at(i) != 0)
            return false;
    }
    return true;
}

void DockSplitter::shiftWidgets(QSplitter *splitter, int row, int fromCol)
{
    kDebug() << "shiftWidgets: row=" << row << "  from col=" << fromCol << endl;
    kDebug() << "row size is: " << m_docks[row].count() << endl;

    for (int i = fromCol; i < m_docks.at(row).count(); ++i)
    {
        kDebug() << "move from " << i << " to last" << endl;
        if (m_docks[row][i])
            splitter->moveToLast(m_docks[row][i]);
        else
            kDebug() << "m_docks[" << row << "][" << i << "] is 0" << endl;
    }
}

int DockSplitter::numRows() const
{
    return m_docks.count();
}

int DockSplitter::numCols(int row) const
{
    if (row < numRows())
        return m_docks[row].count();
    return 0;
}

QPair<int, int> DockSplitter::indexOf(QWidget *dock)
{
    for (int i = 0; i < m_docks.count(); ++i)
        for (int j = 0; j < m_docks.at(i).count(); ++j)
            if (dock == m_docks.at(i).at(j))
                return qMakePair(i, j);
    return qMakePair(0, 0);
}

}

#include "docksplitter.moc"
