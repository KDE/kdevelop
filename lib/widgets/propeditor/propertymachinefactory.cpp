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

#ifndef PURE_QT
#include <klocale.h>
#else
#include "compat_tools.h"
#endif

#include <qmap.h>

#include "property.h"
#include "childproperty.h"
#include "multiproperty.h"
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
#include "psizeedit.h"
#include "pdateedit.h"
#include "pdatetimeedit.h"
#include "purledit.h"
#include "ppointedit.h"
#include "prectedit.h"
#include "psizepolicyedit.h"
#include "pcolorbutton.h"
#include "pyesnobutton.h"
#include "pfontbutton.h"
#include "ppixmapedit.h"
#include "pcursoredit.h"

namespace PropertyLib{

PropertyMachineFactory *PropertyMachineFactory::m_factory = 0;

PropertyMachineFactory::PropertyMachineFactory()
{
}

PropertyMachineFactory::~PropertyMachineFactory()
{
}

Machine *PropertyMachineFactory::machineForProperty(MultiProperty *property)
{
    int type = property->type();
    QString propertyName = property->name();
    QMap<QString, QVariant> valueList = property->valueList();
    
    if (m_registeredForType.contains(type))
        return (*m_registeredForType[type])();

    switch (type)
    {
        case Property::String:
            return new Machine(new PLineEdit(property));
        case Property::Integer:
            return new Machine(new PSpinBox(property));
        case Property::Double:
            return new Machine(new PDoubleNumInput(property));
        case Property::Boolean:
            return new Machine(new PYesNoButton(property));
        case Property::Date:
            return new Machine(new PDateEdit(property));
        case Property::DateTime:
            return new Machine(new PDateTimeEdit(property));
        case Property::StringList:
            return new Machine(new PStringListEdit(property));
        case Property::Color:
            return new Machine(new PColorButton(property));
        case Property::Font:
            return new Machine(new PFontButton(property));
        case Property::Pixmap:
            return new Machine(new PPixmapEdit(property));

        case Property::ValueFromList:
            return new Machine(new PComboBox(property, valueList));
        case Property::Symbol:
            return new Machine(new PSymbolCombo(property));
        case Property::FontName:
            return new Machine(new PFontCombo(property));
        case Property::FileURL:
            return new Machine(new PUrlEdit(KFile::File, property));
        case Property::DirectoryURL:
            return new Machine(new PUrlEdit(KFile::Directory, property));

        case Property::Size:
        {
            Machine *mach = new Machine(new PSizeEdit(property));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Size_Width, i18n("Width"), i18n("Width")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Size_Height, i18n("Height"), i18n("Height")));
            return mach;
        }
        case Property::Point:
        {
            Machine *mach = new Machine(new PPointEdit(property));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Point_X, i18n("x"), i18n("x")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Point_Y, i18n("y"), i18n("y")));
            return mach;
        }
        case Property::Rect:
        {
            Machine *mach = new Machine(new PRectEdit(property));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Rect_X, i18n("x"), i18n("x")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Rect_Y, i18n("y"), i18n("y")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Rect_Width, i18n("Width"), i18n("Width")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::Rect_Height, i18n("Height"), i18n("Height")));
            return mach;
        }
        case Property::SizePolicy:
        {
            QMap<QString, QVariant> spValues;
            spValues[i18n("Fixed")] = QSizePolicy::Fixed;
            spValues[i18n("Minimum")] = QSizePolicy::Minimum;
            spValues[i18n("Maximum")] = QSizePolicy::Maximum;
            spValues[i18n("Preferred")] = QSizePolicy::Preferred;
            spValues[i18n("Expanding")] = QSizePolicy::Expanding;
            spValues[i18n("Minimum Expanding")] = QSizePolicy::MinimumExpanding;
            spValues[i18n("Ignored")] = QSizePolicy::Ignored;
        
            Machine *mach = new Machine(new PSizePolicyEdit(property, spValues));
            property->details.append(ChildProperty(property, i18n("hSizeType"), ChildProperty::SizePolicy_HorData, spValues, i18n("Horizontal Size Type")));
            property->details.append(ChildProperty(property, i18n("vSizeType"), ChildProperty::SizePolicy_VerData, spValues, i18n("Vertical Size Type")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::SizePolicy_HorStretch, i18n("hStretch"), i18n("Horizontal Stretch")));
            property->details.append(ChildProperty(property, Property::Integer, ChildProperty::SizePolicy_VerStretch, i18n("vStretch"), i18n("Vertical Stretch")));
            return mach;
        }
        case Property::Cursor:
        {
            QMap<QString, QVariant> spValues;
            spValues[i18n("Arrow")] = Qt::ArrowCursor;
            spValues[i18n("Up Arrow")] = Qt::UpArrowCursor;
            spValues[i18n("Cross")] = Qt::CrossCursor;
            spValues[i18n("Waiting")] = Qt::WaitCursor;
            spValues[i18n("iBeam")] = Qt::IbeamCursor;
            spValues[i18n("Size Vertical")] = Qt::SizeVerCursor;
            spValues[i18n("Size Horizontal")] = Qt::SizeHorCursor;
            spValues[i18n("Size Slash")] = Qt::SizeBDiagCursor;
            spValues[i18n("Size Backslash")] = Qt::SizeFDiagCursor;
            spValues[i18n("Size All")] = Qt::SizeAllCursor;
            spValues[i18n("Blank")] = Qt::BlankCursor;
            spValues[i18n("Split Vertical")] = Qt::SplitVCursor;
            spValues[i18n("Split Horizontal")] = Qt::SplitHCursor;
            spValues[i18n("Pointing Hand")] = Qt::PointingHandCursor;
            spValues[i18n("Forbidden")] = Qt::ForbiddenCursor;
            spValues[i18n("What's this")] = Qt::WhatsThisCursor;
            Machine *mach = new Machine(new PCursorEdit(property, spValues));
            return mach;
        }

        case Property::List:
        case Property::Map:
        default:
            return new Machine(new PDummyWidget(property));
    }
}

PropertyMachineFactory *PropertyMachineFactory::getInstance()
{
    if (m_factory == 0)
        m_factory = new PropertyMachineFactory();
    return m_factory;
}

bool PropertyMachineFactory::hasDetailedEditors( int type )
{
    if ( (type==Property::Size) || (type==Property::Point) ||
        (type==Property::Rect) || (type==Property::SizePolicy) )
        return true;
    return 0;
}

}
