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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

namespace KDevelop {

  class Declaration;
  class IndexedDeclaration;
  class DeclarationId;
  class TopDUContext;
  class QualifiedIdentifier;

/**
 * Global unique mapping of Declaration-Ids to PersistentSymbolTable, protected through DUChainLock.
 *
 * Currently it is only possible to map exactly one Declaration to exactly one DeclarationId.
 * */
  class KDEVPLATFORMLANGUAGE_EXPORT PersistentSymbolTable {
    public:
    /// Constructor.
    PersistentSymbolTable();
    /// Destructor.
    ~PersistentSymbolTable();

    void addDeclaration(const QualifiedIdentifier& id, const IndexedDeclaration& declaration);

    void removeDeclaration(const QualifiedIdentifier& id, const IndexedDeclaration& declaration);
    
    KDevVarLengthArray<IndexedDeclaration> declarations(const QualifiedIdentifier& id) const;

    private:
      class PersistentSymbolTablePrivate* d;
  };
}

#endif

