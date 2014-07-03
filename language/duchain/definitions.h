/* This file is part of KDevelop
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#include "../languageexport.h"
#include <util/kdevvarlengtharray.h>

#ifndef KDEVPLATFORM_DEFINITIONS_H
#define KDEVPLATFORM_DEFINITIONS_H

class QTextStream;

namespace KDevelop {

  class Declaration;
  class IndexedDeclaration;
  class DeclarationId;
  class TopDUContext;

/**
 * Global mapping of one Declaration-Ids to multiple Definitions, protected through DUChainLock.
 * */
  class KDEVPLATFORMLANGUAGE_EXPORT Definitions {
    public:
    /// Constructor.
    Definitions();
    /// Destructor.
    ~Definitions();
    /**
     * Assigns @param definition to the given @param id.
     * */
    void addDefinition(const DeclarationId& id, const IndexedDeclaration& definition);

    void removeDefinition(const DeclarationId& id, const IndexedDeclaration& definition);
    
    ///Gets all the known definitions assigned to @param id.
    KDevVarLengthArray<IndexedDeclaration> definitions(const DeclarationId& id) const;

    /// Dump contents of the definitions repository to stream @p out
    void dump(const QTextStream& out);

    private:
      class DefinitionsPrivate* d;
  };
}

#endif

