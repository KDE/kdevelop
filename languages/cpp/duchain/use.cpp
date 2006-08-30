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

#include "use.h"

#include "declaration.h"
#include "ducontext.h"

using namespace KTextEditor;

Use::Use(KTextEditor::Range* range, DUContext* context)
  : KDevDocumentRangeObject(range)
  , m_context(0)
  , m_declaration(0)
{
  if (context)
    setContext(context);
}

Use::~Use()
{
  setContext(0);
}

Declaration* Use::declaration() const
{
  return m_declaration;
}

void Use::setDeclaration(Declaration* declaration)
{
  m_declaration = declaration;
}

// kate: indent-width 2;

void Use::setContext(DUContext * context)
{
  if (m_context)
    m_context->m_uses.removeAll(this);

  m_context = context;

  if (m_context)
    m_context->m_uses.append(this);
}

DUContext * Use::context() const
{
  return m_context;
}

bool Use::isOrphan() const
{
  return !m_declaration;
}
