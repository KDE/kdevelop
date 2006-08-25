/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "typesystem.h"

uint qHash (const IntegralType &t)
{
  return qHash (t.name ());
}

uint qHash (const PointerType &t)
{
  return qHash (t.baseType ());
}

uint qHash (const ReferenceType &t)
{
  return qHash (t.baseType ());
}

uint qHash (const FunctionType &t)
{
  QVector<const AbstractType *> arguments (t.arguments ());
  uint hash_value = qHash (t.returnType ());

  for (int i = 0; i < arguments.count (); ++i)
    hash_value = (hash_value << 5) - hash_value + qHash (arguments.at (i));

  return hash_value;
}
