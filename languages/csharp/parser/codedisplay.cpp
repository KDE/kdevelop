/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 * Copyright (c) 2006 Jakob Petsovits <jpetso@gmx.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "codedisplay.h"

#include "csharp_codemodel.h"

#include <kurl.h>
#include <kicon.h>
#include <kstandarddirs.h>

namespace csharp
{

CodeDisplay::CodeDisplay()
{}

CodeDisplay::~CodeDisplay()
{}

QString CodeDisplay::display( const _CodeModelItem *item )
{
    return item->name();
}

QIcon CodeDisplay::decoration( const _CodeModelItem *item )
{
    switch ( item->kind() )
    {
    case _CodeModelItem::Kind_Scope:
        break;
    case _CodeModelItem::Kind_NamespaceDeclaration:
        return loadIcon( "namespace" );
    case _CodeModelItem::Kind_ClassDeclaration:
        return loadIcon( "class" );
    case _CodeModelItem::Kind_StructDeclaration:
        return loadIcon( "struct" );
    case _CodeModelItem::Kind_InterfaceDeclaration:
        return loadIcon( "typedef" );
    case _CodeModelItem::Kind_EnumDeclaration:
        return loadIcon( "enum" );
    case _CodeModelItem::Kind_DelegateDeclaration:
        return loadIcon( "function" );
    case _CodeModelItem::Kind_VariableDeclaration:
        return loadIcon( "field" );
    case _CodeModelItem::Kind_MethodDeclaration:
        return loadIcon( "function" );
    default:
        break;
    }
    return QIcon();
}

QString CodeDisplay::toolTip( const _CodeModelItem *item )
{
    QString qualified = item->qualifiedName().join( "." );
    return qualified;
}

QString CodeDisplay::whatsThis( const _CodeModelItem *item )
{
    return item->name();
}

QIcon CodeDisplay::loadIcon( const QString & name )
{
    return QIcon(KStandardDirs::locate("appdata", QString("pics/%1.png").arg(name)));
}

} // end of namespace csharp

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
