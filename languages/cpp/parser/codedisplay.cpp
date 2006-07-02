/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#include "codemodel.h"

#include <kurl.h>
#include <kicon.h>

CodeDisplay::CodeDisplay()
{}

CodeDisplay::~CodeDisplay()
{}

QString CodeDisplay::display( const _CodeModelItem *item )
{
    switch ( item->kind() )
    {
    case _CodeModelItem::Kind_Function:
    case _CodeModelItem::Kind_FunctionDefinition:
        {
            QStringList signature;
            const _FunctionModelItem *function =
                static_cast<const _FunctionModelItem *>( item );
            signature.append( function->name() );
            signature.append( "(" );

            QStringList args;
            foreach( ArgumentModelItem argument, function->arguments() )
            {
                QString arg = argument->type().toString() + " " + argument->name();
                args.append( arg );
            }
            if ( args.count() )
                signature.append( " " + args.join( ", " ) + " " );

            signature.append( ")" );
            return signature.join( "" );
        }
    case _CodeModelItem::Kind_File:
        return KUrl( item->name() ).fileName();
    default:
        break;
    }
    return item->name();
}

QIcon CodeDisplay::decoration( const _CodeModelItem *item )
{
    switch ( item->kind() )
    {
    case _CodeModelItem::Kind_Scope:
        break;
    case _CodeModelItem::Kind_Namespace:
        return loadIcon( "namespace" );
    case _CodeModelItem::Kind_Member:
        return loadIcon( "field" );
    case _CodeModelItem::Kind_Function:
        return loadIcon( "function" );
    case _CodeModelItem::Kind_Class:
        return loadIcon( "class" );
    case _CodeModelItem::Kind_Enum:
        return loadIcon( "enum" );
    case _CodeModelItem::Kind_FunctionDefinition:
        return loadIcon( "private_function" );
    case _CodeModelItem::Kind_Variable:
        return loadIcon( "field" );
    default:
        break;
    }
    return QIcon();
}

QString CodeDisplay::toolTip( const _CodeModelItem *item )
{
    QString qualified = item->qualifiedName().join( "::" );
    switch ( item->kind() )
    {
    case _CodeModelItem::Kind_Function:
    case _CodeModelItem::Kind_FunctionDefinition:
        {
            QStringList signature;
            const _FunctionModelItem * function =
                static_cast<const _FunctionModelItem *>( item );
            switch ( function->accessPolicy() )
            {
            case CodeModel::Public:
                signature.append( "[public]" );
                break;
            case CodeModel::Protected:
                signature.append( "[protected]" );
                break;
            case CodeModel::Private:
                signature.append( "[private]" );
                break;
            default:
                break;
            }
            switch ( function->functionType() )
            {
            case CodeModel::Signal:
                signature.append( " [signal]" );
                break;
            case CodeModel::Slot:
                signature.append( " [slot]" );
                break;
            default:
                break;
            }
            if ( function->isStatic() )
                signature.append( " static" );
            if ( function->isExtern() )
                signature.append( " extern" );
            if ( function->isVirtual() )
                signature.append( " virtual" );
            if ( function->isInline() )
                signature.append( " inline" );
            if ( function->isExplicit() )
                signature.append( " explicit" );
            if ( function->isAbstract() )
                signature.append( " abstract" );

            signature.append( " " + function->type().toString() +
                              " " + function->display() );

            if ( function->isConstant() )
                signature.append( " const" );

            return signature.join( "" );
        }
    case _CodeModelItem::Kind_Variable:
        break;
    case _CodeModelItem::Kind_Class:
        {
            const _ClassModelItem *_class =
                static_cast<const _ClassModelItem *>( item );
            QString bases = _class->baseClasses().join( ", " );
            return qualified + ( !bases.isEmpty() ? ": " + bases : QString::null );
        }
    default:
        break;
    }
    return qualified;
}

QString CodeDisplay::whatsThis( const _CodeModelItem *item )
{
    return item->name();
}

QIcon CodeDisplay::loadIcon( const QString & name )
{
    return KIcon( name );
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
