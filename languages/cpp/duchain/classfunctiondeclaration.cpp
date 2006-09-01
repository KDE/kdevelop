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

#include "classfunctiondeclaration.h"

#include "ducontext.h"

ClassFunctionDeclaration::ClassFunctionDeclaration(KTextEditor::Range * range)
  : ClassMemberDeclaration(range)
  , m_functionType(Normal)
  , m_isVirtual(false)
  , m_isInline(false)
  , m_isExplicit(false)
{
}

/*bool ClassFunctionDeclaration::isSimilar(KDevCodeItem *other, bool strict ) const
{
  if (!CppClassMemberType::isSimilar(other,strict))
    return false;

  FunctionModelItem func = dynamic_cast<ClassFunctionDeclaration*>(other);

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

ClassFunctionDeclaration::QtFunctionType ClassFunctionDeclaration::functionType() const
{
  return m_functionType;
}

void ClassFunctionDeclaration::setFunctionType(QtFunctionType functionType)
{
  m_functionType = functionType;
}

bool ClassFunctionDeclaration::isConstructor() const
{
  if (context()->type() == DUContext::Class && context()->localScopeIdentifier().top() == identifier())
    return true;
  return false;
}

bool ClassFunctionDeclaration::isDestructor() const
{
  QString id = identifier().toString();
  return context()->type() == DUContext::Class && id.startsWith('~') && id.mid(1) == context()->localScopeIdentifier().top().toString();
}

bool ClassFunctionDeclaration::isVirtual() const
{
  return m_isVirtual;
}

void ClassFunctionDeclaration::setVirtual(bool isVirtual)
{
  m_isVirtual = isVirtual;
}

bool ClassFunctionDeclaration::isInline() const
{
  return m_isInline;
}

void ClassFunctionDeclaration::setInline(bool isInline)
{
  m_isInline = isInline;
}

bool ClassFunctionDeclaration::isExplicit() const
{
  return m_isExplicit;
}

void ClassFunctionDeclaration::setExplicit(bool isExplicit)
{
  m_isExplicit = isExplicit;
}

void ClassFunctionDeclaration::setFunctionSpecifiers(FunctionSpecifiers specifiers)
{
  m_isInline = specifiers & InlineSpecifier;
  m_isExplicit = specifiers & ExplicitSpecifier;
  m_isVirtual = specifiers & VirtualSpecifier;
}

// kate: indent-width 2;
