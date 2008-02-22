/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "declarationid.h"
#include "ducontext.h"
#include <editor/simplecursor.h>
#include "declaration.h"

namespace KDevelop {

DeclarationId::DeclarationId(const HashedString& url, const QualifiedIdentifier& id, uint additionalId)
  : m_url(url), m_identifier(id), m_additionalIdentity(additionalId)
{
  
}

QualifiedIdentifier DeclarationId::identifier() const
{
  return m_identifier;
}

HashedString DeclarationId::url() const
{
  return m_url;
}

uint DeclarationId::additionalIdentity() const
{
  return m_additionalIdentity;
}

Declaration* DeclarationId::getDeclaration(DUContext* context) const
{
  ///@todo This is quite slow, maybe think of a better way, especially for template-declarations.
  //Resolve DeclarationId
kDebug() << "searching" << m_identifier.toString();
  QList<Declaration*> declarations = context->findDeclarations(m_identifier, SimpleCursor::invalid(), AbstractType::Ptr(), 0, DUContext::NoFiltering);
kDebug() << "searched" << m_identifier.toString() <<", found" << declarations.count();
  for(QList<Declaration*>::const_iterator it2 = declarations.begin(); it2 != declarations.end(); ++it2) {
    if( m_additionalIdentity == (*it2)->additionalIdentity() &&
        m_url == (*it2)->url() )
      return *it2;
  }

  declarations = context->findDeclarations(m_identifier, SimpleCursor::invalid(), AbstractType::Ptr(), 0, (DUContext::SearchFlags)(DUContext::NoFiltering | DUContext::NoImportsCheck));
  for(QList<Declaration*>::const_iterator it2 = declarations.begin(); it2 != declarations.end(); ++it2) {
    if( m_additionalIdentity == (*it2)->additionalIdentity() &&
        m_url == (*it2)->url() )
      return *it2;
  }

  return 0;
}

}
