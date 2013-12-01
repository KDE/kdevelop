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
#include <language/util/setrepository.h>
#include "../languageexport.h"
#include "declaration.h"
#include "ducontext.h"
#include "topducontext.h"

#ifndef KDEVPLATFORM_PERSISTENTSYMBOLTABLE_H
#define KDEVPLATFORM_PERSISTENTSYMBOLTABLE_H

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
        data.setDummyData(qMakePair(0u, 0u)); //Since we subtract 1, this equals children -1, -1
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
        data.setDummyData(qMakePair(0u, 0u)); //Since we subtract 1, this equals children -1, -1
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

struct DeclarationTopContextExtractor {
  inline static IndexedTopDUContext extract(const IndexedDeclaration& decl) {
    return decl.indexedTopContext();
  }
};

struct DUContextTopContextExtractor {
  inline static IndexedTopDUContext extract(const IndexedDUContext& ctx) {
    return ctx.indexedTopContext();
  }
};

struct KDEVPLATFORMLANGUAGE_EXPORT RecursiveImportCacheRepository {
  static Utils::BasicSetRepository* repository();
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

    ///Adds declaration @p declaration with id @p id to the symbol table
    ///@warning DUChain must be write locked
    void addDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration);

    ///Adds declaration @p declaration with id @p id to the symbol table
    ///@warning DUChain must be write locked
    void removeDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration);

    ///Retrieves all the declarations for a given IndexedQualifiedIdentifier in an efficient way.
    ///@param id The IndexedQualifiedIdentifier for which the declarations should be retrieved
    ///@param count A reference that will be filled with the count of retrieved declarations
    ///@param declarations A reference to a pointer, that will be filled with a pointer to the retrieved declarations.
    ///@warning DUChain must be read locked as long as the returned data is used
    void declarations(const IndexedQualifiedIdentifier& id, uint& count, const IndexedDeclaration*& declarations) const;

    typedef ConstantConvenientEmbeddedSet<IndexedDeclaration, IndexedDeclarationHandler> Declarations;
    
    ///Retrieves all the declarations for a given IndexedQualifiedIdentifier in an efficient way, and returns
    ///them in a structure that is more convenient than declarations().
    ///@param id The IndexedQualifiedIdentifier for which the declarations should be retrieved
    ///@warning DUChain must be read locked as long as the returned data is used
    Declarations getDeclarations(const IndexedQualifiedIdentifier& id) const;

    typedef Utils::StorableSet<IndexedTopDUContext, IndexedTopDUContextIndexConversion, RecursiveImportCacheRepository, true> CachedIndexedRecursiveImports;
    
    typedef ConvenientEmbeddedSetTreeFilterIterator<IndexedDeclaration, IndexedDeclarationHandler, IndexedTopDUContext, CachedIndexedRecursiveImports, DeclarationTopContextExtractor> FilteredDeclarationIterator;
    ///Retrieves an iterator to all declarations of the given id, filtered by the visilibity given through @param visibility
    ///This is very efficient since it uses a cache
    ///The returned iterator is valid as long as the duchain read lock is held
    FilteredDeclarationIterator getFilteredDeclarations(const IndexedQualifiedIdentifier& id, const TopDUContext::IndexedRecursiveImports& visibility) const;

    static PersistentSymbolTable& self();

    //Very expensive: Checks for problems in the symbol table
    void selfAnalysis();
    
    //Clears the internal cache. Should be called regularly to save memory
    //The duchain must be read-locked
    void clearCache();
    
    private:
      class PersistentSymbolTablePrivate* d;
  };
}

#endif

