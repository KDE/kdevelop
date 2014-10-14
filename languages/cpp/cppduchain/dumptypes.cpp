/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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
#include "debug.h"

#include <language/duchain/types/alltypes.h>

using namespace KDevelop;

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
  m_encountered.clear();
}

bool DumpTypes::preVisit(const AbstractType * type)
{
  ++indent;
  qCDebug(CPPDUCHAIN) << QString(indent*2, ' ') << type->toString();
  return true;
}

void DumpTypes::postVisit(const AbstractType *)
{
  --indent;
}

void DumpTypes::visit(const IntegralType *)
{
}

bool DumpTypes::visit (const KDevelop::AbstractType *type)
{
  return !seen(type);
}

bool DumpTypes::visit(const PointerType * type)
{
  return !seen(type);
}

void DumpTypes::endVisit(const PointerType *)
{
}

bool DumpTypes::visit(const ReferenceType * type)
{
  return !seen(type);
}

void DumpTypes::endVisit(const ReferenceType *)
{
}

bool DumpTypes::visit(const FunctionType * type)
{
  return !seen(type);
}

void DumpTypes::endVisit(const FunctionType *)
{
}

bool DumpTypes::visit(const StructureType * type)
{
  return !seen(type);
}

void DumpTypes::endVisit(const StructureType *)
{
}

bool DumpTypes::visit(const ArrayType * type)
{
  return !seen(type);
}

void DumpTypes::endVisit(const ArrayType *)
{
}


bool DumpTypes::seen(const AbstractType * type)
{
  if (m_encountered.contains(type))
    return true;

  m_encountered.insert(type);
  return false;
}
