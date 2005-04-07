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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "docksplitter.h"

#include <kdebug.h>

namespace Ideal {

DockSplitter::DockSplitter(Orientation orientation, QWidget *parent, const char *name)
    :QSplitter(parent, name), m_orientation(orientation)
{
    switch (m_orientation)
    {
        case Horizontal:
            setOrientation(Vertical);
            break;
        case Vertical:
            setOrientation(Horizontal);
            break;
    }
    setOpaqueResize(true);
    appendSplitter();
}

DockSplitter::~DockSplitter()
{
}

void DockSplitter::addDock(uint row, uint col, QWidget *dock)
{
    if (m_docks.count() <= row)
        for (uint i = m_docks.count(); i <= row ; ++i)
            m_docks.append(QValueList<QWidget*>());
    
    if (m_docks[row].count() <= col)
    {
        for (uint i = m_docks[row].count(); i <= col ; ++i)
            m_docks[row].append(0);
        m_docks[row][col] = dock;
    }
    else if (m_docks[row][col] == 0)
        m_docks[row][col] = dock;
    else
        m_docks[row].insert(m_docks[row].at(col), dock);
    
    if (m_splitters.count() <= row)
        createSplitters(row);
    QSplitter *splitter = m_splitters[row];
    
    dock->reparent(splitter, QPoint(0,0), true);
    if (col < m_docks[row].count()-1)
        shiftWidgets(splitter, row, col+1);
}

void DockSplitter::appendSplitter()
{
    switch (m_orientation)
    {
        case Horizontal:
            m_splitters.append(new QSplitter(Horizontal, this));
            break;
        case Vertical:
            m_splitters.append(new QSplitter(Vertical, this));
            break;
    }
    m_splitters[m_splitters.size()-1]->setOpaqueResize(true);
    m_splitters[m_splitters.size()-1]->show();
}

void DockSplitter::createSplitters(uint index)
{
    kdDebug() << "DockSplitter::createSplitters index = " << index << " count = " << m_splitters.count() << endl; 
    for (uint i = m_splitters.count(); i <= index; ++i)
    {
        kdDebug() << "    appendSplitter..." << endl;
        appendSplitter();
    }
}

void DockSplitter::removeDock(uint row, uint col, bool alsoDelete)
{
    if ((row >= m_docks.count()) || (col >= m_docks[row].count()))
        return;
    
    QWidget *w = m_docks[row][col];
    m_docks[row].remove(m_docks[row].at(col));
    
    if (alsoDelete)
        delete w;

    m_splitters[row]->setMinimumSize(m_splitters[row]->minimumSizeHint());

    if (isRowEmpty(row))
    {
        m_docks.remove(m_docks.at(row));
        delete m_splitters[row];
        m_splitters[row] = 0;
        m_splitters.remove(m_splitters.at(row));
    }
}

bool DockSplitter::isRowEmpty(int row)
{
    if (m_docks[row].count() == 0)
        return true;
    for (uint i = 0; i < m_docks[row].count(); ++i)
        if (m_docks[row][i] != 0)
            return false;
    return true;
}

void DockSplitter::shiftWidgets(QSplitter *splitter, uint row, uint fromCol)
{
    kdDebug() << "shiftWidgets: row=" << row << "  from col=" << fromCol << endl;
    kdDebug() << "row size is: " << m_docks[row].count() << endl;
    
    for (uint i = fromCol; i < m_docks[row].count(); ++i)
    {
        kdDebug() << "move from " << i << " to last" << endl;
        if (m_docks[row][i])
            splitter->moveToLast(m_docks[row][i]);
        else
            kdDebug() << "m_docks[" << row << "][" << i << "] is 0" << endl;
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

QPair<uint, uint> DockSplitter::indexOf(QWidget *dock)
{
    for (uint i = 0; i < m_docks.count(); ++i)
        for (uint j = 0; j < m_docks[i].count(); ++j)
            if (dock == m_docks[i][j])
                return qMakePair(i, j);
    return qMakePair(0u, 0u);
}

}

#include "docksplitter.moc"
