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

#include <QDockWidget>

#include "idealdefs.h"

namespace Ideal {

/**
@short Ideal Tool View
*/
class IDEAL_EXPORT ToolView: public QDockWidget {
public:
    ToolView(const QString &title, QWidget *parent = 0);
    ToolView(QWidget *parent = 0);

    void setPlace(Ideal::Place place);
    Ideal::Place place() const { return m_place; }
    Qt::DockWidgetArea dockPlace() const;

    void setAreaKind(int areaKind) { m_areaKind = areaKind; }
    int areaKind() const { return m_areaKind; }

    static Qt::DockWidgetArea dockPlace(Ideal::Place place);

private:
    void init();

    int m_areaKind;
    Ideal::Place m_place;
};

}

#endif
