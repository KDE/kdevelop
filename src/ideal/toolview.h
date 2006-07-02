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
#ifndef IDEALTOOLVIEW_H
#define IDEALTOOLVIEW_H

#include <QObject>

#include "idealdefs.h"

namespace Ideal {

/**
@short Toolview.

Toolview represents a widget and its placement and area options.
*/
class IDEAL_EXPORT ToolView: public QObject {
public:
    /**Creates a toolview with contents @p view in the @p place.
    @p area defines the or-ed list of allowed areas.*/
    ToolView(QObject *parent, QWidget *contents, Ideal::Place place, int area);
    ~ToolView();

    /** @return the place of the toolview.*/
    Ideal::Place place() const;
    /**The or-ed list of allowed toolview areas.*/
    int area() const;
    /**The contents widget of the toolview.*/
    QWidget *contents() const;

    /** @return the Qt dock place for the toolview.*/
    Qt::DockWidgetArea dockPlace() const;
    /** @return the Qt dock place corresponding to the given @p place.*/
    static Qt::DockWidgetArea dockPlace(Ideal::Place place);

private:
    struct ToolViewPrivate *d;

};

}

#endif
