/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_SUBLIMEAREAWALKERS_H
#define KDEVPLATFORM_SUBLIMEAREAWALKERS_H

#include <QtCore/QList>


namespace Sublime {

//area walkers implementations

template <typename Operator>
Area::WalkerMode Area::walkViewsInternal(Operator &op, AreaIndex *index)
{
    Area::WalkerMode mode = op(index);
    if (mode == Area::StopWalker)
        return mode;
    else if (index->first() && index->second())
    {
        mode = walkViewsInternal(op, index->first());
        if (mode == Area::StopWalker)
            return mode;
        mode = walkViewsInternal(op, index->second());
    }
    return mode;
}

template <typename Operator>
void Area::walkViews(Operator &op, AreaIndex *index)
{
    walkViewsInternal(op, index);
}

template <typename Operator>
void Area::walkToolViews(Operator &op, Positions positions)
{
    QList<View*> currViews = toolViews();
    foreach (View* view, currViews)
    {
        Sublime::Position position = toolViewPosition(view);
        if (position & positions)
            if (op(view, position) == Area::StopWalker)
                break;
    }
}

}

#endif

