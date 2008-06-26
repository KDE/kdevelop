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
#include <duchain.h>
#include "declaration.h"
#include "symboltable.h"

namespace KDevelop {

DeclarationId::DeclarationId(const IndexedQualifiedIdentifier& id, uint additionalId, uint specialization)
  : m_direct(false), m_specialization(specialization)
{
  indirect.m_identifier = id;
  indirect.m_additionalIdentity = additionalId;
  
}

DeclarationId::DeclarationId(const IndexedDeclaration& decl, uint specialization)
  : direct(decl), m_direct(true), m_specialization(specialization)
{
  
}

// IndexedQualifiedIdentifier DeclarationId::identifier() const
// {
//   return m_identifier;
// }
// 
// uint DeclarationId::additionalIdentity() const
// {
//   return m_additionalIdentity;
// }

bool DeclarationId::isDirect() const
{
  return m_direct;
}

void DeclarationId::setSpecialization(uint spec) {
  m_specialization = spec;
}

uint DeclarationId::specialization() const {
  return m_specialization;
}

Declaration* DeclarationId::getDeclaration(TopDUContext* top) const
{
  Declaration* ret = 0;
  
  if(m_direct == false) {
    //Find the declaration by its qualified identifier and additionalIdentity
    QualifiedIdentifier id(indirect.m_identifier);

    QVarLengthArray<Declaration*> declarations;
    SymbolTable::self()->findDeclarationsByHash( id.hash(), declarations );
    
    Identifier lastId = id.last();
    
    FOREACH_ARRAY(Declaration* decl, declarations) {
      if(decl->identifier() == lastId) {
        if(indirect.m_additionalIdentity == decl->additionalIdentity() && (top == decl->topContext() || top->imports(decl->topContext(), SimpleCursor::invalid()))) {
          //Hit
          ret = decl;
          break;
        }
      }
    }
  }else{
    //Find the declaration by m_topContext and m_declaration
  }
  
  if(ret)
    return ret->specialize(m_specialization);
  else
    return 0;
}

}
