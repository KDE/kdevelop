/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#include <qvariant.h>

#include <qmap.h>

class QWidget;
class QString;
class PropertyEditor;
class CanvasBox;

/** Integers that represent the type of the property */
enum PropertyType {
    //standard QVariant types
    String = QVariant::String,      //string
    Integer = QVariant::Int,        //integer
    Double = QVariant::Double,      //double
    Boolean = QVariant::Bool,       //boolean
    StringList = QVariant::StringList,    //string list
    Color = QVariant::Color,        //color
    List = QVariant::List,          //QValueList<QVariant>
    Map = QVariant::Map,            //QMap<QString, QVariant>

    //predefined custom types
    ValueFromList = 2000,           //string value from a list
    Symbol = 2001,                  //unicode symbol code
    FontName = 2002,                //font name, e.g. "times new roman"
    FieldName = 2003,               //field name, normally behaves like a String, but can be overridden
    LineStyle = 2004,               //list of line styles

    UserDefined = 3000              //plugin defined properties should start here
};

/**
  Property.
  Contains name, type (PropertyType) and value.
  Value is a QVariant.
  Property has operator < to be able to be included in QMap<>
  (all objects with properties must have
   map<QString, Property* > propertyList
   or similar).
  Those property lists can be intersected as sets of Property by
  set_intersection() - STL algo function.
  The purpose of Property intersection is to find a common properties
  for various elements and then display them in a property editor.
*/
class Property {
public:
    Property() {}
    Property(int type, QString name, QString description="", QVariant value=QVariant(), bool save=true);
    Property(QString name, QMap<QString, QString> v_correspList,
        QString description="", QVariant value=QVariant(), bool save=true);
    virtual ~Property();

    bool operator<(const Property &prop) const;

    QString name() const;
    void setName(QString name);
    int type() const;
    void setType(int type);
    QVariant value() const;
    void setValue(QVariant value);
    QString description() const;
    void setDescription(QString description);

    void setCorrespList(QMap<QString, QString> list);
    QMap<QString, QString> correspList;

    /**Creates and returns the editor for this property*/
    virtual QWidget *editorOfType(PropertyEditor *editor);

    /**Tells if the property can be saved to a stream, xml, etc.
       There is a possibility to use "GUI" properties that aren't
       stored but used only in a GUI*/
    bool allowSaving();
protected:
    int m_type;
    QString m_name;
    QString m_description;
    QVariant m_value;
    bool m_save;
};

/** Master (according to Jeff Alger) pointer to Property */
template<class P>
class MPropPtr{
public:
    MPropPtr()
    {
        m_prop = new P();
    }

    MPropPtr(P *prop): m_prop(prop) {}

    MPropPtr(const MPropPtr<P>& pp): m_prop(new P(*(pp.m_prop))) {}

    MPropPtr<P>& operator=(const MPropPtr<P>& pp)
    {
        if (this != &pp)
        {
            delete m_prop;
            m_prop = new P(*(pp.m_prop));
        }
        return *this;
    }

    ~MPropPtr()
    {
        delete m_prop;
    }

    P *operator->()
    {
        if (m_prop != 0)
            return m_prop;
        else
            return new P();
    }

    P *operator->() const
    {
        if (m_prop != 0)
            return m_prop;
        else
            return new P();
    }

    bool operator<(const MPropPtr<P>& p) const
    {
        if ((prop()->type() < p.prop()->type()) && (prop()->name() < p.prop()->name()))
            return true;
        else
            return false;        
    }

    P *prop() const
    {
        if (m_prop != 0)
            return m_prop;
        else
            return new P();
    }
private:
    P *m_prop;
};

typedef MPropPtr<Property> PropPtr;

#endif
