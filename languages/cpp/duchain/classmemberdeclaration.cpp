/* This  is part of KDevelop
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

#include "classmemberdeclaration.h"

ClassMemberDeclaration::ClassMemberDeclaration(KTextEditor::Range * range)
  : Declaration(range, ClassScope)
  , m_accessPolicy(Cpp::Public)
  , m_isStatic(false)
  , m_isAuto(false)
  , m_isFriend(false)
  , m_isRegister(false)
  , m_isExtern(false)
  , m_isMutable(false)
{
}

bool ClassMemberDeclaration::isStatic() const
{
  return m_isStatic;
}

void ClassMemberDeclaration::setStatic(bool isStatic)
{
  m_isStatic = isStatic;
}

bool ClassMemberDeclaration::isAuto() const
{
  return m_isAuto;
}

void ClassMemberDeclaration::setAuto(bool isAuto)
{
  m_isAuto = isAuto;
}

bool ClassMemberDeclaration::isFriend() const
{
  return m_isFriend;
}

void ClassMemberDeclaration::setFriend(bool isFriend)
{
  m_isFriend = isFriend;
}

bool ClassMemberDeclaration::isRegister() const
{
  return m_isRegister;
}

void ClassMemberDeclaration::setRegister(bool isRegister)
{
  m_isRegister = isRegister;
}

bool ClassMemberDeclaration::isExtern() const
{
  return m_isExtern;
}

void ClassMemberDeclaration::setExtern(bool isExtern)
{
  m_isExtern = isExtern;
}

bool ClassMemberDeclaration::isMutable() const
{
  return m_isMutable;
}

void ClassMemberDeclaration::setMutable(bool isMutable)
{
  m_isMutable = isMutable;
}

Cpp::AccessPolicy ClassMemberDeclaration::accessPolicy() const
{
  return m_accessPolicy;
}

void ClassMemberDeclaration::setAccessPolicy(Cpp::AccessPolicy accessPolicy)
{
  m_accessPolicy = accessPolicy;
}

void ClassMemberDeclaration::setStorageSpecifiers(StorageSpecifiers specifiers)
{
  m_isStatic = specifiers & StaticSpecifier;
  m_isAuto = specifiers & AutoSpecifier;
  m_isFriend = specifiers & FriendSpecifier;
  m_isRegister = specifiers & RegisterSpecifier;
  m_isExtern = specifiers & ExternSpecifier;
  m_isMutable = specifiers & MutableSpecifier;
}

// kate: indent-width 2;
