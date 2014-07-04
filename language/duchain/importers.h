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

#ifndef KDEVPLATFORM_IMPORTERS_H
#define KDEVPLATFORM_IMPORTERS_H

#include "languageexport.h"
#include "declarationid.h"


namespace KDevelop {

  class DeclarationId;
  class IndexedDUContext;

/**
 * Global mapping of Declaration-Ids to contexts that import the associated context, protected through DUChainLock.
 * This is used as an alternative to the local importers list within DUContext, only for indirect imports(Across different files, or with templates).
 *
 * This has the advantage that imports stay valid even if the imported context is deleted temporarily, and stored top-contexts don't need to be
 * changed just because imports were added.
 *
 * To retrieve the actual uses, query the duchain for the files.
 * */
  class KDEVPLATFORMLANGUAGE_EXPORT Importers {
    public:
    /// Constructor.
    Importers();
    /// Destructor.
    ~Importers();
    /**
     * Adds a top-context to the users-list of the given id
     * */
    void addImporter(const DeclarationId& id, const IndexedDUContext& use);
    /**
     * Removes the given top-context from the list of uses
     * */
    void removeImporter(const DeclarationId& id, const IndexedDUContext& use);

    ///Gets the top-contexts of all users assigned to the declaration-id
    KDevVarLengthArray<IndexedDUContext> importers(const DeclarationId& id) const;

    static Importers& self();
    
    private:
      class ImportersPrivate* d;
  };
}

#endif

