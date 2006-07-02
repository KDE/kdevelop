/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "toolview.h"

namespace Ideal {

struct ToolViewPrivate {
    QWidget *contents;
    Ideal::Place place;
    int area;
};

ToolView::ToolView(QObject *parent, QWidget *contents, Ideal::Place place, int area)
    :QObject(parent)
{
    d = new ToolViewPrivate;
    d->contents = contents;
    d->place = place;
    d->area = area;
}

ToolView::~ToolView()
{
    delete d;
}

Qt::DockWidgetArea ToolView::dockPlace() const
{
    return dockPlace(d->place);
}

Qt::DockWidgetArea ToolView::dockPlace(Ideal::Place place)
{
    Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea;
    if (place == Ideal::Right) dockArea = Qt::RightDockWidgetArea;
    else if (place == Ideal::Bottom) dockArea = Qt::BottomDockWidgetArea;
    else if (place == Ideal::Top) dockArea = Qt::TopDockWidgetArea;
    return dockArea;
}

Ideal::Place ToolView::place() const
{
    return d->place;
}

int Ideal::ToolView::area() const
{
    return d->area;
}

QWidget *ToolView::contents() const
{
    return d->contents;
}

}
