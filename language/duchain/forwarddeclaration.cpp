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

#include "duchain.h"
#include "duchainlock.h"
#include "ducontext.h"
#include "use.h"
#include "definition.h"
#include "symboltable.h"
#include "use_p.h"

using namespace KTextEditor;

namespace KDevelop
{

class ForwardDeclarationPrivate
{
public:
  Declaration* m_resolvedDeclaration;
};

ForwardDeclaration::ForwardDeclaration(const ForwardDeclaration& rhs) : Declaration(rhs), d(new ForwardDeclarationPrivate) {
  d->m_resolvedDeclaration = 0; ///@todo think about maybe using duchainpointer here, so we don't need to register ourselves as a forward-declaration
  setResolved( rhs.d->m_resolvedDeclaration );
}

ForwardDeclaration::ForwardDeclaration(KTextEditor::Range* range, Scope scope, DUContext* context )
  : Declaration(range, scope, context)
  , d(new ForwardDeclarationPrivate)
{
  d->m_resolvedDeclaration = 0;
}

ForwardDeclaration::~ForwardDeclaration()
{
  setResolved(0);
  delete d;
}

Declaration * ForwardDeclaration::resolved() const
{
  ENSURE_CHAIN_READ_LOCKED

  return d->m_resolvedDeclaration;
}

void ForwardDeclaration::setResolved(Declaration * declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (d->m_resolvedDeclaration)
    d->m_resolvedDeclaration->removeForwardDeclaration(this);

  d->m_resolvedDeclaration = declaration;

  if (d->m_resolvedDeclaration) {
    d->m_resolvedDeclaration->addForwardDeclaration(this);

    Q_ASSERT(!isDefinition());
    Q_ASSERT(!definition());

    // Offload uses...
    foreach (Use* use, uses())
      use->d->setDeclaration(d->m_resolvedDeclaration);
  }
}

bool ForwardDeclaration::isForwardDeclaration() const
{
  return true;
}

Declaration* ForwardDeclaration::clone() const {
  return new ForwardDeclaration(*this);
}

}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
