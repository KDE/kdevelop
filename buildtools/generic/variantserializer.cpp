/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "variantserializer.h"

void VariantSerializer::storeValue( QDomElement & node, const QVariant & var )
{
    if (var.type() == QVariant::String)
        storeString(node, var);
    else if (var.type() == QVariant::StringList)
        storeStringList(node, var);
    else if (var.type() == QVariant::Int)
        storeInt(node, var);
    else if (var.type() == QVariant::Double)
        storeDouble(node, var);
    else if (var.type() == QVariant::Bool)
        storeBool(node, var);
}

void VariantSerializer::storeString( QDomElement & node, const QVariant & var )
{
    node.setAttribute("type", "String");
    QDomText text = node.ownerDocument().createTextNode(var.toStringList().join(":::"));
    node.appendChild(text);
}

void VariantSerializer::storeStringList( QDomElement & node, const QVariant & var )
{
    node.setAttribute("type", "StringList");
    QDomText text = node.ownerDocument().createTextNode(var.toStringList().join(":::"));
    node.appendChild(text);
}

void VariantSerializer::storeBool( QDomElement & node, const QVariant & var )
{
    node.setAttribute("type", "Bool");
    QDomText text = node.ownerDocument().createTextNode(var.toString());
    node.appendChild(text);
}

void VariantSerializer::storeInt( QDomElement & node, const QVariant & var )
{
    node.setAttribute("type", "Int");
    QDomText text = node.ownerDocument().createTextNode(var.toString());
    node.appendChild(text);
}

void VariantSerializer::storeDouble( QDomElement & node, const QVariant & var )
{
    node.setAttribute("type", "Double");
    QDomText text = node.ownerDocument().createTextNode(var.toString());
    node.appendChild(text);
}

QVariant VariantSerializer::loadString( const QDomText & node )
{
    return QVariant(node.nodeValue());
}

QVariant VariantSerializer::loadStringList( const QDomText & node )
{
    return QVariant(QStringList::split(":::", node.nodeValue()));
}

QVariant VariantSerializer::loadBool( const QDomText & node )
{
    if (node.nodeValue() == "false")
        return QVariant(false);
    else
        return QVariant(true);
}

QVariant VariantSerializer::loadInt( const QDomText & node )
{
    return QVariant(node.nodeValue().toInt());
}

QVariant VariantSerializer::loadDouble( const QDomText & node )
{
    return QVariant(node.nodeValue().toDouble());
}

QVariant VariantSerializer::loadValue( const QDomElement & el )
{
    QString type = el.attribute("type", "QString");
    QDomText val = el.firstChild().toText();
    if (type == "String")
        return loadString(val);
    else if (type == "StringList")
        return loadStringList(val);
    else if (type == "Bool")
        return loadBool(val);
    else if (type == "Int")
        return loadInt(val);
    else if (type == "Double")
        return loadDouble(val);
    return QVariant();
}


