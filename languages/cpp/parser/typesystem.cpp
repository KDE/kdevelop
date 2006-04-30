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

TypeEnvironment::TypeEnvironment ()
{
}

const QString *TypeEnvironment::intern (const QString &name)
{
  return &*_M_name_table.insert (name);
}

const IntegralType *TypeEnvironment::integralType (const QString *name)
{
  return &*_M_integral_type_table.insert (IntegralType (name));
}

const PointerType *TypeEnvironment::pointerType (const AbstractType *baseType)
{
  return &*_M_pointer_type_table.insert (PointerType (baseType));
}

const ReferenceType *TypeEnvironment::referenceType (const AbstractType *baseType)
{
  return &*_M_reference_type_table.insert (ReferenceType (baseType));
}

const FunctionType *TypeEnvironment::functionType (const AbstractType *returnType,
    const QVector<const AbstractType *> &arguments)
{
  return &*_M_function_type_table.insert (FunctionType (returnType, arguments));
}

const FunctionType *TypeEnvironment::functionType (const AbstractType *returnType)
{
  QVector<const AbstractType *> arguments (0);
  return &*_M_function_type_table.insert (FunctionType (returnType, arguments));
}

const FunctionType *TypeEnvironment::functionType (const AbstractType *returnType, const AbstractType *arg_1)
{
  QVector<const AbstractType *> arguments (1);
  arguments[0] = arg_1;
  return &*_M_function_type_table.insert (FunctionType (returnType, arguments));
}

const FunctionType *TypeEnvironment::functionType (const AbstractType *returnType, const AbstractType *arg_1,
    const AbstractType *arg_2)
{
  QVector<const AbstractType *> arguments (2);
  arguments[0] = arg_1;
  arguments[1] = arg_2;
  return &*_M_function_type_table.insert (FunctionType (returnType, arguments));
}

const FunctionType *TypeEnvironment::functionType (const AbstractType *returnType, const AbstractType *arg_1,
    const AbstractType *arg_2, const AbstractType *arg_3)
{
  QVector<const AbstractType *> arguments (3);
  arguments[0] = arg_1;
  arguments[1] = arg_2;
  arguments[2] = arg_3;
  return &*_M_function_type_table.insert (FunctionType (returnType, arguments));
}

const FunctionType *TypeEnvironment::functionType (const AbstractType *returnType, const AbstractType *arg_1,
    const AbstractType *arg_2, const AbstractType *arg_3,
    const AbstractType *arg_4)
{
  QVector<const AbstractType *> arguments (4);
  arguments[0] = arg_1;
  arguments[1] = arg_2;
  arguments[2] = arg_3;
  arguments[3] = arg_4;
  return &*_M_function_type_table.insert (FunctionType (returnType, arguments));
}
