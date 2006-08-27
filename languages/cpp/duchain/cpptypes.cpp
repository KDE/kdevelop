/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Adam Treat <treat@kde.org>
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

#include "cpptypes.h"

// ---------------------------------------------------------------------------
const QList<CppClassType::BaseClassInstance>& CppClassType::baseClasses() const
{
  return m_baseClasses;
}

void CppClassType::addBaseClass(const BaseClassInstance& baseClass)
{
  m_baseClasses.append(baseClass);
}

void CppClassType::removeBaseClass(CppClassType::Ptr baseClass)
{
  for (int i = 0; i < m_baseClasses.count(); ++i)
    if (m_baseClasses[i].baseClass == baseClass) {
      m_baseClasses.removeAt(i);
      return;
    }
}

void CppClassType::setClassType(ClassType type)
{
  m_classType = type;
}

CppClassType::ClassType CppClassType::classType() const
{
  return m_classType;
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
AbstractType::Ptr CppTypeAliasType::type() const
{
  return m_type;
}

void CppTypeAliasType::setType(AbstractType::Ptr type)
{
  m_type = type;
}

// ---------------------------------------------------------------------------
const QList<CppEnumeratorType::Ptr>& CppEnumerationType::enumerators() const
{
  return m_enumerators;
}

void CppEnumerationType::addEnumerator(CppEnumeratorType::Ptr item)
{
  m_enumerators.append(item);
}

void CppEnumerationType::removeEnumerator(CppEnumeratorType::Ptr item)
{
  m_enumerators.removeAll(item);
}

// ---------------------------------------------------------------------------
QString CppEnumeratorType::value() const
{
  return m_value;
}

void CppEnumeratorType::setValue(const QString &value)
{
  m_value = value;
}

CppTypeAliasType::CppTypeAliasType()
{
}

CppEnumeratorType::CppEnumeratorType()
{
}

CppClassType::CppClassType()
  : m_classType(Class)
{
}

// kate: space-indent on; indent-width 2; replace-tabs on;

CppIntegralType::CppIntegralType(IntegralTypes type, CppIntegralType::TypeModifiers modifiers)
  : m_type(type)
  , m_modifiers(modifiers)
{
}

CppIntegralType::TypeModifiers CppIntegralType::typeModifiers() const
{
  return m_modifiers;
}

CppIntegralType::IntegralTypes CppIntegralType::integralType() const
{
  return m_type;
}
