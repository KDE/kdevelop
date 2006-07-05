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
#ifndef IDEALBUTTONBAR
#define IDEALBUTTONBAR

#include <QMap>
#include <QToolBar>

#include "idealdefs.h"

namespace Ideal {

class ToolViewWidget;
class ButtonContainer;
class Button;
class MainWindow;

/**
@short Toolbar for toolview buttons.

Shows the ButtonContainer widget as the first enrty on the toolbar.
*/
class IDEAL_EXPORT ButtonBar: public QToolBar {
    Q_OBJECT
public:
    ButtonBar(Ideal::Place place, MainWindow *parent = 0);

    /**Adds the button for the toolview.*/
    void addToolViewButton(ToolViewWidget *view);
    /**Shows the button for the toolview.*/
    void showToolViewButton(ToolViewWidget *view);
    /**Hides the button for the toolview.*/
    void hideToolViewButton(ToolViewWidget *view);
    /**Removes the button for the toolview.*/
    void removeToolViewButton(ToolViewWidget *view);

    /** @return the Qt toolbar area code.*/
    Qt::ToolBarArea toolBarPlace();
    /** @return the Qt toolbar area code for given Ideal @p place.*/
    static Qt::ToolBarArea toolBarPlace(Ideal::Place place);

//     MainWindow *mainWindow();

private slots:
    /**Sets the toolview visibility after clicking its button.*/
    void setToolViewWidgetVisibility();

private:
    struct ButtonBarPrivate *d;

};

}

#endif
