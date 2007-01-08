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
#include "areaprinter.h"

#include <sublime/view.h>
#include <sublime/areaindex.h>

using namespace Sublime;

// class AreaViewsPrinter

AreaViewsPrinter::AreaViewsPrinter()
{
    result = "\n";
}

Area::WalkerMode AreaViewsPrinter::operator()(Sublime::AreaIndex *index)
{
    result += printIndentation(index) + "[ ";
    if (index->views().isEmpty())
        result += printOrientation(index->orientation());
    else
    {
        foreach (View *view, index->views())
            result += view->objectName() + " ";
    }
    result += "]\n";
    return Area::ContinueWalker;
}

QString AreaViewsPrinter::printIndentation(Sublime::AreaIndex *index) const
{
    QString i = "";
    while (index = index->parent())
        i += "    ";
    return i;
}

QString AreaViewsPrinter::printOrientation(Qt::Orientation o) const
{
    if (o == Qt::Vertical)
        return "vertical splitter";
    else
        return "horizontal splitter";
}



// class AreaToolViewsPrinter

AreaToolViewsPrinter::AreaToolViewsPrinter()
{
    result = "\n";
}

Area::WalkerMode AreaToolViewsPrinter::operator()(Sublime::View *view, Sublime::Position position)
{
    result += view->objectName() + " [ " + printPosition(position) + " ]" + "\n";
    return Area::ContinueWalker;
}

QString AreaToolViewsPrinter::printPosition(Sublime::Position position)
{
    switch (position)
    {
        case Sublime::Left: return "left";
        case Sublime::Right: return "right";
        case Sublime::Bottom: return "bottom";
        case Sublime::Top: return "top";
        default: return "wrong position";
    }
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
