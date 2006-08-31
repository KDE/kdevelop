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

#ifndef TYPEREPOSITORY_H
#define TYPEREPOSITORY_H

#include "cpptypes.h"

#include <QSet>
#include <QMultiHash>

class QMutex;

class TypeRepository
{
public:
  static TypeRepository* self();

  CppIntegralType::Ptr integral(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone, Cpp::CVSpecs cv = Cpp::CVNone) const;

  /**
   * Registers the given \a input type, and returns a pointer to the
   * type, or the previously registered type if it has been encountered
   * before.
   */
  AbstractType::Ptr registerType(AbstractType::Ptr input);

private:
  TypeRepository();
  void newIntegralType(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone);
  CppIntegralType::Ptr getIntegral(int index, int cv) const;

  AbstractType::Ptr registerPointer(CppPointerType::Ptr input);
  AbstractType::Ptr registerReference(CppReferenceType::Ptr input);
  AbstractType::Ptr registerFunction(CppFunctionType::Ptr input);
  AbstractType::Ptr registerArray(ArrayType::Ptr input);

  static TypeRepository* s_instance;

  // Inbuilt integral types
  QVector<CppIntegralType::Ptr> m_integrals;

  QMultiHash<AbstractType::Ptr, CppPointerType::Ptr> m_pointers;
  QMultiHash<AbstractType::Ptr, CppReferenceType::Ptr> m_references;
  QMultiHash<int, CppFunctionType::Ptr> m_functions;
  //QSet<CppClassType::Ptr> m_structures;
  QMultiHash<AbstractType::Ptr, ArrayType::Ptr> m_arrays;
  //QSet<AbstractType::Ptr> m_others;

  QMutex* m_mutex;
};

#endif // TYPEREPOSITORY_H
