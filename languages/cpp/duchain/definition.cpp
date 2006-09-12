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

#include "definition.h"

#include <QReadLocker>
#include <QWriteLocker>

#include "topducontext.h"
#include "declaration.h"
#include "duchain.h"

using namespace KTextEditor;

Definition::Definition(KTextEditor::Range* range, DUContext* context)
  : DUChainBase(context->topContext())
  , KDevDocumentRangeObject(range)
  , m_context(0)
  , m_declaration(0)
{
  setContext(context);
}

Definition::~Definition()
{
  setContext(0);

  if (Declaration* dec = declaration())
    dec->setDefinition(0);

  DUChain::definitionChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

DUContext* Definition::context() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_context;
}

void Definition::setContext(DUContext* context)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (m_context) {
    m_context->takeDefinition(this);
    DUChain::definitionChanged(this, DUChainObserver::Removal, DUChainObserver::Context, m_context);
  }

  m_context = context;

  if (m_context) {
    m_context->addDefinition(this);
    DUChain::definitionChanged(this, DUChainObserver::Addition, DUChainObserver::Context, m_context);
  }
}

Declaration* Definition::declaration() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_declaration;
}

void Definition::setDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (m_declaration)
    DUChain::definitionChanged(this, DUChainObserver::Removal, DUChainObserver::DefinitionRelationship, m_declaration);

  // TODO if declaration is 0, highlight as definition without declaration
  m_declaration = declaration;

  if (m_declaration)
    DUChain::definitionChanged(this, DUChainObserver::Addition, DUChainObserver::DefinitionRelationship, m_declaration);
}

// kate: indent-width 2;
