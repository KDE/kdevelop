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
#ifndef IDEALMAINWINDOW_H
#define IDEALMAINWINDOW_H

#include <QList>
#include <QMainWindow>

#include "idealdefs.h"

namespace Ideal {

class ButtonBar;
class ToolView;
class Area;

/**
@short Main Window for the Ideal UI.
*/
class IDEAL_EXPORT MainWindow: public QMainWindow {
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /**Sets the current area for the main window and uses area's configuration
    to place tool and code views and fill button bars.
    See @ref Area class documentation for more information about areas.*/
    virtual void setArea(int area);
    /** @return the current area or 0 if no area was set.*/
    Area *area() const;

    /**Adds the toolview @p view to the main window. The toolview button will not be shown
    until the area is set for the mainwindow using @ref setArea method. The toolview
    itself will be shown as late as possible.
    @param defaultPlace defines where in the window the toolview will be placed (note that
    this parameter is only a hint to mainwindow areas to place the view for the first time).
    @param area is or-ed list of allowed areas for this toolview.*/
    virtual void addToolView(QWidget *view, Ideal::Place defaultPlace, int area);
    /**Removes permanently the toolview from the main window. The current area is
    asked to remove it from the window layout also. To set just the visibility
    of the toolview use @ref hideToolView and @ref showToolView.*/
    virtual void removeToolView(QWidget *view);

    /** @return the list of all available toolviews in the mainwindow including
    hidden views and views not available in the current area.*/
    QList<ToolView*> toolViews() const;
    /** @return the button bar for given @p place.*/
    ButtonBar *buttonBar(Ideal::Place place);

protected:
    /**Factory method to create the toolview. Reimplement this to return
    ToolView subclasses here.*/
    virtual ToolView *createToolView(QWidget *view, Ideal::Place defaultPlace, int area);
    /**Factory method to create the button bar for toolview buttons.
    Reimplement this to return ButtonBar subclasses here.*/
    virtual ButtonBar *createButtonBar(Ideal::Place place);

private:
    struct MainWindowPrivate *d;
    friend class MainWindowPrivate;

};

}

#endif
