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
  m_integrals.reserve(18 * 4);

  // void - 0
  newIntegralType(CppIntegralType::TypeVoid);

  // bool - 1
  newIntegralType(CppIntegralType::TypeBool);

  // char - 2
  newIntegralType(CppIntegralType::TypeChar);
  newIntegralType(CppIntegralType::TypeChar, CppIntegralType::ModifierUnsigned);
  newIntegralType(CppIntegralType::TypeChar, CppIntegralType::ModifierSigned);

  // int - 5
  newIntegralType(CppIntegralType::TypeInt);
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierUnsigned);
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierSigned);

  // short int - 8
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierShort);
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierShort | CppIntegralType::ModifierUnsigned);
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierShort | CppIntegralType::ModifierSigned);

  // long int - 11
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierLong);
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierLong | CppIntegralType::ModifierUnsigned);
  newIntegralType(CppIntegralType::TypeInt, CppIntegralType::ModifierLong | CppIntegralType::ModifierSigned);

  // float - 14
  newIntegralType(CppIntegralType::TypeFloat);

  // double - 15
  newIntegralType(CppIntegralType::TypeDouble);
  newIntegralType(CppIntegralType::TypeDouble, CppIntegralType::ModifierLong);

  // wchar_t - 17
  newIntegralType(CppIntegralType::TypeWchar_t);
}

CppIntegralType::Ptr TypeRepository::integral(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers, Cpp::CVSpecs cv) const
{
  switch (type) {
    case CppIntegralType::TypeVoid:
      if (!modifiers)
        return getIntegral(0, cv);
      break;

    case CppIntegralType::TypeBool:
      if (!modifiers)
        return getIntegral(1, cv);
      break;

    case CppIntegralType::TypeChar:
      if (!modifiers)
        return getIntegral(2, cv);
      if (modifiers == CppIntegralType::ModifierUnsigned)
        return getIntegral(3, cv);
      if (modifiers == CppIntegralType::ModifierSigned)
        return getIntegral(4, cv);
      break;

    case CppIntegralType::TypeInt:
      if (!modifiers)
        return getIntegral(5, cv);
      if (modifiers == CppIntegralType::ModifierUnsigned)
        return getIntegral(6, cv);
      if (modifiers == CppIntegralType::ModifierSigned)
        return getIntegral(7, cv);

      if (modifiers == CppIntegralType::ModifierShort)
        return getIntegral(8, cv);
      if (modifiers == CppIntegralType::ModifierShort | CppIntegralType::ModifierUnsigned)
        return getIntegral(9, cv);
      if (modifiers == CppIntegralType::ModifierShort | CppIntegralType::ModifierSigned)
        return getIntegral(10, cv);

      if (!modifiers == CppIntegralType::ModifierLong)
        return getIntegral(11, cv);
      if (modifiers == CppIntegralType::ModifierLong | CppIntegralType::ModifierUnsigned)
        return getIntegral(12, cv);
      if (modifiers == CppIntegralType::ModifierLong | CppIntegralType::ModifierSigned)
        return getIntegral(13, cv);
      break;

    case CppIntegralType::TypeFloat:
      if (!modifiers)
        return getIntegral(14, cv);
      break;

    case CppIntegralType::TypeDouble:
      if (!modifiers)
        return getIntegral(15, cv);
      if (modifiers == CppIntegralType::ModifierLong)
        return getIntegral(16, cv);

    case CppIntegralType::TypeWchar_t:
      if (!modifiers)
        return getIntegral(17, cv);
      break;

    default:
      break;
  }

  return CppIntegralType::Ptr();
}

void TypeRepository::newIntegralType(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers)
{
  CppIntegralType* plainType = new CppIntegralType(type, modifiers);

  CppIntegralType* constVersion = new CppIntegralType(type, modifiers);
  constVersion->setConstant(true);

  CppIntegralType* volatileVersion = new CppIntegralType(type, modifiers);
  volatileVersion->setVolatile(true);

  CppIntegralType* constVolatileVersion = new CppIntegralType(type, modifiers);
  constVolatileVersion->setVolatile(true);
  constVolatileVersion->setConstant(true);

  m_integrals.append(CppIntegralType::Ptr(plainType));
  m_integrals.append(CppIntegralType::Ptr(constVersion));
  m_integrals.append(CppIntegralType::Ptr(volatileVersion));
  m_integrals.append(CppIntegralType::Ptr(constVolatileVersion));
}

TypeRepository* TypeRepository::self()
{
  if (!s_instance)
    s_instance = new TypeRepository();

  return s_instance;
}

CppIntegralType::Ptr TypeRepository::getIntegral(int index, int cv) const
{
  return m_integrals.at((index * 4) + cv);
}

AbstractType::Ptr TypeRepository::registerType(AbstractType::Ptr input)
{
  if (!input) {
    kWarning() << k_funcinfo << "Asked to register a null type." << endl;
    return input;
  }

  switch (input->whichType()) {
    case AbstractType::TypeAbstract:
      return input;

    case AbstractType::TypeIntegral:
      return input;

    case AbstractType::TypePointer:
      return registerPointer(CppPointerType::Ptr::dynamicCast(input));

    case AbstractType::TypeReference:
      return registerReference(CppReferenceType::Ptr::dynamicCast(input));

    case AbstractType::TypeFunction:
      return registerFunction(CppFunctionType::Ptr::dynamicCast(input));

    case AbstractType::TypeStructure:
      return input;

    case AbstractType::TypeArray:
      return registerArray(ArrayType::Ptr::dynamicCast(input));

    default:
      return input;
  }
}

AbstractType::Ptr TypeRepository::registerPointer(CppPointerType::Ptr input)
{
  Q_ASSERT(input);

  if (!input->baseType())
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  const Cpp::CVSpecs cv = input->cv();

  if (m_pointers.contains(input->baseType())) {
    QMultiHash<AbstractType::Ptr, CppPointerType::Ptr>::ConstIterator it(m_pointers.constFind(input->baseType()));
    if (it != m_pointers.constEnd())
      for (; it.key() == input->baseType(); ++it)
        if (it.value()->cv() == cv)
          // Match
          return AbstractType::Ptr::staticCast(it.value());
  }

  // No match
  m_pointers.insert(input->baseType(), input);
  return AbstractType::Ptr::staticCast(input);
}

AbstractType::Ptr TypeRepository::registerReference(CppReferenceType::Ptr input)
{
  Q_ASSERT(input);

  if (!input->baseType())
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  const Cpp::CVSpecs cv = input->cv();

  if (m_references.contains(input->baseType())) {
    QMultiHash<AbstractType::Ptr, CppReferenceType::Ptr>::ConstIterator it = m_references.constFind(input->baseType());
    if (it != m_references.constEnd())
      for (; it.key() == input->baseType(); ++it)
        if (it.value()->cv() == cv)
          // Match
          return AbstractType::Ptr::staticCast(it.value());
  }

  // No match
  m_references.insert(input->baseType(), input);
  return AbstractType::Ptr::staticCast(input);
}

AbstractType::Ptr TypeRepository::registerFunction(CppFunctionType::Ptr input)
{
  Q_ASSERT(input);

  if (!input->returnType())
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  foreach (const AbstractType::Ptr& argument, input->arguments())
    if (!argument)
      // Invalid
      return AbstractType::Ptr::staticCast(input);

  const int numElements = input->arguments().count();
  const Cpp::CVSpecs cv = input->cv();

  QMultiHash<int, CppFunctionType::Ptr>::ConstIterator it = m_functions.constFind(numElements);
  if (it != m_functions.constEnd()) {
    for (; it.key() == numElements; ++it) {
      if (it.value()->cv() == cv) {
        if (it.value()->returnType() != input->returnType())
          goto nomatch;

        for (int i = 0; i < numElements; ++i)
          if (it.value()->arguments()[i] != input->arguments()[i])
            goto nomatch;

        // Match
        return AbstractType::Ptr::staticCast(it.value());
      }
      nomatch:
      continue;
    }
  }

  // No match
  m_functions.insert(input->arguments().count(), input);
  return AbstractType::Ptr::staticCast(input);
}

AbstractType::Ptr TypeRepository::registerArray(ArrayType::Ptr input)
{
  Q_ASSERT(input);

  if (!input->elementType())
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  if (m_arrays.contains(input->elementType())) {
    QMultiHash<AbstractType::Ptr, ArrayType::Ptr>::ConstIterator it = m_arrays.constFind(input->elementType());
    if (it != m_arrays.constEnd())
      for (; it.key() == input->elementType(); ++it)
        // Match
        return AbstractType::Ptr::staticCast(it.value());
  }

  // No match
  m_arrays.insert(input->elementType(), input);
  return AbstractType::Ptr::staticCast(input);
}
