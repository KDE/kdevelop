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
#include "language/languageexport.h"
#include <util/kdevvarlengtharray.h>

#ifndef PERSISTENTSYMBOLTABLE_H
#define PERSISTENTSYMBOLTABLE_H

namespace KDevelop {

  class Declaration;
  class IndexedDeclaration;
  class DeclarationId;
  class TopDUContext;
  class QualifiedIdentifier;

/**
 * Global symbol-table that is stored to disk, and allows retrieving declarations that currently are not loaded to memory.
 * */
  class KDEVPLATFORMLANGUAGE_EXPORT PersistentSymbolTable {
    public:
    /// Constructor.
    PersistentSymbolTable();
    /// Destructor.
    ~PersistentSymbolTable();

    void addDeclaration(const QualifiedIdentifier& id, const IndexedDeclaration& declaration);

    void removeDeclaration(const QualifiedIdentifier& id, const IndexedDeclaration& declaration);

    ///Retrieves all the declarations for a given QualifiedIdentifier in an efficient way.
    ///@param id The QualifiedIdentifier for which the declarations should be retrieved
    ///@param countTarget A reference that will be filled with the count of retrieved declarations
    ///@param declarations A reference to a pointer, that will be filled with a pointer to the retrieved declarations.
    void declarations(const QualifiedIdentifier& id, uint& count, const IndexedDeclaration*& declarations) const;

    static PersistentSymbolTable& self();
    
    private:
      class PersistentSymbolTablePrivate* d;
  };
}

#endif

