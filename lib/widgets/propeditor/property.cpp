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
#include <qstring.h>
#include <qwidget.h>

#include "propertywidget.h"
#include "propertyeditor.h"
#include "property.h"

#include "plineedit.h"
#include "pspinbox.h"
#include "plinestyle.h"
#include "pfontcombo.h"
#include "psymbolcombo.h"
#include "pcombobox.h"
#include "pcolorcombo.h"
#include "pstringlistedit.h"
#include "pcheckbox.h"
#include "pdoublenuminput.h"

Property::Property(int type, QString name, QString description, QVariant value, bool save):
    m_type(type), m_name(name), m_description(description), m_value(value), m_save(save)
{
}

Property::Property(QString name, QMap<QString, QString> v_correspList,
    QString description, QVariant value, bool save):
    correspList(v_correspList),
    m_type(ValueFromList), m_name(name), m_description(description), m_value(value),
    m_save(save)
{

}

Property::~Property()
{
}

bool Property::allowSaving()
{
   return m_save;
}

bool Property::operator<(const Property &prop) const
{
    if ((type() < prop.type()) && (name() < prop.name()))
        return true;
    else
        return false;
}

QString Property::name() const
{
    return m_name;
}

void Property::setName(QString name)
{
    m_name = name;
}

int Property::type() const
{
    return m_type;
}

void Property::setType(int type)
{
    m_type = type;
}

QVariant Property::value() const
{
    return m_value;
}

void Property::setValue(QVariant value)
{
    m_value = value;
}

QString Property::description() const
{
    return m_description;
}

void Property::setDescription(QString description)
{
    m_description = description;
}

QWidget *Property::editorOfType(PropertyEditor *editor)
{
//TODO: enable property plugin system
//    QWidget *w=0;
//    editor->createPluggedInEditor(w, editor,this,cb);
//    if (w) return w;

    switch (type())
    {
        case List:
        case Map:
            return new QWidget(0);

        case Integer:
            return new PSpinBox(editor, name(), value(), 0, 10000, 1, 0);

        case Double:
            return new PDoubleNumInput(editor, name(), value(), 0);

        case Boolean:
            return new PCheckBox(editor, name(), value(), 0);

        case StringList:
            return new PStringListEdit(editor, name(), value(), 0);

        case Color:
            return new PColorCombo(editor, name(), value(), 0);

        case FontName:
            return new PFontCombo(editor, name(), value(), 0);

        case Symbol:
            return new PSymbolCombo(editor, name(), value(), 0);

        case LineStyle:
            return new PLineStyle(editor, name(), value(), 0);

        case ValueFromList:
            return new PComboBox(editor, name(), value(), &correspList, false, 0, 0);

        case FieldName:
        case String:
        default:
            return new PLineEdit(editor, name(), value(), 0);
    }
    return 0;
}

void Property::setCorrespList(QMap<QString, QString> list)
{
    correspList = list;
}
