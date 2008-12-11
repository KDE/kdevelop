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
#include <util/convenientfreelist.h>
#include "../languageexport.h"
#include "declaration.h"
#include "ducontext.h"

#ifndef PERSISTENTSYMBOLTABLE_H
#define PERSISTENTSYMBOLTABLE_H

namespace KDevelop {

  class Declaration;
  class IndexedDeclaration;
  class IndexedDUContext;
  class DeclarationId;
  class TopDUContext;
  class IndexedQualifiedIdentifier;

///@todo move into own header
class KDEVPLATFORMLANGUAGE_EXPORT IndexedDeclarationHandler {
    public:
    inline static int leftChild(const IndexedDeclaration& m_data) {
        return ((int)(m_data.dummyData().first))-1;
    }
    inline static void setLeftChild(IndexedDeclaration& m_data, int child) {
        m_data.setDummyData(qMakePair((uint)(child+1), m_data.dummyData().second));
    }
    inline static int rightChild(const IndexedDeclaration& m_data) {
        return ((int)m_data.dummyData().second)-1;
    }
    inline static void setRightChild(IndexedDeclaration& m_data, int child) {
        m_data.setDummyData(qMakePair(m_data.dummyData().first, (uint)(child+1)));
    }
    inline static void createFreeItem(IndexedDeclaration& data) {
        data = IndexedDeclaration();
        data.setIsDummy(true);
        data.setDummyData(qMakePair(0u, 0u)); //Since we substract 1, this equals children -1, -1
    }
    //Copies this item into the given one
    inline static void copyTo(const IndexedDeclaration& m_data, IndexedDeclaration& data) {
        data = m_data;
    }
    
    inline static bool isFree(const IndexedDeclaration& m_data) {
        return m_data.isDummy();
    }

    inline static bool equals(const IndexedDeclaration& m_data, const IndexedDeclaration& rhs) {
      return m_data == rhs;
    }
};

class KDEVPLATFORMLANGUAGE_EXPORT IndexedDUContextHandler {
    public:
    inline static int leftChild(const IndexedDUContext& m_data) {
        return ((int)(m_data.dummyData().first))-1;
    }
    inline static void setLeftChild(IndexedDUContext& m_data, int child) {
        m_data.setDummyData(qMakePair((uint)(child+1), m_data.dummyData().second));
    }
    inline static int rightChild(const IndexedDUContext& m_data) {
        return ((int)m_data.dummyData().second)-1;
    }
    inline static void setRightChild(IndexedDUContext& m_data, int child) {
        m_data.setDummyData(qMakePair(m_data.dummyData().first, (uint)(child+1)));
    }
    inline static void createFreeItem(IndexedDUContext& data) {
        data = IndexedDUContext();
        data.setIsDummy(true);
        data.setDummyData(qMakePair(0u, 0u)); //Since we substract 1, this equals children -1, -1
    }
    //Copies this item into the given one
    inline static void copyTo(const IndexedDUContext& m_data, IndexedDUContext& data) {
        data = m_data;
    }
    inline static bool isFree(const IndexedDUContext& m_data) {
        return m_data.isDummy();
    }

    inline static bool equals(const IndexedDUContext& m_data, const IndexedDUContext& rhs) {
      return m_data == rhs;
    }
};

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

    typedef ConstantConvenientEmbeddedSet<IndexedDeclaration, IndexedDeclarationHandler> Declarations;
    
    ///Retrieves all the declarations for a given IndexedQualifiedIdentifier in an efficient way, and returns
    ///them in a structure that is more convenient than declarations().
    ///@param id The IndexedQualifiedIdentifier for which the declarations should be retrieved
    Declarations getDeclarations(const IndexedQualifiedIdentifier& id) const;

    
    void addContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context);

    void removeContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context);

    ///Retrieves all the contexts for a given IndexedQualifiedIdentifier in an efficient way.
    ///@param id The IndexedQualifiedIdentifier for which the contexts should be retrieved
    ///@param count A reference that will be filled with the count of retrieved contexts
    ///@param contexts A reference to a pointer, that will be filled with a pointer to the retrieved contexts.
    void contexts(const IndexedQualifiedIdentifier& id, uint& count, const IndexedDUContext*& contexts) const;
    
    typedef ConstantConvenientEmbeddedSet<IndexedDUContext, IndexedDUContextHandler> Contexts;

    ///Retrieves all the contexts for a given IndexedQualifiedIdentifier in an efficient way, and returns them
    ///in a more user-friendly structure then contexs().
    ///@param id The IndexedQualifiedIdentifier for which the contexts should be retrieved
    Contexts getContexts(const IndexedQualifiedIdentifier& id) const;
    
    static PersistentSymbolTable& self();

    //Very expensive: Checks for problems in the symbol table
    void selfAnalysis();
    
    private:
      class PersistentSymbolTablePrivate* d;
  };
}

#endif

