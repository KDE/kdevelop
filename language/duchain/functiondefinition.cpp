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

#include "functiondefinition.h"
#include "duchainregister.h"
#include "definitions.h"

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(FunctionDefinition);

FunctionDefinition::FunctionDefinition(FunctionDefinitionData& data) : FunctionDeclaration(data)
{
}

FunctionDefinition::FunctionDefinition(const RangeInRevision& range, DUContext* context)
  : FunctionDeclaration(*new FunctionDefinitionData, range)
{
  d_func_dynamic()->setClassId(this);
  if( context )
    setContext( context );
}

FunctionDefinition::FunctionDefinition(const FunctionDefinition& rhs) : FunctionDeclaration(*new FunctionDefinitionData(*rhs.d_func())) {
}

FunctionDefinition::~FunctionDefinition() {
  if(!topContext()->isOnDisk())
    DUChain::definitions()->removeDefinition(d_func()->m_declaration, this);
}

Declaration* FunctionDefinition::declaration(const TopDUContext* topContext) const
{
  ENSURE_CAN_READ
  
  KDevVarLengthArray<Declaration*> declarations = d_func()->m_declaration.getDeclarations(topContext ? topContext : this->topContext());
  
  FOREACH_ARRAY(Declaration* decl, declarations) {
    if(!dynamic_cast<FunctionDefinition*>(decl))
      return decl;
  }
  
  return 0;
}

bool FunctionDefinition::hasDeclaration() const
{
  return d_func()->m_declaration.isValid();
}

void FunctionDefinition::setDeclaration(Declaration* declaration)
{
  ENSURE_CAN_WRITE

  if(declaration) {
    DUChain::definitions()->addDefinition(declaration->id(), this);
    d_func_dynamic()->m_declaration = declaration->id();
  }else{
    if(d_func()->m_declaration.isValid()) {
      DUChain::definitions()->removeDefinition(d_func()->m_declaration, this);
      d_func_dynamic()->m_declaration = DeclarationId();
    }
  }
}

FunctionDefinition* FunctionDefinition::definition(const Declaration* decl)
{
  ENSURE_CHAIN_READ_LOCKED
  KDevVarLengthArray<IndexedDeclaration> allDefinitions = DUChain::definitions()->definitions(decl->id());
  FOREACH_ARRAY(const IndexedDeclaration& decl, allDefinitions) {
    if(decl.data()) ///@todo Find better ways of deciding which definition to use
      return dynamic_cast<FunctionDefinition*>(decl.data());
  }
  return 0;
}

Declaration* FunctionDefinition::clonePrivate() const
{
  return new FunctionDefinition(*new FunctionDefinitionData(*d_func()));
}

}
