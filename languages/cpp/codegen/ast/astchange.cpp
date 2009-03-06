/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#include "astchange.h"

bool BaseSpecifierChange::isVirtual() const
{
  return m_nodeChanges->virt;
}

void BaseSpecifierChange::setVirtual(bool isVirtual)
{
  m_nodeChanges->virt = isVirtual ? 1 : 0;
}

int BaseSpecifierChange::accessSpecifier() const
{
  return m_nodeChanges->access_specifier;
}

void BaseSpecifierChange::setAccessSpecifier(int token)
{
  m_nodeChanges->access_specifier = token;
}
