/* This file is part of KDevelop
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

#ifndef CLASSMEMBERDECLARATION_H
#define CLASSMEMBERDECLARATION_H

#include "declaration.h"

/**
 * Represents a single class member definition in a definition-use chain.
 */
class ClassMemberDeclaration : public Declaration
{
public:
  ClassMemberDeclaration(KTextEditor::Range* range, DUContext* context);

  Cpp::AccessPolicy accessPolicy() const;
  void setAccessPolicy(Cpp::AccessPolicy accessPolicy);

  enum StorageSpecifier {
    StaticSpecifier   = 0x1,
    AutoSpecifier     = 0x2,
    FriendSpecifier   = 0x4,
    ExternSpecifier   = 0x8,
    RegisterSpecifier = 0x10,
    MutableSpecifier  = 0x20
  };
  Q_DECLARE_FLAGS(StorageSpecifiers, StorageSpecifier);

  void setStorageSpecifiers(StorageSpecifiers specifiers);

  bool isStatic() const;
  void setStatic(bool isStatic);

  bool isAuto() const;
  void setAuto(bool isAuto);

  bool isFriend() const;
  void setFriend(bool isFriend);

  bool isRegister() const;
  void setRegister(bool isRegister);

  bool isExtern() const;
  void setExtern(bool isExtern);

  bool isMutable() const;
  void setMutable(bool isMutable);

private:
  Cpp::AccessPolicy m_accessPolicy;
  bool m_isStatic: 1;
  bool m_isAuto: 1;
  bool m_isFriend: 1;
  bool m_isRegister: 1;
  bool m_isExtern: 1;
  bool m_isMutable: 1;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ClassMemberDeclaration::StorageSpecifiers);

#endif // CLASSMEMBERDECLARATION_H

// kate: indent-width 2;
