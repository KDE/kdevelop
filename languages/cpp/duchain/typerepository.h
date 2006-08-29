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
  CppPointerType::Ptr registerType(CppPointerType* input);
  CppReferenceType::Ptr registerType(CppReferenceType* input);

private:
  TypeRepository();
  void newIntegralType(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone);
  CppIntegralType::Ptr getIntegral(int index, int cv) const;

  static TypeRepository* s_instance;

  QVector<CppIntegralType::Ptr> m_integrals;

  QSet<PointerType::Ptr> m_pointers;
  QSet<ReferenceType::Ptr> m_references;
  QSet<FunctionType::Ptr> m_functions;
  QSet<StructureType::Ptr> m_structures;
  QSet<ArrayType::Ptr> m_arrays;
  QSet<AbstractType::Ptr> m_others;
};

#endif // TYPEREPOSITORY_H
