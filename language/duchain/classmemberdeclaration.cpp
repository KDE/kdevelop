/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "classmemberdeclaration.h"
#include "classmemberdeclarationdata.h"
#include "duchainregister.h"
#include <editor/rangeinrevision.h>

namespace KDevelop
{

ClassMemberDeclarationData::ClassMemberDeclarationData()
{
  m_accessPolicy = Declaration::Public;
  m_isStatic = false;
  m_isAuto = false;
  m_isFriend = false;
  m_isRegister = false;
  m_isExtern = false;
  m_isMutable = false;
  m_isNative = false;
  m_isSynchronized = false;
  m_isStrictFP = false;
  m_isAbstract = false;
}
ClassMemberDeclarationData::ClassMemberDeclarationData( const ClassMemberDeclarationData& rhs )
    : DeclarationData( rhs )
{
  m_accessPolicy = rhs.m_accessPolicy;
  m_isStatic = rhs.m_isStatic;
  m_isAuto = rhs.m_isAuto;
  m_isFriend = rhs.m_isFriend;
  m_isRegister = rhs.m_isRegister;
  m_isExtern = rhs.m_isExtern;
  m_isMutable = rhs.m_isMutable;
  m_isNative = rhs.m_isNative;
  m_isSynchronized = rhs.m_isSynchronized;
  m_isStrictFP = rhs.m_isStrictFP;
  m_isAbstract = rhs.m_isAbstract;
}

ClassMemberDeclaration::ClassMemberDeclaration(const ClassMemberDeclaration& rhs) : Declaration(*new ClassMemberDeclarationData(*rhs.d_func())) {
}

REGISTER_DUCHAIN_ITEM(ClassMemberDeclaration);

Declaration* ClassMemberDeclaration::clonePrivate() const {
  return new ClassMemberDeclaration(*this);
}

ClassMemberDeclaration::ClassMemberDeclaration(const RangeInRevision& range, DUContext* context)
  : Declaration(*new ClassMemberDeclarationData, range )
{
  d_func_dynamic()->setClassId(this);
  if( context )
    setContext( context );
}

ClassMemberDeclaration::ClassMemberDeclaration(ClassMemberDeclarationData& dd, const RangeInRevision& range )
  : Declaration(dd, range)
{
}

ClassMemberDeclaration::ClassMemberDeclaration(ClassMemberDeclarationData& dd)
  : Declaration(dd)
{
}

ClassMemberDeclaration::~ClassMemberDeclaration()
{
}

bool ClassMemberDeclaration::isStatic() const
{
  return d_func()->m_isStatic;
}

void ClassMemberDeclaration::setStatic(bool isStatic)
{
  d_func_dynamic()->m_isStatic = isStatic;
}

bool ClassMemberDeclaration::isAuto() const
{
  return d_func()->m_isAuto;
}

void ClassMemberDeclaration::setAuto(bool isAuto)
{
  d_func_dynamic()->m_isAuto = isAuto;
}

bool ClassMemberDeclaration::isFriend() const
{
  return d_func()->m_isFriend;
}

void ClassMemberDeclaration::setFriend(bool isFriend)
{
  d_func_dynamic()->m_isFriend = isFriend;
}

bool ClassMemberDeclaration::isRegister() const
{
  return d_func()->m_isRegister;
}

void ClassMemberDeclaration::setRegister(bool isRegister)
{
  d_func_dynamic()->m_isRegister = isRegister;
}

bool ClassMemberDeclaration::isExtern() const
{
  return d_func()->m_isExtern;
}

void ClassMemberDeclaration::setExtern(bool isExtern)
{
  d_func_dynamic()->m_isExtern = isExtern;
}

bool ClassMemberDeclaration::isMutable() const
{
  return d_func()->m_isMutable;
}

void ClassMemberDeclaration::setMutable(bool isMutable)
{
  d_func_dynamic()->m_isMutable = isMutable;
}

Declaration::AccessPolicy ClassMemberDeclaration::accessPolicy() const
{
  return d_func()->m_accessPolicy;
}

void ClassMemberDeclaration::setAccessPolicy(Declaration::AccessPolicy accessPolicy)
{
  d_func_dynamic()->m_accessPolicy = accessPolicy;
}

bool ClassMemberDeclaration::isNative() const
{
  return d_func()->m_isNative;
}

void ClassMemberDeclaration::setNative(bool native)
{
  d_func_dynamic()->m_isNative = native;
}

bool ClassMemberDeclaration::isStrictFP() const
{
  return d_func()->m_isStrictFP;
}

void ClassMemberDeclaration::setStrictFP(bool strictFP)
{
  d_func_dynamic()->m_isStrictFP = strictFP;
}

bool ClassMemberDeclaration::isSynchronized() const
{
  return d_func()->m_isSynchronized;
}

void ClassMemberDeclaration::setSynchronized(bool synchronized)
{
  d_func_dynamic()->m_isSynchronized = synchronized;
}

bool ClassMemberDeclaration::isAbstract() const
{
  return d_func()->m_isAbstract;
}

void ClassMemberDeclaration::setAbstract(bool abstract)
{
  d_func_dynamic()->m_isAbstract = abstract;
}


void ClassMemberDeclaration::setStorageSpecifiers(StorageSpecifiers specifiers)
{
  DUCHAIN_D_DYNAMIC(ClassMemberDeclaration);
  d->m_isStatic = specifiers & StaticSpecifier;
  d->m_isAuto = specifiers & AutoSpecifier;
  d->m_isFriend = specifiers & FriendSpecifier;
  d->m_isRegister = specifiers & RegisterSpecifier;
  d->m_isExtern = specifiers & ExternSpecifier;
  d->m_isMutable = specifiers & MutableSpecifier;
  d->m_isFinal = specifiers & FinalSpecifier;
  d->m_isSynchronized = specifiers & SynchronizedSpecifier;
  d->m_isNative = specifiers & NativeSpecifier;
  d->m_isStrictFP = specifiers & StrictFPSpecifier;
  d->m_isAbstract = specifiers & AbstractSpecifier;
}
}
