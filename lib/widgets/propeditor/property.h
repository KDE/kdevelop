/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo  <cloudtemple@mskat.net>    *
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
#ifndef PROPERTY_H
#define PROPERTY_H

#include <qvariant.h>

#include <qmap.h>

class QWidget;
class QString;

namespace PropertyLib{

/** @file property.h
@short Contains @ref Property class and @ref PropertyType enum.
*/

/**
@short Property.

It includes support for QStringList properties, an i18n'ed label and stores an old value to allow undo.

Contains name, type and value.

Type can be one of predefined types (including standard @ref QVariant types) by @ref PropertyType 
enum or custom user type. User defined types should have values more than 3000.

Value is a @ref QVariant.

Property can optionally have a list of possible values.
In that case use @ref ValueFromList type and valueList member.
Use @ref description for i18n'ed label.

Examples:
creating property:
\code 
Property *property = new Property(String, name, description, value)
\endcode
using convenience constructor to create property of ValueFromList type:
\code
Property *property = new Property(name, possibleValuesList, description, value);
\endcode
*/
class Property {
public:
    /** PropertyType.
        Integers that represent the type of the property. */
    enum PropertyType {
        //standard supported QVariant types
        Invalid = QVariant::Invalid        /**<invalid property type*/,
        Map = QVariant::Map                /**<QMap<QString, QVariant>*/,
        List = QVariant::List              /**<QValueList<QVariant>*/,       
        String = QVariant::String          /**<string*/,
        StringList = QVariant::StringList  /**<string list*/,
        Font = QVariant::Font              /**<font*/,
        Pixmap = QVariant::Pixmap          /**<pixmap*/,
        //@todo implement QVariant::Brush
        Rect = QVariant::Rect              /**<rectangle (x,y, width, height)*/,
        Size = QVariant::Size              /**<size (width, height)*/,
        Color = QVariant::Color            /**<color*/,
        //@todo implement QVariant::Palette
        //@todo implement QVariant::ColorGroup
        //@todo implement QVariant::IconSet
        Point = QVariant::Point            /**<point (x,y)*/,
        //@todo implement QVariant::Image
        Integer = QVariant::Int            /**<integer*/,
        //@todo implement QVariant::UInt
        Boolean = QVariant::Bool           /**<boolean*/,
        Double = QVariant::Double          /**<double*/,
        //@todo implement QVariant::CString
        //@todo implement QVariant::PointArray
        //@todo implement QVariant::Region
        //@todo implement QVariant::Bitmap
        Cursor = QVariant::Cursor          /**<cursor*/,
        SizePolicy = QVariant::SizePolicy  /**<size policy (horizontal, vertical)*/,
        Date = QVariant::Date              /**<date*/,
        //@todo implement QVariant::Time
        DateTime = QVariant::DateTime      /**<date and time*/,
        //@todo implement QVariant::ByteArray
        //@todo implement QVariant::BitArray
        //@todo implement QVariant::KeySequence
        //@todo implement QVariant::Pen
        //@todo implement QVariant::Long
        //@todo implement QVariant::LongLong
        //@todo implement QVariant::ULongLong
        

        //predefined custom types
        ValueFromList = 2000               /**<string value from a list*/,
        Symbol = 2001                      /**<unicode symbol code*/,
        FontName = 2002                    /**<font name, e.g. "times new roman"*/,
        FileURL = 2003                     /**<url of a file*/,
        DirectoryURL = 2004                /**<url of a directory*/,

        UserDefined = 3000                 /**<plugin defined properties should start here*/
    };

    /**Constructs empty property.*/
    Property() {}
    /**Constructs property.*/
    Property(int type, const QString &name, const QString &description,
        const QVariant &value = QVariant(), bool save = true, bool readOnly = false);
    /**Constructs property with @ref ValueFromList type.*/
    Property(const QString &name, const QMap<QString, QVariant> &v_valueList,
        const QString &description, const QVariant &value = QVariant(), bool save = true, bool readOnly = false);
    virtual ~Property();

    virtual bool operator<(const Property &prop) const;

    /**@return the name of the property.*/
    virtual QString name() const;
    /**Sets the name of the property.*/
    virtual void setName(const QString &name);
    /**@return the type of the property.*/
    virtual int type() const;
    /**Sets the type of the property.*/
    virtual void setType(int type);
    /**@return the value of the property.*/
    virtual QVariant value() const;
    /**Sets the value of the property.*/
    virtual void setValue(const QVariant &value, bool rememberOldValue = true);
    /**@return the description of the property.*/
    virtual QString description() const;
    /**Sets the description of the property.*/
    virtual void setDescription(const QString &description);
    /**Sets the string-to-value correspondence list of the property.
    This is used to create comboboxes-like property editors.*/
    virtual void setValueList(const QMap<QString, QVariant> &list);
    /**The string-to-value correspondence list of the property.*/
    QMap<QString, QVariant> valueList;

    /**Tells if the property can be saved to a stream, xml, etc.
    There is a possibility to use "GUI" properties that aren't
    stored but used only in a GUI.*/
    virtual bool allowSaving() const;
    /**Tells if the property is read only.*/
    virtual bool readOnly() const;
    /**Tells if the property is visible.*/
    virtual bool visible() const;
    /**Set the visibility.*/
    virtual void setVisible(const bool visible);
    
    /**Gets the previous property value.*/
    virtual QVariant oldValue() const;
    
private:
//    Property(Property &property) {};
//    void operator=(Property &property) {};

    int m_type;
    QString m_name;
    QString m_description;
    QVariant m_value;
    QVariant m_oldValue;
    bool m_save;
    bool m_readOnly;
    bool m_visible;
};

}

#endif
