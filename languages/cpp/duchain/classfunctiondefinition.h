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

#ifndef CLASSFUNCTIONDEFINITION_H
#define CLASSFUNCTIONDEFINITION_H

#include "classmemberdefinition.h"

/**
 * Represents a single variable definition in a definition-use chain.
 */
class ClassFunctionDefinition : public ClassMemberDefinition
{
public:
  ClassFunctionDefinition(KTextEditor::Range* range);

  enum QtFunctionType
  {
    Normal,
    Signal,
    Slot
  };

  QtFunctionType functionType() const;
  void setFunctionType(QtFunctionType functionType);

  bool isConstructor() const;
  void setConstructor(bool isConstructor);

  bool isDestructor() const;
  void setDestructor(bool isDestructor);

  enum FunctionSpecifier {
    VirtualSpecifier  = 0x1,
    InlineSpecifier   = 0x2,
    ExplicitSpecifier = 0x4
  };
  Q_DECLARE_FLAGS(FunctionSpecifiers, FunctionSpecifier);

  void setFunctionSpecifiers(FunctionSpecifiers specifiers);

  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  bool isInline() const;
  void setInline(bool isInline);

  bool isExplicit() const;
  void setExplicit(bool isExplicit);

  //bool isSimilar(KDevCodeItem *other, bool strict = true) const;

private:
  QtFunctionType m_functionType;
  bool m_constructor: 1;
  bool m_destructor: 1;
  bool m_isVirtual: 1;
  bool m_isInline: 1;
  bool m_isExplicit: 1;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ClassFunctionDefinition::FunctionSpecifiers);

#endif // CLASSFUNCTIONDEFINITION_H

// kate: indent-width 2;
