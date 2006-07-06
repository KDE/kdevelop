/***************************************************************************
 *   Copyright (C) 2004-2006 by Alexander Dymo                             *
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
#ifndef IDEALBUTTON_H
#define IDEALBUTTON_H

#include <QPushButton>
#include <QIcon>

#include "idealdefs.h"

class QStyleOptionButton;

namespace Ideal {

/**
@short A button to place onto the ButtonContainer

A QPushButton derivative with a sizehint of a QToolButton. These buttons
can be rotated (rotation depends on a "place" in Ideal mode).
*/
class IDEAL_EXPORT Button : public QPushButton {
    Q_OBJECT
public:
    Button(QWidget *parent, Ideal::Place place, const QString text, const QIcon &icon = QIcon(),
        const QString &description = "");

    /**Sets the description used as a tooltip.*/
    void setDescription(const QString &description);
    /**Returns the description.*/
    QString description() const;

    /**Sets the place of a button.*/
    void setPlace(Ideal::Place place);
    /**Sets the mode of a button.*/
    void setMode(Ideal::ButtonMode mode);

    QSize sizeHint() const;
    QSize sizeHint(const QString &text) const;

    /**Updates size of a widget. Used after squeezing button's text.*/
    void updateSize();

    /**Returns the real (i.e. not squeezed) text of a button.*/
    QString realText() const;

protected:
    ButtonMode mode();

    void paintEvent(QPaintEvent *);

private:
    virtual ~Button();
    void fixDimensions(Place oldPlace);

    QStyleOptionButton styleOption() const;

    void enableIcon();
    void disableIcon();
    void enableText();
    void disableText();

    struct ButtonPrivate *d;

friend class ButtonContainer;
};

}

#endif
