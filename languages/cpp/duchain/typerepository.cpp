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

#include "typerepository.h"

TypeRepository* TypeRepository::s_instance = 0;

TypeRepository::TypeRepository()
{
  m_integrals.reserve(18);

  // void - 0
  newIntegralType(new CppIntegralType(CppIntegralType::TypeVoid));

  // bool - 1
  newIntegralType(new CppIntegralType(CppIntegralType::TypeBool));

  // char - 2
  newIntegralType(new CppIntegralType(CppIntegralType::TypeChar));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeChar, CppIntegralType::ModifierUnsigned));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeChar, CppIntegralType::ModifierSigned));

  // int - 5
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierUnsigned));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierSigned));

  // short int - 8
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierShort));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierShort | CppIntegralType::ModifierUnsigned));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierShort | CppIntegralType::ModifierSigned));

  // long int - 11
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierLong));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierLong | CppIntegralType::ModifierUnsigned));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierLong | CppIntegralType::ModifierSigned));

  // float - 14
  newIntegralType(new CppIntegralType(CppIntegralType::TypeFloat));

  // double - 15
  newIntegralType(new CppIntegralType(CppIntegralType::TypeDouble));
  newIntegralType(new CppIntegralType(CppIntegralType::TypeDouble, CppIntegralType::ModifierLong));

  // wchar_t - 17
  newIntegralType(new CppIntegralType(CppIntegralType::TypeWchar_t));
}

IntegralType::Ptr TypeRepository::integral(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers) const
{
  switch (type) {
    case CppIntegralType::TypeVoid:
      if (!modifiers)
        return m_integrals.at(0);
      break;

    case CppIntegralType::TypeBool:
      if (!modifiers)
        return m_integrals.at(1);
      break;

    case CppIntegralType::TypeChar:
      if (!modifiers)
        return m_integrals.at(2);
      if (modifiers == CppIntegralType::ModifierUnsigned)
        return m_integrals.at(3);
      if (modifiers == CppIntegralType::ModifierSigned)
        return m_integrals.at(4);
      break;

    case CppIntegralType::TypeInt:
      if (!modifiers)
        return m_integrals.at(5);
      if (modifiers == CppIntegralType::ModifierUnsigned)
        return m_integrals.at(6);
      if (modifiers == CppIntegralType::ModifierSigned)
        return m_integrals.at(7);

      if (modifiers == CppIntegralType::ModifierShort)
        return m_integrals.at(8);
      if (modifiers == CppIntegralType::ModifierShort | CppIntegralType::ModifierUnsigned)
        return m_integrals.at(9);
      if (modifiers == CppIntegralType::ModifierShort | CppIntegralType::ModifierSigned)
        return m_integrals.at(10);

      if (!modifiers == CppIntegralType::ModifierLong)
        return m_integrals.at(11);
      if (modifiers == CppIntegralType::ModifierLong | CppIntegralType::ModifierUnsigned)
        return m_integrals.at(12);
      if (modifiers == CppIntegralType::ModifierLong | CppIntegralType::ModifierSigned)
        return m_integrals.at(13);
      break;

    case CppIntegralType::TypeFloat:
      if (!modifiers)
        return m_integrals.at(14);
      break;

    case CppIntegralType::TypeDouble:
      if (!modifiers)
        return m_integrals.at(15);
      if (modifiers == CppIntegralType::ModifierLong)
        return m_integrals.at(16);

    case CppIntegralType::TypeWchar_t:
      if (!modifiers)
        return m_integrals.at(17);
      break;

    default:
      break;
  }

  return IntegralType::Ptr();
}

void TypeRepository::newIntegralType(IntegralType * type)
{
  m_integrals.append(IntegralType::Ptr(type));
}

TypeRepository* TypeRepository::self()
{
  if (!s_instance)
    s_instance = new TypeRepository();

  return s_instance;
}
