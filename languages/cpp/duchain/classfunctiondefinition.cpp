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

#include "classfunctiondefinition.h"

// kate: indent-width 2;

ClassFunctionDefinition::ClassFunctionDefinition(KTextEditor::Range * range)
  : ClassMemberDefinition(range)
  , m_functionType(Normal)
  , m_isVirtual(false)
  , m_isInline(false)
  , m_isAbstract(false)
  , m_isExplicit(false)
{
}

/*bool ClassFunctionDefinition::isSimilar(KDevCodeItem *other, bool strict ) const
{
  if (!CppClassMemberType::isSimilar(other,strict))
    return false;

  FunctionModelItem func = dynamic_cast<ClassFunctionDefinition*>(other);

  if (isConstant() != func->isConstant())
    return false;

  if (arguments().count() != func->arguments().count())
    return false;

  for (int i=0; i<arguments().count(); ++i)
    {
      ArgumentModelItem arg1 = arguments().at(i);
      ArgumentModelItem arg2 = arguments().at(i);

      if (arg1->type() != arg2->type())
        return false;
    }

  return true;
}*/

ClassFunctionDefinition::QtFunctionType ClassFunctionDefinition::functionType() const
{
  return m_functionType;
}

void ClassFunctionDefinition::setFunctionType(QtFunctionType functionType)
{
  m_functionType = functionType;
}

bool ClassFunctionDefinition::isConstructor() const
{
  return m_constructor;
}

bool ClassFunctionDefinition::isDestructor() const
{
  return m_destructor;
}

bool ClassFunctionDefinition::isVirtual() const
{
  return m_isVirtual;
}

void ClassFunctionDefinition::setVirtual(bool isVirtual)
{
  m_isVirtual = isVirtual;
}

bool ClassFunctionDefinition::isInline() const
{
  return m_isInline;
}

void ClassFunctionDefinition::setInline(bool isInline)
{
  m_isInline = isInline;
}

bool ClassFunctionDefinition::isExplicit() const
{
  return m_isExplicit;
}

void ClassFunctionDefinition::setExplicit(bool isExplicit)
{
  m_isExplicit = isExplicit;
}

bool ClassFunctionDefinition::isAbstract() const
{
  return m_isAbstract;
}

void ClassFunctionDefinition::setAbstract(bool isAbstract)
{
  m_isAbstract = isAbstract;
}
