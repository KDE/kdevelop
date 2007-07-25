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

namespace KDevelop
{
class ClassMemberDeclarationPrivate
{
public:
  Declaration::AccessPolicy m_accessPolicy;
  bool m_isStatic: 1;
  bool m_isAuto: 1;
  bool m_isFriend: 1;
  bool m_isRegister: 1;
  bool m_isExtern: 1;
  bool m_isMutable: 1;
};

ClassMemberDeclaration::ClassMemberDeclaration(const ClassMemberDeclaration& rhs) : Declaration(rhs), d(new ClassMemberDeclarationPrivate) {
  d->m_accessPolicy = rhs.d->m_accessPolicy; 
  d->m_isStatic = rhs.d->m_isStatic;
  d->m_isAuto = rhs.d->m_isAuto;
  d->m_isFriend = rhs.d->m_isFriend;
  d->m_isRegister = rhs.d->m_isRegister;
  d->m_isExtern = rhs.d->m_isExtern;
  d->m_isMutable = rhs.d->m_isMutable;
}

Declaration* ClassMemberDeclaration::clone() const {
  return new ClassMemberDeclaration(*this);
}

ClassMemberDeclaration::ClassMemberDeclaration(KTextEditor::Range * range, DUContext* context)
  : Declaration(range, ClassScope, context)
  , d(new ClassMemberDeclarationPrivate)
{
  d->m_accessPolicy = Declaration::Public;
  d->m_isStatic = false;
  d->m_isAuto = false;
  d->m_isFriend = false;
  d->m_isRegister = false;
  d->m_isExtern = false;
  d->m_isMutable = false;
}

ClassMemberDeclaration::~ClassMemberDeclaration()
{
  delete d;
}

bool ClassMemberDeclaration::isStatic() const
{
  return d->m_isStatic;
}

void ClassMemberDeclaration::setStatic(bool isStatic)
{
  d->m_isStatic = isStatic;
}

bool ClassMemberDeclaration::isAuto() const
{
  return d->m_isAuto;
}

void ClassMemberDeclaration::setAuto(bool isAuto)
{
  d->m_isAuto = isAuto;
}

bool ClassMemberDeclaration::isFriend() const
{
  return d->m_isFriend;
}

void ClassMemberDeclaration::setFriend(bool isFriend)
{
  d->m_isFriend = isFriend;
}

bool ClassMemberDeclaration::isRegister() const
{
  return d->m_isRegister;
}

void ClassMemberDeclaration::setRegister(bool isRegister)
{
  d->m_isRegister = isRegister;
}

bool ClassMemberDeclaration::isExtern() const
{
  return d->m_isExtern;
}

void ClassMemberDeclaration::setExtern(bool isExtern)
{
  d->m_isExtern = isExtern;
}

bool ClassMemberDeclaration::isMutable() const
{
  return d->m_isMutable;
}

void ClassMemberDeclaration::setMutable(bool isMutable)
{
  d->m_isMutable = isMutable;
}

Declaration::AccessPolicy ClassMemberDeclaration::accessPolicy() const
{
  return d->m_accessPolicy;
}

void ClassMemberDeclaration::setAccessPolicy(Declaration::AccessPolicy accessPolicy)
{
  d->m_accessPolicy = accessPolicy;
}

void ClassMemberDeclaration::setStorageSpecifiers(StorageSpecifiers specifiers)
{
  d->m_isStatic = specifiers & StaticSpecifier;
  d->m_isAuto = specifiers & AutoSpecifier;
  d->m_isFriend = specifiers & FriendSpecifier;
  d->m_isRegister = specifiers & RegisterSpecifier;
  d->m_isExtern = specifiers & ExternSpecifier;
  d->m_isMutable = specifiers & MutableSpecifier;
}
}
// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
