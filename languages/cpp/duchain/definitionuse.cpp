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

#include "definitionuse.h"

#include "definition.h"

using namespace KTextEditor;

DefinitionUse::DefinitionUse(KTextEditor::Range* range)
  : KDevDocumentRangeObject(range)
  , m_definition(0)
{
}

Definition* DefinitionUse::definition() const
{
  return m_definition;
}

void DefinitionUse::setDefinition(Definition* definition)
{
  m_definition = definition;
}

// kate: indent-width 2;
