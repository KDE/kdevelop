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

#include <QObject>
#include "../languageexport.h"

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/**
 * Global unique mapping of Declaration-Ids to Definitions, protected through DUChainLock.
 *
 * Currently it is only possible to map exactly one Definition to exactly one DeclarationId.
 * */

namespace KDevelop {

  class Declaration;
  class DeclarationId;
  class Definition;
  class TopDUContext;

  class KDEVPLATFORMLANGUAGE_EXPORT Definitions {
    public:
    Definitions();
    ~Definitions();
    /**
     * Assigns @param definition to the given @param id.
     *
     * If @param definition is zero, removes the assignment.
     * */
    void setDefinition(const DeclarationId& id, Definition* definition);

    ///Gets the definition assigned to @param id, or zero.
    Definition* definition(const DeclarationId& id) const;

    ///Gets the declaration assigned to the given @param definition in @param context
    Declaration* declaration(const Definition* definition, TopDUContext* context) const;
    
    private:
      class DefinitionsPrivate* d;
  };
}

#endif

