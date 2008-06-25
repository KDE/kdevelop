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
#include "topducontext.h"
#include <editor/simplecursor.h>
#include "declaration.h"
#include "symboltable.h"

namespace KDevelop {

DeclarationId::DeclarationId(const IndexedQualifiedIdentifier& id, uint additionalId, uint specialization)
  : m_identifier(id), m_additionalIdentity(additionalId), m_direct(false), m_specialization(specialization)
{
  
}

DeclarationId::DeclarationId(uint topContext, uint declaration, uint specialization)
  : m_topContext(topContext), m_declaration(declaration), m_direct(true), m_specialization(specialization)
{
  
}

IndexedQualifiedIdentifier DeclarationId::identifier() const
{
  return m_identifier;
}

uint DeclarationId::additionalIdentity() const
{
  return m_additionalIdentity;
}

Declaration* DeclarationId::getDeclaration(TopDUContext* context) const
{
  QualifiedIdentifier id(m_identifier);

  TopDUContext* top = context->topContext();
  
  QVarLengthArray<Declaration*> declarations;
  SymbolTable::self()->findDeclarationsByHash( id.hash(), declarations );
  
  Identifier lastId = id.last();
  
  FOREACH_ARRAY(Declaration* decl, declarations) {
    if(decl->identifier() == lastId) {
      if(m_additionalIdentity == decl->additionalIdentity() && (top == decl->topContext() || top->imports(decl->topContext(), SimpleCursor::invalid()))) {
        //Hit
        return decl;
      }
    }
  }

  return 0;
}

}
