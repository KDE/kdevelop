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
#ifndef VARIANTSERIALIZER_H
#define VARIANTSERIALIZER_H

#include <qdom.h>
#include <qvariant.h>
#include <qstringlist.h>

namespace VariantSerializer{

void storeValue(QDomElement &node, const QVariant &var);

void storeString(QDomElement &node, const QVariant &var);
void storeStringList(QDomElement &node, const QVariant &var);
void storeBool(QDomElement &node, const QVariant &var);
void storeInt(QDomElement &node, const QVariant &var);
void storeDouble(QDomElement &node, const QVariant &var);

QVariant loadValue(const QDomElement &el);

QVariant loadString(const QDomText &node);
QVariant loadStringList(const QDomText &node);
QVariant loadBool(const QDomText &node);
QVariant loadInt(const QDomText &node);
QVariant loadDouble(const QDomText &node);

}

#endif
