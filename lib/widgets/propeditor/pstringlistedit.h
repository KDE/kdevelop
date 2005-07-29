/***************************************************************************
 *   Copyright (C) 2003-2004 by Alexander Dymo                             *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef PSTRINGLISTEDIT_H
#define PSTRINGLISTEDIT_H

#include "propertywidget.h"

class QLineEdit;
class QPushButton;
class QHBoxLayout;

namespace PropertyLib{

/**
@short %Property editor with string list editor.
*/
class PStringListEdit: public PropertyWidget
{
    Q_OBJECT
public:
    PStringListEdit(MultiProperty *property, QWidget *parent = 0, const char *name = 0);

    /**@return the value currently entered in the editor widget.*/
    virtual QVariant value() const;
    /**Sets the value shown in the editor widget. Set emitChange to false
    if you don't want to emit propertyChanged signal.*/
    virtual void setValue(const QVariant &value, bool emitChange=true);
    /**Function to draw a property viewer when the editor isn't shown.*/
    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

private slots:
    void showEditor();

private:
    QLineEdit *edit;
    QPushButton *pbSelect;
    QHBoxLayout *l;

    QStringList m_list;
};

}

#endif
