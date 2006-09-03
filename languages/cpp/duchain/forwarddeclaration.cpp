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

#include "forwarddeclaration.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "ducontext.h"
#include "use.h"
#include "definition.h"
#include "cpptypes.h"
#include "symboltable.h"

using namespace KTextEditor;

ForwardDeclaration::ForwardDeclaration(KTextEditor::Range* range, Scope scope )
  : Declaration(range, scope)
  , m_resolvedDeclaration(0)
{
}

ForwardDeclaration::~ForwardDeclaration()
{
  setResolved(0);
}

Declaration * ForwardDeclaration::resolved() const
{
  return m_resolvedDeclaration;
}

void ForwardDeclaration::setResolved(Declaration * declaration)
{
  if (m_resolvedDeclaration)
    m_resolvedDeclaration->m_forwardDeclarations.removeAll(this);

  m_resolvedDeclaration = declaration;

  if (m_resolvedDeclaration) {
    m_resolvedDeclaration->m_forwardDeclarations.append(this);

    Q_ASSERT(!isDefinition());
    Q_ASSERT(!definition());

    // Offload uses...
    foreach (Use* use, uses())
      use->setDeclaration(m_resolvedDeclaration);
  }
}

bool ForwardDeclaration::isForwardDeclaration() const
{
  return true;
}
