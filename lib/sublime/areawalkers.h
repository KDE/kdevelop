/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef SUBLIMEAREAWALKERS_H
#define SUBLIMEAREAWALKERS_H

namespace Sublime {

template <typename Operator>
void Area::walkViews(Operator &op, AreaIndex *index)
{
    bool result = op(index);
    if (result)
        return;
    else if (index->first() && index->second())
    {
        walkViews(op, index->first());
        walkViews(op, index->second());
    }
}

template <typename Operator>
void Area::walkToolViews(Operator &op, Positions positions)
{
    foreach (View *view, toolViews())
    {
        Sublime::Position position = toolViewPosition(view);
        if (position & positions)
            if (op(view, position))
                break;
    }
}

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
