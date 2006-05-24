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
#ifndef IDEALBUTTON_H
#define IDEALBUTTON_H

#include <qpushbutton.h>
#include <qiconset.h>

#include "comdefs.h"

class KAction;

namespace Ideal {

class ButtonBar;

/**
@short A button to place onto the ButtonBar

A QPushButton derivative with a size of a QToolBar. Button can be rotated
(placed onto different places in ideal mode).
*/
class Button : public QPushButton {
    Q_OBJECT
public:
    Button(ButtonBar *parent, const QString text, const QIconSet &icon = QIconSet(),
        const QString &description = QString::null);

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
    void setRealText(const QString &text);

protected:
    ButtonMode mode();

    virtual void drawButton(QPainter *p);
    virtual void drawButtonLabel(QPainter *p);

    virtual void contextMenuEvent(QContextMenuEvent *e);

protected slots:
    void assignAccel();

private:
    virtual ~Button();

    void fixDimensions(Place oldPlace);

    void enableIconSet();
    void disableIconSet();
    void enableText();
    void disableText();

    ButtonBar *m_buttonBar;

    QString m_description;
    Place m_place;

    QString m_realText;
    QIconSet m_realIconSet;

    KAction *m_assignAccelAction;

friend class ButtonBar;
};

}

#endif
