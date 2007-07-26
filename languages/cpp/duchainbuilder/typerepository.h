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

#ifndef TYPEREPOSITORY_H
#define TYPEREPOSITORY_H

#include <QtCore/QSet>
#include <QtCore/QMultiHash>
#include <QtCore/QMutex>

#include "cpptypes.h"
#include "cppduchainbuilderexport.h"

class KDEVCPPDUCHAINBUILDER_EXPORT  TypeRepository
{
public:
  static TypeRepository* self();

  CppIntegralType::Ptr integral(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone, KDevelop::Declaration::CVSpecs cv = KDevelop::Declaration::CVNone) const;

  ///@todo The type-repository holds shared-pointers to types, and may keep AbstractType's alive that come from already deleted du-chains. AbstractType's with no declarations should be cleaned away on a regular basis.

  /**
   * Registers the given \a input type, and returns a pointer to the
   * type, or the previously registered type if it has been encountered
   * before.
   */
  KDevelop::AbstractType::Ptr registerType(KDevelop::AbstractType::Ptr input);

private:
  TypeRepository();

  void newIntegralType(CppIntegralType::IntegralTypes type, CppIntegralType::TypeModifiers modifiers = CppIntegralType::ModifierNone);
  CppIntegralType::Ptr getIntegral(int index, int cv) const;

  KDevelop::AbstractType::Ptr registerPointer(CppPointerType::Ptr input);
  KDevelop::AbstractType::Ptr registerReference(CppReferenceType::Ptr input);
  KDevelop::AbstractType::Ptr registerFunction(CppFunctionType::Ptr input);
  KDevelop::AbstractType::Ptr registerArray(KDevelop::ArrayType::Ptr input);

  static TypeRepository* s_instance;

  // Inbuilt integral types
  QVector<CppIntegralType::Ptr> m_integrals;

  QMultiHash<KDevelop::AbstractType::Ptr, CppPointerType::Ptr> m_pointers;
  QMultiHash<KDevelop::AbstractType::Ptr, CppReferenceType::Ptr> m_references;
  QMultiHash<int, CppFunctionType::Ptr> m_functions;
  //QSet<CppClassType::Ptr> m_structures;
  QMultiHash<KDevelop::AbstractType::Ptr, KDevelop::ArrayType::Ptr> m_arrays;
  //QSet<AbstractType::Ptr> m_others;

  mutable QMutex m_mutex;
};

#endif // TYPEREPOSITORY_H
