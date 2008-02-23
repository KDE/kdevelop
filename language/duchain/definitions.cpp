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

#include "definitions.h"
#include "declarationid.h"
#include "duchainpointer.h"
#include <QHash>
#include <QVector>

namespace KDevelop {

uint qHash(const KDevelop::DeclarationId& id) {
  return id.hash();
}

struct DefinitionsPrivate {
  //Maps definitions to declaration-ids
  QHash<DeclarationId, DeclarationPointer> m_definitions;
  //Maps declaration-ids to definitions
  QHash<DeclarationPointer, DeclarationId> m_declarationIdsForDefinitions;
};

Definitions::Definitions() : d(new DefinitionsPrivate())
{
}

Definitions::~Definitions()
{
  delete d;
}

///Assigns @param definition to the given @param id.
void Definitions::setDefinition(const DeclarationId& id, Declaration* definition)
{
  if(!definition)
    d->m_definitions.remove(id);
  else {
    d->m_declarationIdsForDefinitions.insert(DeclarationPointer(definition), id);
    d->m_definitions.insert(id, DeclarationPointer(definition));
  }
}

///Gets the definition assigned to @param id, or zero.
Declaration* Definitions::definition(const DeclarationId& id) const
{
  QHash<DeclarationId, DeclarationPointer>::const_iterator it = d->m_definitions.find(id);
  if(it != d->m_definitions.end()) {
    return (*it).data();
  }else{
    return 0;
  }
}

Declaration* Definitions::declaration(const Declaration* definition, TopDUContext* context) const
{
  QHash<DeclarationPointer, DeclarationId>::const_iterator it = d->m_declarationIdsForDefinitions.find(DeclarationPointer(const_cast<Declaration*>(definition)));
  if(it == d->m_declarationIdsForDefinitions.end())
    return 0;

  return (*it).getDeclaration(context);
}


}

