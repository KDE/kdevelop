/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo                             *
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
#ifndef PCOMBOBOX_H
#define PCOMBOBOX_H

#include <qmap.h>

#include <qcombobox.h>

#include "propertywidget.h"

class QComboBox;

namespace PropertyLib{

/**
@short %Property editor with combobox.
*/
class PComboBox: public PropertyWidget{
    Q_OBJECT
public:
    /**This constructor is used for read-only selection combo. It provides a value from valueList*/
    PComboBox(MultiProperty *property, const QMap<QString, QVariant> &list, QWidget *parent = 0);
    /**This constructor is used for read-write selection combo. It provides a value from valueList*/
    PComboBox(MultiProperty *property, const QMap<QString, QVariant> &list, bool rw, QWidget *parent = 0);

    /**@return the value currently entered in the editor widget.*/
    virtual QVariant value() const;
    /**Sets the value shown in the editor widget. Set emitChange to false
    if you don't want to emit propertyChanged signal.*/
    virtual void setValue(const QVariant &value, bool emitChange=true);
    /**Sets the list of possible values shown in the editor widget. This method 
    does not emit propertyChanged signal. Reimplemented because combobox is used
    to display possible values from valueList.*/
    virtual void setValueList(const QMap<QString, QVariant> &valueList);

protected:
    QString findDescription(const QVariant &value);

protected slots:
    void updateProperty(int val);

private:
    virtual void fillBox();
    void init(bool rw = false);
    
    /** map<description, value>*/
    QMap<QString, QVariant> m_valueList;

    QComboBox *m_edit;
};

}

#endif
