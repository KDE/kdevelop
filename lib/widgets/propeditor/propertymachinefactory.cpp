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
#include "propertymachinefactory.h"

#include "property.h"
#include "plineedit.h"
#include "pspinbox.h"
#include "pdoublenuminput.h"
#include "pcheckbox.h"
#include "pstringlistedit.h"
#include "pcolorcombo.h"
#include "pdummywidget.h"
#include "pcombobox.h"
#include "psymbolcombo.h"
#include "pfontcombo.h"

PropertyMachineFactory *PropertyMachineFactory::m_factory = 0;

PropertyMachineFactory::PropertyMachineFactory()
{
}

PropertyMachineFactory::~PropertyMachineFactory()
{
}

Machine *PropertyMachineFactory::machineForProperty(const QString propertyName, int type, const QMap<QString, QVariant> &valueList)
{
    if (m_registeredForType.contains(type))
        return (*m_registeredForType[type])();

    switch (type)
    {
        case String:
            return new Machine(new PLineEdit(propertyName));
        case Integer:
            return new Machine(new PSpinBox(propertyName));
        case Double:
            return new Machine(new PDoubleNumInput(propertyName));
        case Boolean:
            return new Machine(new PCheckBox(propertyName));
        case StringList:
            return new Machine(new PStringListEdit(propertyName));
        case Color:
            return new Machine(new PColorCombo(propertyName));

        case ValueFromList:
            return new Machine(new PComboBox(propertyName, valueList));
        case Symbol:
            return new Machine(new PSymbolCombo(propertyName));
        case FontName:
            return new Machine(new PFontCombo(propertyName));
    
        case List:
        case Map:
        default:
            return new Machine(new PDummyWidget(propertyName));
    }
}

PropertyMachineFactory *PropertyMachineFactory::getInstance()
{
    if (m_factory == 0)
        m_factory = new PropertyMachineFactory();
    return m_factory;
}
