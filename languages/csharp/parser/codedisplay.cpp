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
#include "csharp_codemodel_chameleon.h"

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
    QString display = item->name();
    ModelItemChameleon chameleon( item->toItem() );

    switch ( item->kind() )
    {
    case _CodeModelItem::Kind_DelegateDeclaration:
    case _CodeModelItem::Kind_MethodDeclaration:
        {
            bool initial = true;
            display += "(";
            foreach ( ParameterModelItem param, chameleon->parameters().item )
            {
                if ( !initial )
                    display += ", ";
                else
                    initial = false;

                display += param->type()->toString();
            }
            display += ")";
        }
    default:
        break;
    }

    return display;
}

QString CodeDisplay::toolTip( const _CodeModelItem *item )
{
    QString tooltip;
    ModelItemChameleon chameleon( item->toItem() );

    Nullable<access_policy::access_policy_enum> policy =
            chameleon->accessPolicy();

    if ( !policy.isNull )
    {
        bool showAccessPolicy = true;

        if ( MethodDeclarationModelItem modelItem =
                model_dynamic_cast<MethodDeclarationModelItem>(item->toItem()) )
        {
            if ( modelItem->isConstructor() || modelItem->isFinalizer()
                || modelItem->isInterfaceMethodDeclaration() )
            {
                showAccessPolicy = false;
            }
        }

        if ( showAccessPolicy )
        {
            switch ( policy.item )
            {
            case access_policy::access_private:
                tooltip += "[private] "; break;
            case access_policy::access_protected:
                tooltip += "[protected] "; break;
            case access_policy::access_protected_internal:
                tooltip += "[protected internal] "; break;
            case access_policy::access_internal:
                tooltip += "[internal] "; break;
            case access_policy::access_public:
                tooltip += "[public] "; break;
            default:
                Q_ASSERT( false );
            }
        }
    }

    Nullable<bool> is;

    if ( is = chameleon->isStatic(), is.isNull == false && is.item == true )
        tooltip += "static ";
    if ( is = chameleon->isConstant(), is.isNull == false && is.item == true )
        tooltip += "const ";
    if ( is = chameleon->isNew(), is.isNull == false && is.item == true )
        tooltip += "new ";
    if ( is = chameleon->isAbstract(), is.isNull == false && is.item == true )
        tooltip += "abstract ";
    if ( is = chameleon->isSealed(), is.isNull == false && is.item == true )
        tooltip += "sealed ";
    if ( is = chameleon->isReadonly(), is.isNull == false && is.item == true )
        tooltip += "readonly ";
    if ( is = chameleon->isVolatile(), is.isNull == false && is.item == true )
        tooltip += "volatile ";
    if ( is = chameleon->isVirtual(), is.isNull == false && is.item == true )
        tooltip += "virtual ";
    if ( is = chameleon->isOverride(), is.isNull == false && is.item == true )
        tooltip += "override ";
    if ( is = chameleon->isExtern(), is.isNull == false && is.item == true )
        tooltip += "extern ";
    if ( is = chameleon->isUnsafe(), is.isNull == false && is.item == true )
        tooltip += "unsafe ";

    switch ( item->kind() )
    {
    case _CodeModelItem::Kind_NamespaceDeclaration:
        tooltip += "namespace ";
        break;
    case _CodeModelItem::Kind_ClassDeclaration:
        tooltip += "class ";
        break;
    case _CodeModelItem::Kind_StructDeclaration:
        tooltip += "struct ";
        break;
    case _CodeModelItem::Kind_InterfaceDeclaration:
        tooltip += "interface ";
        break;
    case _CodeModelItem::Kind_EnumDeclaration:
        tooltip += "enum ";
        break;
    case _CodeModelItem::Kind_DelegateDeclaration:
    case _CodeModelItem::Kind_MethodDeclaration:
        {
            bool initial = true;

            if ( item->kind() == _CodeModelItem::Kind_DelegateDeclaration )
                tooltip += "delegate ";

            tooltip += chameleon->returnType().item->toString() + " "
                       + item->name() + " (";

            foreach ( ParameterModelItem param, chameleon->parameters().item )
            {
                if ( !initial )
                    tooltip += ", ";
                else
                    initial = false;

                tooltip += param->toString();
            }

            tooltip += ")";
            return tooltip;
        }
    case _CodeModelItem::Kind_VariableDeclaration:
        tooltip += chameleon->type().item->toString() + " ";
        break;
    default:
        break;
    }

    tooltip += item->qualifiedName().join( "." );
    return tooltip;
}

QIcon CodeDisplay::decoration( const _CodeModelItem *item )
{
    switch ( item->kind() )
    {
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
    case _CodeModelItem::Kind_MethodDeclaration:
        return loadIcon( "function" );
    case _CodeModelItem::Kind_VariableDeclaration:
    case _CodeModelItem::Kind_EnumValue:
        return loadIcon( "field" );
    default:
        break;
    }
    return QIcon();
}

QString CodeDisplay::whatsThis( const _CodeModelItem *item )
{
    return toolTip( item );
}

QIcon CodeDisplay::loadIcon( const QString & name )
{
    return QIcon( KStandardDirs::locate("appdata", QString("pics/%1.png").arg(name)) );
}

} // end of namespace csharp

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
