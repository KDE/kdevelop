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

class Button;
class ButtonContainer;

/**
@short Toolbar for toolview buttons.

Shows the ButtonContainer widget as the first enrty on the toolbar.
*/
class IDEAL_EXPORT ButtonBar: public QToolBar {
    Q_OBJECT
public:
    ButtonBar(Ideal::Place place, QWidget *parent = 0);
    virtual ~ButtonBar();

    /**Adds the button for the toolview.*/
    virtual void addToolViewButton(Button *button);
    /**Removes the button for the toolview.*/
    virtual void removeToolViewButton(Button *button);

    /** @return the Qt toolbar area code.*/
    Qt::ToolBarArea toolBarPlace();
    /** @return the Qt toolbar area code for given Ideal @p place.*/
    static Qt::ToolBarArea toolBarPlace(Ideal::Place place);

    /** @return true if the button bar is empty.*/
    bool isEmpty() const;

protected:
    /**Factory method to create the button container.
    Reimplement to return ButtonContainer subclasses here.*/
    virtual ButtonContainer *createButtonContainer(Ideal::Place place);

private:
    struct ButtonBarPrivate *d;
    friend class ButtonBarPrivate;

};

}

#endif
