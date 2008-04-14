/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "contextmenuextension.h"

#include <QtCore/QList>
#include <QtCore/QMap>

#include <kaction.h>

namespace KDevelop
{

class ContextMenuExtensionPrivate
{
public:
    QMap<ContextMenuExtension::Group,QList<KAction*> > extensions;
};

ContextMenuExtension::ContextMenuExtension()
    : d(new ContextMenuExtensionPrivate)
{
}

ContextMenuExtension::~ContextMenuExtension()
{
}


ContextMenuExtension::ContextMenuExtension( const ContextMenuExtension& rhs )
    : d( new ContextMenuExtensionPrivate )
{
    Q_FOREACH( ContextMenuExtension::Group group, rhs.d->extensions.keys() )
    {
        d->extensions[group] = rhs.d->extensions[group];
    }
}

ContextMenuExtension& ContextMenuExtension::operator=( const ContextMenuExtension& rhs )
{
    if( this == &rhs )
        return *this;

    Q_FOREACH( ContextMenuExtension::Group group, rhs.d->extensions.keys() )
    {
        d->extensions[group] = rhs.d->extensions[group];
    }
    return *this;
}

QList<KAction*> ContextMenuExtension::actions( ContextMenuExtension::Group group )
{
    if( d->extensions.contains( group ) )
        return d->extensions.value( group );
    else
        return QList<KAction*>();
}

void ContextMenuExtension::addAction( ContextMenuExtension::Group group, KAction* action )
{
    if( d->extensions.contains( group ) )
    {
        d->extensions.insert( group, QList<KAction*>() << action );
    } else
    {
        d->extensions[group].append( action );
    }
}

}
