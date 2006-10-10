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

#include <QReadLocker>
#include <QWriteLocker>

#include "declaration.h"
#include "topducontext.h"
#include "duchain.h"

using namespace KTextEditor;

Use::Use(KTextEditor::Range* range, DUContext* context)
  : DUChainBase(context ? context->topContext() : 0)
  , KDevDocumentRangeObject(range)
  , m_context(0)
  , m_declaration(0)
{
  if (context)
    setContext(context);
}

Use::~Use()
{
  setContext(0);

  if (m_declaration)
    m_declaration->removeUse(this);

  DUChain::useChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

Declaration* Use::declaration() const
{
  QReadLocker lock(&m_declarationLock);

  return m_declaration;
}

void Use::setDeclaration(Declaration* declaration)
{
  QWriteLocker lock(&m_declarationLock);

  if (m_declaration)
    DUChain::useChanged(this, DUChainObserver::Removal, DUChainObserver::DeclarationRelationship, m_declaration);

  m_declaration = declaration;

  if (m_declaration)
    DUChain::useChanged(this, DUChainObserver::Addition, DUChainObserver::DeclarationRelationship, m_declaration);
}

void Use::setContext(DUContext * context)
{
  if (m_context) {
    ENSURE_CHAIN_WRITE_LOCKED

    m_context->removeUse(this);

    DUChain::useChanged(this, DUChainObserver::Removal, DUChainObserver::Context, m_context);
  }

  m_context = context;

  if (m_context) {
    ENSURE_CHAIN_WRITE_LOCKED

    m_context->addUse(this);

    DUChain::useChanged(this, DUChainObserver::Addition, DUChainObserver::Context, m_context);
  }
}

DUContext * Use::context() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_context;
}

bool Use::isOrphan() const
{
  return !declaration();
}

// kate: indent-width 2;
