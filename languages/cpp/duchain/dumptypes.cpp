/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "dumptypes.h"

DumpTypes::DumpTypes()
  : indent(0)
{
}

DumpTypes::~ DumpTypes()
{
}

void DumpTypes::dump(const AbstractType * type)
{
  type->accept(this);
}

bool DumpTypes::preVisit(const AbstractType * type)
{
  ++indent;
  kDebug() << QString(indent*2, ' ') << type->toString() << endl;
  return true;
}

void DumpTypes::postVisit(const AbstractType * type)
{
  --indent;
}

void DumpTypes::visit(const IntegralType * type)
{
}

bool DumpTypes::visit(const PointerType * type)
{
  return true;
}

void DumpTypes::endVisit(const PointerType * type)
{
}

bool DumpTypes::visit(const ReferenceType * type)
{
  return true;
}

void DumpTypes::endVisit(const ReferenceType * type)
{
}

bool DumpTypes::visit(const FunctionType * type)
{
  return true;
}

void DumpTypes::endVisit(const FunctionType * type)
{
}

bool DumpTypes::visit(const StructureType * type)
{
  return true;
}

void DumpTypes::endVisit(const StructureType * type)
{
}

bool DumpTypes::visit(const ArrayType * type)
{
  return true;
}

void DumpTypes::endVisit(const ArrayType * type)
{
}

// kate: space-indent on; indent-width 2; replace-tabs on;
