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
@short The container of main window layout.

Provides means of saving, restoring and managing the layout of windows
and toolviews in the main window.
*/
class IDEAL_EXPORT Area {
public:
    /**Kind of the area. Each application can define its own areas.*/
    enum Kind { Code = 1, Debug = 2, Design = 4 };

    Area(int kind, MainWindow *mainWindow);
    virtual ~Area();

    virtual void placeToolView(ToolView *toolView);

private:
    void initArea();

    struct AreaPrivate *d;

};

}

#endif
