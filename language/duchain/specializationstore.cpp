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

#include "specializationstore.h"
#include "declarationid.h"
#include "ducontext.h"
#include "declaration.h"

namespace KDevelop {

SpecializationStore::SpecializationStore()
{
}

SpecializationStore::~SpecializationStore()
{
}

SpecializationStore& SpecializationStore::self()
{
  static SpecializationStore store;
  return store;
}

void SpecializationStore::set(const DeclarationId& declaration,
                              const IndexedInstantiationInformation& specialization)
{
  Q_ASSERT(specialization.index() >> 16);
  m_specializations[declaration] = specialization;
}

IndexedInstantiationInformation SpecializationStore::get(const DeclarationId& declaration)
{
  QHash<DeclarationId, IndexedInstantiationInformation>::const_iterator it = m_specializations.constFind(declaration);
  if(it != m_specializations.constEnd())
    return *it;
  else
    return IndexedInstantiationInformation();
}

void SpecializationStore::clear(const DeclarationId& declaration)
{
  QHash<DeclarationId, IndexedInstantiationInformation>::iterator it = m_specializations.find(declaration);
  if(it != m_specializations.end())
    m_specializations.erase(it);
}

void SpecializationStore::clear()
{
  m_specializations.clear();
}

Declaration* SpecializationStore::applySpecialization(Declaration* declaration, TopDUContext* source,
                                                      bool recursive)
{
  if(!declaration)
    return 0;

  IndexedInstantiationInformation specialization = get(declaration->id());
  if(specialization.index())
    return declaration->specialize(specialization, source);

  if(declaration->context() && recursive) {
    //Find a parent that has a specialization, and specialize this with the info and required depth
    int depth = 0;
    DUContext* ctx = declaration->context();
    IndexedInstantiationInformation specialization;
    while(ctx && !specialization.index()) {
      if(ctx->owner())
        specialization = get(ctx->owner()->id());
      ++depth;
      ctx = ctx->parentContext();
    }

    if(specialization.index())
      return declaration->specialize(specialization, source, depth);
  }

  return declaration;
}

DUContext* SpecializationStore::applySpecialization(DUContext* context, TopDUContext* source,
                                                    bool recursive)
{
  if(!context)
    return 0;

  if(Declaration* declaration = context->owner())
    return applySpecialization(declaration, source, recursive)->internalContext();

  if(context->parentContext() && recursive) {
    //Find a parent that has a specialization, and specialize this with the info and required depth
    int depth = 0;
    DUContext* ctx = context->parentContext();
    IndexedInstantiationInformation specialization;
    while(ctx && !specialization.index()) {
      if(ctx->owner())
        specialization = get(ctx->owner()->id());
      ++depth;
      ctx = ctx->parentContext();
    }

    if(specialization.index())
      return context->specialize(specialization, source, depth);
  }

  return context;
}

}
