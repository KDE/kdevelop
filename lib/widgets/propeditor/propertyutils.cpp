/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
#include "propertyutils.h"

namespace PropertyUtils{

PropPtr createProperty( int type, QString name, QString description, QVariant value, bool save )
{
    return *(new PropPtr( new Property(type, name, description, value, save) ));
}

PropPtr createProperty( QString name, QMap< QString, QString > v_correspList, QString description, QVariant value, bool save )
{
    return *(new PropPtr( new Property(name, v_correspList, description, value, save) ));
}


void variantMapToPropertyList( const QMap< QString, QVariant > & variantMap, QMap< QString, PropPtr > & properties )
{
    for (QMap<QString, QVariant>::const_iterator it = variantMap.begin(); it != variantMap.end(); ++it)
    {
        properties[it.key()] = createProperty(it.data().type(), it.key(), "", it.data(), true);
    }
}

void PropertyUtils::propertyListToVariantMap( const QMap< QString, PropPtr > & properties, QMap< QString, QVariant > & variantMap )
{
    for (QMap<QString, PropPtr>::const_iterator it = properties.begin(); it != properties.end(); ++it)
    {
        if ((*it)->allowSaving())
            variantMap[(*it)->name()] = (*it)->value();
    }
}

}
