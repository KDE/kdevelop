
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

#include "typerepository.h"

#include <QMutexLocker>

#include <kglobal.h>

#include <duchain/identifier.h>

using namespace KDevelop;

TypeRepository* TypeRepository::self()
{
  K_GLOBAL_STATIC(TypeRepository, s_instance);
  return s_instance;
}

TypeRepository::TypeRepository()
{
  m_integrals.reserve(18 * 4);

  // void - 0
  newIntegralType(TypeVoid);

  // bool - 1
  newIntegralType(TypeBool);

  // char - 2
  newIntegralType(TypeChar);
  newIntegralType(TypeChar, ModifierUnsigned);
  newIntegralType(TypeChar, ModifierSigned);

  // int - 5
  newIntegralType(TypeInt);
  newIntegralType(TypeInt, ModifierUnsigned);
  newIntegralType(TypeInt, ModifierSigned);

  // short int - 8
  newIntegralType(TypeInt, ModifierShort);
  newIntegralType(TypeInt, ModifierShort | ModifierUnsigned);
  newIntegralType(TypeInt, ModifierShort | ModifierSigned);

  // long int - 11
  newIntegralType(TypeInt, ModifierLong);
  newIntegralType(TypeInt, ModifierLong | ModifierUnsigned);
  newIntegralType(TypeInt, ModifierLong | ModifierSigned);

  // float - 14
  newIntegralType(TypeFloat);

  // double - 15
  newIntegralType(TypeDouble);
  newIntegralType(TypeDouble, ModifierLong);

  // wchar_t - 17
  newIntegralType(TypeWchar_t);
}

CppIntegralType::Ptr TypeRepository::integral(IntegralTypes type, TypeModifiers modifiers, Declaration::CVSpecs cv) const
{
  switch (type) {
    case TypeVoid:
      if (!modifiers)
        return getIntegral(0, cv);
      break;

    case TypeBool:
      if (!modifiers)
        return getIntegral(1, cv);
      break;

    case TypeChar:
      if (!modifiers)
        return getIntegral(2, cv);
      if (modifiers == ModifierUnsigned)
        return getIntegral(3, cv);
      if (modifiers == ModifierSigned)
        return getIntegral(4, cv);
      break;

    case TypeInt:
      if (!modifiers)
        return getIntegral(5, cv);
      if (modifiers == ModifierUnsigned)
        return getIntegral(6, cv);
      if (modifiers == ModifierSigned)
        return getIntegral(7, cv);

      if (modifiers == ModifierShort)
        return getIntegral(8, cv);
      if (modifiers == ModifierShort | ModifierUnsigned)
        return getIntegral(9, cv);
      if (modifiers == ModifierShort | ModifierSigned)
        return getIntegral(10, cv);

      if (!modifiers == ModifierLong)
        return getIntegral(11, cv);
      if (modifiers == ModifierLong | ModifierUnsigned)
        return getIntegral(12, cv);
      if (modifiers == ModifierLong | ModifierSigned)
        return getIntegral(13, cv);
      break;

    case TypeFloat:
      if (!modifiers)
        return getIntegral(14, cv);
      break;

    case TypeDouble:
      if (!modifiers)
        return getIntegral(15, cv);
      if (modifiers == ModifierLong)
        return getIntegral(16, cv);

    case TypeWchar_t:
      if (!modifiers)
        return getIntegral(17, cv);
      break;

    default:
      break;
  }

  return CppIntegralType::Ptr();
}

void TypeRepository::newIntegralType(IntegralTypes type, TypeModifiers modifiers)
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

CppIntegralType::Ptr TypeRepository::getIntegral(int index, int cv) const
{
  return m_integrals.at((index * 4) + cv);
}

AbstractType::Ptr TypeRepository::registerType(AbstractType::Ptr input)
{
  if (!input) {
    kWarning(9007) << "Asked to register a null type." ;
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

    case AbstractType::TypeDelayed:
      return registerDelayedType(DelayedType::Ptr::dynamicCast(input));
    default:
      return input;
  }
}

AbstractType::Ptr TypeRepository::registerPointer(CppPointerType::Ptr input)
{
  QMutexLocker lock(&m_mutex);

  Q_ASSERT(input);

  if (!input->baseType())
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  const Declaration::CVSpecs cv = input->cv();

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
  QMutexLocker lock(&m_mutex);

  Q_ASSERT(input);

  if (!input->baseType())
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  const Declaration::CVSpecs cv = input->cv();

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

int hashFromFunction(CppFunctionType* function) {
  int ret = 1;
  const int numElements = function->arguments().count();
  for (int i = 0; i < numElements; ++i)
    ret = (int)((size_t)function->arguments()[i].data()) + ret*13;
  ret = (int)((size_t)function) + ret * 37;
  ret = (int)((size_t)function->cv()) + ret * 17;
  return ret;
  
}

AbstractType::Ptr TypeRepository::registerFunction(CppFunctionType::Ptr input)
{
  QMutexLocker lock(&m_mutex);

  Q_ASSERT(input);

  AbstractType* returnType = input->returnType().data();
  const int numElements = input->arguments().count();
  if (!returnType)
    // Invalid
    return AbstractType::Ptr::staticCast(input);

  foreach (const AbstractType::Ptr& argument, input->arguments())
    if (!argument)
      // Invalid
      return AbstractType::Ptr::staticCast(input);

  int hash = hashFromFunction(input.data());
  
  const Declaration::CVSpecs cv = input->cv();

  QMultiHash<int, CppFunctionType::Ptr>::ConstIterator it = m_functions.constFind(hash);
  if (it != m_functions.constEnd()) {
    for (; it.key() == hash; ++it) {
      if(numElements != (*it)->arguments().count())
        continue;
      if (it.value()->cv() == cv) {
        if (it.value()->returnType() != returnType)
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
  m_functions.insert(hash, input);
  return AbstractType::Ptr::staticCast(input);
}

KDevelop::AbstractType::Ptr TypeRepository::registerDelayedType(KDevelop::DelayedType::Ptr input)
{
  QMutexLocker lock(&m_mutex);
  Q_ASSERT(input);
  QMultiHash<TypeIdentifier, DelayedType::Ptr>::const_iterator it = m_delayedTypes.find(input->identifier());
  for(;it != m_delayedTypes.end(); ++it) {
    if( (*it)->kind() == input->kind() )
      return KDevelop::AbstractType::Ptr::staticCast(*it);
  }

  m_delayedTypes.insert(input->identifier(), input);
  return KDevelop::AbstractType::Ptr::staticCast(input);
}


AbstractType::Ptr TypeRepository::registerArray(ArrayType::Ptr input)
{
  QMutexLocker lock(&m_mutex);

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
