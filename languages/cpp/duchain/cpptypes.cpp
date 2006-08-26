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

bool CppTypeInfo::operator==(const CppTypeInfo& other)
{
  return m_constant == other.m_constant && m_volatile == other.m_volatile;
}

// ---------------------------------------------------------------------------
const QList<CppBaseClassInstance>& CppClassType::baseClasses() const
{
  return m_baseClasses;
}

void CppClassType::addBaseClass(const CppBaseClassInstance& baseClass)
{
  m_baseClasses.append(baseClass);
}

void CppClassType::removeBaseClass(CppClassType* baseClass)
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
/*bool CppClassFunctionType::isSimilar(KDevCodeItem *other, bool strict ) const
{
  if (!CppClassMemberType::isSimilar(other,strict))
    return false;

  FunctionModelItem func = dynamic_cast<CppClassFunctionType*>(other);

  if (isConstant() != func->isConstant())
    return false;

  if (arguments().count() != func->arguments().count())
    return false;

  for (int i=0; i<arguments().count(); ++i)
    {
      ArgumentModelItem arg1 = arguments().at(i);
      ArgumentModelItem arg2 = arguments().at(i);

      if (arg1->type() != arg2->type())
        return false;
    }

  return true;
}*/

CppClassFunctionType::QtFunctionType CppClassFunctionType::functionType() const
{
  return m_functionType;
}

void CppClassFunctionType::setFunctionType(QtFunctionType functionType)
{
  m_functionType = functionType;
}

bool CppClassFunctionType::isConstructor() const
{
  return m_constructor;
}

bool CppClassFunctionType::isDestructor() const
{
  return m_destructor;
}

bool CppClassFunctionType::isVirtual() const
{
  return m_isVirtual;
}

void CppClassFunctionType::setVirtual(bool isVirtual)
{
  m_isVirtual = isVirtual;
}

bool CppClassFunctionType::isInline() const
{
  return m_isInline;
}

void CppClassFunctionType::setInline(bool isInline)
{
  m_isInline = isInline;
}

bool CppClassFunctionType::isExplicit() const
{
  return m_isExplicit;
}

void CppClassFunctionType::setExplicit(bool isExplicit)
{
  m_isExplicit = isExplicit;
}

bool CppClassFunctionType::isAbstract() const
{
  return m_isAbstract;
}

void CppClassFunctionType::setAbstract(bool isAbstract)
{
  m_isAbstract = isAbstract;
}

// ---------------------------------------------------------------------------
AbstractType* CppTypeAliasType::type() const
{
  return m_type;
}

void CppTypeAliasType::setType(AbstractType* type)
{
  m_type = type;
}

// ---------------------------------------------------------------------------
const QList<CppEnumeratorType*>& CppEnumerationType::enumerators() const
{
  return m_enumerators;
}

void CppEnumerationType::addEnumerator(CppEnumeratorType* item)
{
  m_enumerators.append(item);
}

void CppEnumerationType::removeEnumerator(CppEnumeratorType* item)
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

// ---------------------------------------------------------------------------
bool CppClassMemberType::isStatic() const
{
  return m_isStatic;
}

void CppClassMemberType::setStatic(bool isStatic)
{
  m_isStatic = isStatic;
}

bool CppClassMemberType::isAuto() const
{
  return m_isAuto;
}

void CppClassMemberType::setAuto(bool isAuto)
{
  m_isAuto = isAuto;
}

bool CppClassMemberType::isFriend() const
{
  return m_isFriend;
}

void CppClassMemberType::setFriend(bool isFriend)
{
  m_isFriend = isFriend;
}

bool CppClassMemberType::isRegister() const
{
  return m_isRegister;
}

void CppClassMemberType::setRegister(bool isRegister)
{
  m_isRegister = isRegister;
}

bool CppClassMemberType::isExtern() const
{
  return m_isExtern;
}

void CppClassMemberType::setExtern(bool isExtern)
{
  m_isExtern = isExtern;
}

bool CppClassMemberType::isMutable() const
{
  return m_isMutable;
}

void CppClassMemberType::setMutable(bool isMutable)
{
  m_isMutable = isMutable;
}

CppClassMemberType::CppClassMemberType(CppClassType* owner)
  : m_isStatic(0)
  , m_isAuto(0)
  , m_isFriend(0)
  , m_isRegister(0)
  , m_isExtern(0)
  , m_isMutable(0)
{
  Q_UNUSED(owner);
}

CppClassFunctionType::CppClassFunctionType(CppClassType* owner)
  : CppClassMemberType(owner)
  , m_functionType(Normal)
  , m_isVirtual(false)
  , m_isInline(false)
  , m_isAbstract(false)
  , m_isExplicit(false)
{
}

CppTypeAliasType::CppTypeAliasType(DUContext* context)
{
  Q_UNUSED(context);
}

CppEnumeratorType::CppEnumeratorType(CppEnumerationType* enumeration)
{
  Q_UNUSED(enumeration);
}

CppClassType::CppClassType(DUContext* context)
  : m_classType(Class)
{
  Q_UNUSED(context);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
