/* This file is part of KDevelop
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

#include "duchain.h"

#include <ktexteditor/smartrange.h>

#include "typesystem.h"

void Definition::removeUse( KTextEditor::SmartRange * range )
{
  m_uses.removeAll(range);
}

void Definition::addUse( KTextEditor::SmartRange * range )
{
  m_uses.append(range);
}

const QList< KTextEditor::SmartRange * > & Definition::uses( ) const
{
  return m_uses;
}

const QString & Definition::identifier( ) const
{
  return m_identifier;
}

AbstractType * Definition::type( ) const
{
  return m_type;
}

Definition::Scope Definition::scope( ) const
{
  return m_scope;
}

Definition::Definition( AbstractType * type, const QString & identifier, Scope scope )
  : m_scope(scope)
  , m_type(type)
  , m_identifier(identifier)
{
}

// kate: indent-width 2;
