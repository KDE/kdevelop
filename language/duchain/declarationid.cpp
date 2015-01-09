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
#include "duchain.h"
#include "declaration.h"
#include "persistentsymboltable.h"
#include "instantiationinformation.h"

#include "../editor/cursorinrevision.h"

#include <util/convenientfreelist.h>

namespace KDevelop {

DeclarationId::DeclarationId(const IndexedQualifiedIdentifier& id, uint additionalId,
                             const IndexedInstantiationInformation& specialization)
  : m_direct(false)
  , m_specialization(specialization)
{
  indirect.m_identifier = id;
  indirect.m_additionalIdentity = additionalId;
}

DeclarationId::DeclarationId(const IndexedDeclaration& decl,
                             const IndexedInstantiationInformation& specialization)
  : direct(decl)
  , m_direct(true)
  , m_specialization(specialization)
{

}

bool DeclarationId::isDirect() const
{
  return m_direct;
}

void DeclarationId::setSpecialization(const IndexedInstantiationInformation& spec)
{
  m_specialization = spec;
}

IndexedInstantiationInformation DeclarationId::specialization() const
{
  return m_specialization;
}

KDevVarLengthArray<Declaration*> DeclarationId::getDeclarations(const TopDUContext* top) const
{
  KDevVarLengthArray<Declaration*> ret;

  if(m_direct == false) {
    //Find the declaration by its qualified identifier and additionalIdentity
    QualifiedIdentifier id(indirect.m_identifier);

    if(top) {
      //Do filtering
      PersistentSymbolTable::FilteredDeclarationIterator filter =
          PersistentSymbolTable::self().getFilteredDeclarations(id, top->recursiveImportIndices());
      for(; filter; ++filter) {
          Declaration* decl = filter->data();
          if(decl && indirect.m_additionalIdentity == decl->additionalIdentity()) {
            //Hit
            ret.append(decl);
          }
      }
    }else{
      //Just accept anything
      PersistentSymbolTable::Declarations decls = PersistentSymbolTable::self().getDeclarations(id);
      PersistentSymbolTable::Declarations::Iterator decl = decls.iterator();
      for(; decl; ++decl) {
        const IndexedDeclaration& iDecl(*decl);

          ///@todo think this over once we don't pull in all imported top-context any more
        //Don't trigger loading of top-contexts from here, it will create a lot of problems
        if((!DUChain::self()->isInMemory(iDecl.topContextIndex())))
          continue;

        if(!top) {
          Declaration* decl = iDecl.data();
          if(decl && indirect.m_additionalIdentity == decl->additionalIdentity()) {
            //Hit
            ret.append(decl);
          }
        }
      }
    }
  }else{
    Declaration* decl = direct.declaration();
    if(decl)
      ret.append(decl);
  }

  if(!ret.isEmpty() && m_specialization.index()) {
    KDevVarLengthArray<Declaration*> newRet;
    for (Declaration* decl : ret) {
        Declaration* specialized = decl->specialize(m_specialization, top ? top : decl->topContext());
        if(specialized)
          newRet.append(specialized);
    }
    return newRet;
  }
  return ret;
}

Declaration* DeclarationId::getDeclaration(const TopDUContext* top, bool instantiateIfRequired) const
{
  Declaration* ret = 0;

  if(m_direct == false) {
    //Find the declaration by its qualified identifier and additionalIdentity
    QualifiedIdentifier id(indirect.m_identifier);

    if(top) {
      //Do filtering
      PersistentSymbolTable::FilteredDeclarationIterator filter =
          PersistentSymbolTable::self().getFilteredDeclarations(id, top->recursiveImportIndices());
      for(; filter; ++filter) {
          Declaration* decl = filter->data();
          if(decl && indirect.m_additionalIdentity == decl->additionalIdentity()) {
            //Hit
            ret = decl;
            if(!ret->isForwardDeclaration())
              break;
          }
      }
    }else{
      //Just accept anything
      PersistentSymbolTable::Declarations decls = PersistentSymbolTable::self().getDeclarations(id);
      PersistentSymbolTable::Declarations::Iterator decl = decls.iterator();
      for(; decl; ++decl) {
        const IndexedDeclaration& iDecl(*decl);

        ///@todo think this over once we don't pull in all imported top-context any more
        //Don't trigger loading of top-contexts from here, it will create a lot of problems
        if((!DUChain::self()->isInMemory(iDecl.topContextIndex())))
          continue;

        if(!top) {
          Declaration* decl = iDecl.data();
          if(decl && indirect.m_additionalIdentity == decl->additionalIdentity()) {
            //Hit
            ret = decl;
            if(!ret->isForwardDeclaration())
              break;
          }
        }
      }
    }
  }else{
    //Find the declaration by m_topContext and m_declaration
    ret = direct.declaration();
  }


  if(ret)
  {
    if(m_specialization.isValid())
    {
      const TopDUContext* topContextForSpecialization = top;
      if(!instantiateIfRequired)
        topContextForSpecialization = 0; //If we don't want to instantiate new declarations, set the top-context to zero, so specialize(..) will only look-up
      else if(!topContextForSpecialization)
        topContextForSpecialization = ret->topContext();

      return ret->specialize(m_specialization, topContextForSpecialization);
    }else{
      return ret;
    }
  }else
    return 0;
}

QualifiedIdentifier DeclarationId::qualifiedIdentifier() const
{

  if(!m_direct) {
    QualifiedIdentifier baseIdentifier = indirect.m_identifier.identifier();
    if(!m_specialization.index())
      return baseIdentifier;
    return m_specialization.information().applyToIdentifier(baseIdentifier);
  } else {
    Declaration* decl = getDeclaration(0);
    if(decl)
      return decl->qualifiedIdentifier();

    return QualifiedIdentifier(i18n("(unknown direct declaration)"));
  }

  return QualifiedIdentifier(i18n("(missing)")) + indirect.m_identifier.identifier();
}

}
