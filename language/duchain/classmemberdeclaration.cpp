/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
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

#include "classmemberdeclaration.h"
#include "classmemberdeclaration_p.h"

namespace KDevelop
{

ClassMemberDeclarationPrivate::ClassMemberDeclarationPrivate()
{
  m_accessPolicy = Declaration::Public;
  m_isStatic = false;
  m_isAuto = false;
  m_isFriend = false;
  m_isRegister = false;
  m_isExtern = false;
  m_isMutable = false;
  
}
ClassMemberDeclarationPrivate::ClassMemberDeclarationPrivate( const ClassMemberDeclarationPrivate& rhs ) 
    : DeclarationPrivate( rhs )
{
  m_accessPolicy = rhs.m_accessPolicy;
  m_isStatic = rhs.m_isStatic;
  m_isAuto = rhs.m_isAuto;
  m_isFriend = rhs.m_isFriend;
  m_isRegister = rhs.m_isRegister;
  m_isExtern = rhs.m_isExtern;
  m_isMutable = rhs.m_isMutable;
}
  
ClassMemberDeclaration::ClassMemberDeclaration(const ClassMemberDeclaration& rhs) : Declaration(*new ClassMemberDeclarationPrivate(*rhs.d_func())) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

Declaration* ClassMemberDeclaration::clone() const {
  return new ClassMemberDeclaration(*this);
}

ClassMemberDeclaration::ClassMemberDeclaration(const HashedString& url, const SimpleRange& range, DUContext* context)
  : Declaration(*new ClassMemberDeclarationPrivate,url, range, ClassScope )
{
  if( context )
    setContext( context );
}

ClassMemberDeclaration::ClassMemberDeclaration(ClassMemberDeclarationPrivate& dd, const HashedString& url, const SimpleRange& range, Scope s)
  : Declaration(dd, url, range, s)
{
}

ClassMemberDeclaration::ClassMemberDeclaration(ClassMemberDeclarationPrivate& dd)
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
  d_func()->m_isStatic = isStatic;
}

bool ClassMemberDeclaration::isAuto() const
{
  return d_func()->m_isAuto;
}

void ClassMemberDeclaration::setAuto(bool isAuto)
{
  d_func()->m_isAuto = isAuto;
}

bool ClassMemberDeclaration::isFriend() const
{
  return d_func()->m_isFriend;
}

void ClassMemberDeclaration::setFriend(bool isFriend)
{
  d_func()->m_isFriend = isFriend;
}

bool ClassMemberDeclaration::isRegister() const
{
  return d_func()->m_isRegister;
}

void ClassMemberDeclaration::setRegister(bool isRegister)
{
  d_func()->m_isRegister = isRegister;
}

bool ClassMemberDeclaration::isExtern() const
{
  return d_func()->m_isExtern;
}

void ClassMemberDeclaration::setExtern(bool isExtern)
{
  d_func()->m_isExtern = isExtern;
}

bool ClassMemberDeclaration::isMutable() const
{
  return d_func()->m_isMutable;
}

void ClassMemberDeclaration::setMutable(bool isMutable)
{
  d_func()->m_isMutable = isMutable;
}

Declaration::AccessPolicy ClassMemberDeclaration::accessPolicy() const
{
  return d_func()->m_accessPolicy;
}

void ClassMemberDeclaration::setAccessPolicy(Declaration::AccessPolicy accessPolicy)
{
  d_func()->m_accessPolicy = accessPolicy;
}

void ClassMemberDeclaration::setStorageSpecifiers(StorageSpecifiers specifiers)
{
  Q_D(ClassMemberDeclaration);
  d->m_isStatic = specifiers & StaticSpecifier;
  d->m_isAuto = specifiers & AutoSpecifier;
  d->m_isFriend = specifiers & FriendSpecifier;
  d->m_isRegister = specifiers & RegisterSpecifier;
  d->m_isExtern = specifiers & ExternSpecifier;
  d->m_isMutable = specifiers & MutableSpecifier;
}
}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
