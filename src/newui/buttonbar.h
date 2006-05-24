/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef IDEALBUTTONBAR_H
#define IDEALBUTTONBAR_H

#include <qwidget.h>
#include <qvaluelist.h>

#include "comdefs.h"

#include <qlayout.h>

namespace Ideal {

class Button;
class ButtonBar;

/**@short A layout for a ButtonBar class.

Overrides minimumSize method to allow shrinking button bar buttons.*/
class ButtonLayout: public QBoxLayout{
public:
    ButtonLayout(ButtonBar *parent, Direction d, int margin = 0, int spacing = -1, const char * name = 0);

    virtual QSize minimumSize() const;

private:
    ButtonBar *m_buttonBar;
};

/**
@short A bar with tool buttons.

Looks like a toolbar but has another behaviour. It is suitable for
placing on the left(right, bottom, top) corners of a window as a bar with slider.
*/
class ButtonBar : public QWidget {
    Q_OBJECT
public:
    ButtonBar(Place place, ButtonMode mode = IconsAndText,
        QWidget *parent = 0, const char *name = 0);
    virtual ~ButtonBar();

    /**Adds a button to the bar.*/
    virtual void addButton(Button *button);
    /**Removes a button from the bar and deletes the button.*/
    virtual void removeButton(Button *button);

    /**Sets the mode.*/
    void setMode(ButtonMode mode);
    /**@returns the mode.*/
    ButtonMode mode() const;

    /**@returns the place.*/
    Place place() const;

    bool autoResize() const;
    void setAutoResize(bool b);

    /**Shrinks the button bar so all buttons are visible. Shrinking is done by
    reducing the amount of text shown on buttons. Button icon size is a minimum size
    of a button. If a button does not have an icon, it displays "...".*/
    virtual void shrink(int preferredDimension, int actualDimension);
    virtual void deshrink(int preferredDimension, int actualDimension);
    /**Restores the size of button bar buttons.*/
    virtual void unshrink();

protected:
    virtual void resizeEvent ( QResizeEvent *ev );

    int originalDimension();

private:
    void fixDimensions();
    void setButtonsPlace(Ideal::Place place);

    typedef QValueList<Button*> ButtonList;
    ButtonList m_buttons;

    ButtonMode m_mode;
    Place m_place;

    ButtonLayout *l;

    bool m_shrinked;
    bool m_autoResize;
};

}

#endif
