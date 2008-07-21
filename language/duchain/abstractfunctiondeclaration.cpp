/* This file is part of KDevelop
    Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#include "abstractfunctiondeclaration.h"
#include <QStringList>

namespace KDevelop {

AbstractFunctionDeclaration::~AbstractFunctionDeclaration() {
}

bool AbstractFunctionDeclaration::isVirtual() const
{
  return data()->m_isVirtual;
}

void AbstractFunctionDeclaration::setVirtual(bool isVirtual)
{
  dynamicData()->m_isVirtual = isVirtual;
}

bool AbstractFunctionDeclaration::isInline() const
{
  return data()->m_isInline;
}

void AbstractFunctionDeclaration::setInline(bool isInline)
{
  dynamicData()->m_isInline = isInline;
}

bool AbstractFunctionDeclaration::isExplicit() const
{
  return data()->m_isExplicit;
}

void AbstractFunctionDeclaration::setExplicit(bool isExplicit)
{
  dynamicData()->m_isExplicit = isExplicit;
}

void AbstractFunctionDeclaration::setFunctionSpecifiers(FunctionSpecifiers specifiers)
{
  dynamicData()->m_isInline = specifiers & InlineSpecifier;
  dynamicData()->m_isExplicit = specifiers & ExplicitSpecifier;
  dynamicData()->m_isVirtual = specifiers & VirtualSpecifier;
}

}


// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
