/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mskat.net                                                 *
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
#ifndef PCOLORBUTTON_H
#define PCOLORBUTTON_H

#include "propertywidget.h"

#ifndef PURE_QT
class KColorButton;
#else
class QPushButton;
#include <qcolor.h>
#endif
namespace PropertyLib{

/**
@short %Property editor with color chooser button.
*/
class PColorButton: public PropertyWidget
{
    Q_OBJECT
public:
    PColorButton(MultiProperty *property, QWidget *parent = 0, const char *name = 0);

    virtual QVariant value() const;
    virtual void drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value);
    virtual void setValue(const QVariant& value, bool emitChange);
    
protected slots:
    void updateProperty(const QColor& color);

    void changeColor();

private:
#ifndef PURE_QT
    KColorButton *m_edit;
#else
    QPushButton *m_edit;
    QColor m_color;
#endif
};

}

#endif
