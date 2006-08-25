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

#include "typeinstance.h"

TypeInstance::TypeInstance(KTextEditor::Cursor * definitionPoint)
  : KDevDocumentCursorObject(definitionPoint)
{
}

const Identifier & TypeInstance::identifier() const
{
  return m_identifier;
}

void TypeInstance::setIdentifier(const Identifier & identifier)
{
  m_identifier = identifier;
}

AbstractType * TypeInstance::type() const
{
  return m_type;
}

void TypeInstance::setType(AbstractType * type)
{
  m_type = type;
}
