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

const QString ContextMenuExtension::FileGroup     = "FileGroup";
const QString ContextMenuExtension::RefactorGroup = "RefactorGroup";
const QString ContextMenuExtension::BuildGroup    = "BuildGroup";
const QString ContextMenuExtension::RunGroup      = "RunGroup";
const QString ContextMenuExtension::DebugGroup    = "DebugGroup";
const QString ContextMenuExtension::EditGroup     = "EditGroup";
const QString ContextMenuExtension::VcsGroup      = "VcsGroup";
const QString ContextMenuExtension::ProjectGroup  = "ProjectGroup";
const QString ContextMenuExtension::ExtensionGroup  = "ExtensionGroup";


class ContextMenuExtensionPrivate
{
public:
    QMap<QString,QList<QAction*> > extensions;
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
    d->extensions = rhs.d->extensions;
}

ContextMenuExtension& ContextMenuExtension::operator=( const ContextMenuExtension& rhs )
{
    if( this == &rhs )
        return *this;

    d->extensions = rhs.d->extensions;
    return *this;
}

QList<QAction*> ContextMenuExtension::actions( const QString& group ) const
{
    if( d->extensions.contains( group ) )
        return d->extensions.value( group );
    else
        return QList<QAction*>();
}

void ContextMenuExtension::addAction( const QString& group, QAction* action )
{
    if( !d->extensions.contains( group ) )
    {
        d->extensions.insert( group, QList<QAction*>() << action );
    } else
    {
        d->extensions[group].append( action );
    }
}

}
