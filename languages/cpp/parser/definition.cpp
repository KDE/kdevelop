/* This  is part of KDevelop
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

#include "definition.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

using namespace KTextEditor;

Definition::Definition( const AbstractType * type, const QString & identifier, Scope scope )
  : m_scope(scope)
  , m_type(type)
  , m_identifier(identifier)
{
}

void Definition::removeUse( SmartRange * range )
{
  m_uses.removeAll(range);
}

void Definition::addUse( SmartRange * range )
{
  m_uses.append(range);
}

const QList< SmartRange * > & Definition::uses( ) const
{
  return m_uses;
}

const QString & Definition::identifier( ) const
{
  return m_identifier;
}

const AbstractType * Definition::type( ) const
{
  return m_type;
}

Definition::Scope Definition::scope( ) const
{
  return m_scope;
}

// kate: indent-width 2;
