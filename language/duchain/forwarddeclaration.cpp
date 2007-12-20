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
#include "declaration_p.h"

using namespace KTextEditor;

namespace KDevelop
{

class ForwardDeclarationPrivate : public DeclarationPrivate
{
public:
  ForwardDeclarationPrivate() {
    m_resolvedDeclaration = 0;
  }
  ForwardDeclarationPrivate( const ForwardDeclarationPrivate& rhs ) 
      : DeclarationPrivate( rhs )
  {
    m_resolvedDeclaration = 0;
  }
  Declaration* m_resolvedDeclaration;
};

ForwardDeclaration::ForwardDeclaration(const ForwardDeclaration& rhs) : Declaration(*new ForwardDeclarationPrivate(*rhs.d_func())) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

ForwardDeclaration::ForwardDeclaration(const HashedString& url, const SimpleRange& range, Scope scope, DUContext* context )
  : Declaration(*new ForwardDeclarationPrivate, url, range, scope)
{
  if( context )
    setContext( context );
}

ForwardDeclaration::~ForwardDeclaration()
{
  setResolved(0);
}

Declaration * ForwardDeclaration::resolved() const
{
  ENSURE_CAN_READ

  return d_func()->m_resolvedDeclaration;
}

void ForwardDeclaration::setResolved(Declaration * declaration)
{
  ENSURE_CAN_WRITE
  Q_D(ForwardDeclaration);
  if (d->m_resolvedDeclaration)
    d->m_resolvedDeclaration->removeForwardDeclaration(this);

  d->m_resolvedDeclaration = declaration;

  if (d->m_resolvedDeclaration) {
    d->m_resolvedDeclaration->addForwardDeclaration(this);

    Q_ASSERT(!isDefinition());
    Q_ASSERT(!definition());

    // Offload uses...
    foreach (Use* use, uses())
      use->d_func()->setDeclaration(d->m_resolvedDeclaration);
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

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
