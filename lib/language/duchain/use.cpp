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
#include "topducontext.h"
#include "duchain.h"
#include "duchainlock.h"

using namespace KTextEditor;

namespace KDevelop
{

class UsePrivate
{
public:
  DUContext* m_context;
  Declaration* m_declaration;
};

Use::Use(KTextEditor::Range* range, DUContext* context)
  : DUChainBase(range)
  , d(new UsePrivate)
{
  d->m_context = 0;
  d->m_declaration = 0;
  if (context)
    setContext(context);
}

Use::~Use()
{
  setContext(0);

  if (d->m_declaration)
    d->m_declaration->removeUse(this);

  DUChain::useChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
  delete d;
}

Declaration* Use::declaration() const
{
  ENSURE_CHAIN_READ_LOCKED

  return d->m_declaration;
}

void Use::setDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (d->m_declaration)
    DUChain::useChanged(this, DUChainObserver::Removal, DUChainObserver::DeclarationRelationship, d->m_declaration);

  d->m_declaration = declaration;

  if (d->m_declaration)
    DUChain::useChanged(this, DUChainObserver::Addition, DUChainObserver::DeclarationRelationship, d->m_declaration);
}

void Use::setContext(DUContext * context)
{
  if (d->m_context) {
    ENSURE_CHAIN_WRITE_LOCKED

    d->m_context->removeUse(this);

    DUChain::useChanged(this, DUChainObserver::Removal, DUChainObserver::Context, d->m_context);
  }

  d->m_context = context;

  if (d->m_context) {
    ENSURE_CHAIN_WRITE_LOCKED

    d->m_context->addUse(this);

    DUChain::useChanged(this, DUChainObserver::Addition, DUChainObserver::Context, d->m_context);
  }
}

DUContext * Use::context() const
{
  ENSURE_CHAIN_READ_LOCKED

  return d->m_context;
}

bool Use::isOrphan() const
{
  return !declaration();
}

TopDUContext * Use::topContext() const
{
  if (context())
    return context()->topContext();

  return 0;
}

}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
