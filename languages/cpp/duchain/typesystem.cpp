/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>
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


#include "typesystem.h"

FunctionType::FunctionType()
  : m_returnType (0)
{
}

void FunctionType::setReturnType(AbstractType::Ptr returnType)
{
  m_returnType = returnType;
}

void FunctionType::addArgument(AbstractType::Ptr argument)
{
  m_arguments.append(argument);
}

void FunctionType::removeArgument(AbstractType::Ptr argument)
{
  m_arguments.removeAll(argument);
}

void StructureType::addElement(AbstractType::Ptr element)
{
  m_elements.append(element);
}

void StructureType::removeElement(AbstractType::Ptr element)
{
  m_elements.removeAll(element);
}

PointerType::PointerType()
  : m_baseType(0)
{
}

ReferenceType::ReferenceType()
  : m_baseType (0)
{
}

IntegralType::IntegralType(const QString & name)
  : m_name(name)
{
}

IntegralType::IntegralType()
{
}
