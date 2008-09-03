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

#include <util/kdevvarlengtharray.h>
#include "../languageexport.h"

#ifndef PERSISTENTSYMBOLTABLE_H
#define PERSISTENTSYMBOLTABLE_H

namespace KDevelop {

  class Declaration;
  class IndexedDeclaration;
  class IndexedDUContext;
  class DeclarationId;
  class TopDUContext;
  class IndexedQualifiedIdentifier;

/**
 * Global symbol-table that is stored to disk, and allows retrieving declarations that currently are not loaded to memory.
 * */
  class KDEVPLATFORMLANGUAGE_EXPORT PersistentSymbolTable {
    public:
    /// Constructor.
    PersistentSymbolTable();
    /// Destructor.
    ~PersistentSymbolTable();

    void addDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration);

    void removeDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration);

    ///Retrieves all the declarations for a given IndexedQualifiedIdentifier in an efficient way.
    ///@param id The IndexedQualifiedIdentifier for which the declarations should be retrieved
    ///@param count A reference that will be filled with the count of retrieved declarations
    ///@param declarations A reference to a pointer, that will be filled with a pointer to the retrieved declarations.
    void declarations(const IndexedQualifiedIdentifier& id, uint& count, const IndexedDeclaration*& declarations) const;

    
    void addContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context);

    void removeContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context);

    ///Retrieves all the contexts for a given IndexedQualifiedIdentifier in an efficient way.
    ///@param id The IndexedQualifiedIdentifier for which the contexts should be retrieved
    ///@param count A reference that will be filled with the count of retrieved contexts
    ///@param contexts A reference to a pointer, that will be filled with a pointer to the retrieved contexts.
    void contexts(const IndexedQualifiedIdentifier& id, uint& count, const IndexedDUContext*& contexts) const;
    
    static PersistentSymbolTable& self();
    
    private:
      class PersistentSymbolTablePrivate* d;
  };
}

#endif

