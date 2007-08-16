/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "contextowner.h"
#include "declaration.h"
#include "definition.h"
#include "ducontext.h"

namespace  KDevelop {

struct ContextOwnerPrivate
{
  ContextOwnerPrivate() : m_internalContext(0) {
  }
  DUContext* m_internalContext;
};

ContextOwner::ContextOwner(DUChainBase* self) : d(new ContextOwnerPrivate) {
}

ContextOwner::~ContextOwner()
{
  if( d->m_internalContext )
    d->m_internalContext->setOwner(0);
  
  delete d;
}

void ContextOwner::setInternalContext(DUContext* context)
{
//  ENSURE_CAN_WRITE ///@todo this check is needed
  
  if( context == d->m_internalContext )
    return;

  DUContext* oldInternalContext = d->m_internalContext;
  
  d->m_internalContext = context;

  //Q_ASSERT( !oldInternalContext || oldInternalContext->owner() == this );
  if( oldInternalContext && oldInternalContext->owner() == this )
    oldInternalContext->setOwner(0);
  

  if( d->m_internalContext )
    d->m_internalContext->setOwner(this);
}

DUContext* ContextOwner::internalContext() const {
  return d->m_internalContext;
}

Declaration* ContextOwner::asDeclaration() {
  return dynamic_cast<KDevelop::Declaration*>(this);
}

///If this is a definition, this returns the pointer, else zero.
Definition* ContextOwner::asDefinition() {
  return dynamic_cast<KDevelop::Definition*>(this);
}

const Declaration* ContextOwner::asDeclaration() const {
  return dynamic_cast<const KDevelop::Declaration*>(this);
}

///If this is a definition, this returns the pointer, else zero.
const Definition* ContextOwner::asDefinition() const {
  return dynamic_cast<const KDevelop::Definition*>(this);
}

}
