/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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
#include "definition_p.h"

#include "topducontext.h"
#include "declaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "definitions.h"

using namespace KTextEditor;

namespace KDevelop
{

DefinitionPrivate::DefinitionPrivate()
  : m_context(0)
{
}

Definition::Definition(const HashedString& url, const SimpleRange& range, DUContext* context)
  : DUChainBase(*new DefinitionPrivate(), url, range)
  , ContextOwner(this)
{
  if( context )
    setContext(context);
}

Definition::~Definition()
{
  setContext(0);

  //DUChain::definitionChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

DUContext* Definition::context() const
{
  ENSURE_CHAIN_READ_LOCKED
  
  return d_func()->m_context;
}

void Definition::setContext(DUContext* context)
{
  ENSURE_CHAIN_WRITE_LOCKED
  Q_D(Definition);
  if (d->m_context) {
    d->m_context->takeDefinition(this);
    //DUChain::definitionChanged(this, DUChainObserver::Removal, DUChainObserver::Context, d->m_context);
  }

  d->m_context = context;

  if (d->m_context) {
    d->m_context->addDefinition(this);
    //DUChain::definitionChanged(this, DUChainObserver::Addition, DUChainObserver::Context, d->m_context);
  }
}

Declaration* Definition::declaration(TopDUContext* topContext) const
{
  ENSURE_CHAIN_READ_LOCKED

  return DUChain::definitions()->declaration(this, topContext ? topContext : this->topContext());
}

// kate: indent-width 2;

TopDUContext * Definition::topContext() const
{
  if (context())
    return context()->topContext();

  return 0;
}
}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
