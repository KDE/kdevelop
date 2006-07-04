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
#ifndef IDEALAREA_H
#define IDEALAREA_H

#include "idealdefs.h"

namespace Ideal {

class ToolView;
class MainWindow;

/**
@short Main window area - the profile and layout of tool and code views.

Provides means of saving, restoring and managing the layout of code
and tool views in the main window. It stores and restores positions,
shows and hides toolview buttons, layouts code views.

Area also works as the profile for tool and code views. It determines
whether the given tool or code view is visible at the moment and
*/
class IDEAL_EXPORT Area {
public:
    /**Kind of the area. Each application can define its own areas.*/
    enum Kind { Default = 0, Code = 1, Debug = 2, Design = 4 };

    /**Constructs the area of given @p kind in the @p mainWindow and layouts
    its tool and code views in the mainwindow.*/
    Area(int kind, MainWindow *mainWindow);
    virtual ~Area();

    /** @return the kind of the area.*/
    int kind() const;

    /**Adds the toolview to this area if possible.*/
    virtual void addToolView(ToolView *view);
    /**Permanently removes the toolview from this area if possible.*/
    virtual void removeToolView(ToolView *view);
    /**Shows the toolview and its button in this area if possible.*/
    virtual void showToolView(ToolView *view);
    /**Hides the toolview and its button in this area if possible.*/
    virtual void hideToolView(ToolView *view);

private:
    struct AreaPrivate *d;

};

}

#endif
