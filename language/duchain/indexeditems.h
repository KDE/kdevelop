/***************************************************************************
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INDEXEDITEMS_H
#define INDEXEDITEMS_H

#include <sys/types.h>
#include "../languageexport.h"

namespace KDevelop {

class Declaration;
class TopDUContext;
class IndexedString;
  
///Allows simple indirect access to top-contexts with on-demand loading
class KDEVPLATFORMLANGUAGE_EXPORT IndexedTopDUContext {
  public:
    inline IndexedTopDUContext(uint index) : m_index(index) {
      if(!index)
        setIsDummy(true);
    }
    IndexedTopDUContext(TopDUContext* context = 0);
    
    enum {
      DummyMask = 1u<<31u
    };
    
    ///Returns the top-context represented by this indexed top-context. If it wasn't loaded yet, it is loaded.
    ///The duchain must be read-locked when this is called!
    ///To prevent it from being unloaded, store it into a ReferencedTopDUContext before releasing the duchain lock.
    TopDUContext* data() const;
    
    ///Returns whether the top-context is currently loaded. If not, it will be loaded when you call data().
    bool isLoaded() const;
    
    inline bool operator==(const IndexedTopDUContext& rhs) const {
      return m_index == rhs.m_index;
    }
    
    inline bool operator!=(const IndexedTopDUContext& rhs) const {
      return m_index != rhs.m_index;
    }
    
    inline bool operator<(const IndexedTopDUContext& rhs) const {
      return m_index < rhs.m_index;
    }
    
    inline bool isValid() const {
      return m_index && !isDummy();
    }
    
    inline uint index() const {
      if(isDummy())
        return 0;
      else
        return m_index;
    }
    
    inline bool isDummy() const {
      return m_index & DummyMask;
    }
    
    void setIsDummy(bool isDummy) {
      if(isDummy)
        m_index |= DummyMask;
      else
        m_index &= ~((uint)DummyMask);
    }
    
    ///Allows giving this IndexedTopDUContext some data while logically keeping it invalid.
    ///It will still return zero on index(), data(), etc.
    ///@param first The highest of this value bit will be removed
    void setDummyData(ushort first, ushort second) {
      Q_ASSERT(isDummy());
      m_index = ((((uint)first)<<16) + second) | DummyMask;
    }

    ///The data previously set through setDummyData(). Initially 0.
    QPair<ushort, ushort> dummyData() const {
      uint withoutMask = m_index & (~((uint)DummyMask));
      return qMakePair((ushort)(withoutMask >> 16), (ushort)withoutMask);
    }
    
    IndexedString url() const;
  private:
  uint m_index;
  friend class IndexedTopDUContextEmbeddedTreeHandler;
};

struct IndexedTopDUContextIndexConversion {
  inline static uint toIndex(const IndexedTopDUContext& top) {
    return top.index();
  }
  
  inline static IndexedTopDUContext toItem(uint index) {
    return IndexedTopDUContext(index);
  }
};

class IndexedTopDUContextEmbeddedTreeHandler {
    public:
    static int leftChild(const IndexedTopDUContext& m_data) {
        return int(m_data.dummyData().first)-1;
    }
    static void setLeftChild(IndexedTopDUContext& m_data, int child) {
        m_data.setDummyData((ushort)(child+1), m_data.dummyData().second);
    }
    static int rightChild(const IndexedTopDUContext& m_data) {
        return int(m_data.dummyData().second)-1;
    }
    static void setRightChild(IndexedTopDUContext& m_data, int child) {
        m_data.setDummyData(m_data.dummyData().first, (ushort)(child+1));
    }
    static void createFreeItem(IndexedTopDUContext& data) {
        data = IndexedTopDUContext();
        data.setIsDummy(true);
        data.setDummyData(0u, 0u); //Since we subtract 1, this equals children -1, -1
    }
    //Copies this item into the given one
    static void copyTo(const IndexedTopDUContext& m_data, IndexedTopDUContext& data) {
        data = m_data;
    }
    static bool isFree(const IndexedTopDUContext& m_data) {
        return m_data.isDummy();
    }

    static bool equals(const IndexedTopDUContext& m_data, const IndexedTopDUContext& rhs) {
      return m_data == rhs;
    }
};



///Represents a declaration only by its global indices
class KDEVPLATFORMLANGUAGE_EXPORT IndexedDeclaration {
  public:
    IndexedDeclaration(Declaration* decl = 0);
    IndexedDeclaration(uint topContext, uint declarationIndex);

    ///Duchain must be read locked
    Declaration* declaration() const;

    ///Duchain must be read locked
    Declaration* data() const {
      return declaration();
    }

    inline bool operator==(const IndexedDeclaration& rhs) const {
      return m_topContext == rhs.m_topContext && m_declarationIndex == rhs.m_declarationIndex;
    }

    inline uint hash() const {
      if(isDummy())
        return 0;
      return (m_topContext * 53 + m_declarationIndex) * 23;
    }

    ///@warning The duchain needs to be locked when this is called
    inline bool isValid() const {
      return !isDummy() && declaration() != 0;
    }

    inline bool operator<(const IndexedDeclaration& rhs) const {
      Q_ASSERT(!isDummy());
      return m_topContext < rhs.m_topContext || (m_topContext == rhs.m_topContext && m_declarationIndex < rhs.m_declarationIndex);
    }

    ///Index of the Declaration within the top context
    inline uint localIndex() const {
      if(isDummy())
        return 0;
      else
        return m_declarationIndex;
    }

    inline uint topContextIndex() const {
      if(isDummy())
        return 0;
      else
        return m_topContext;
    }

    inline IndexedTopDUContext indexedTopContext() const {
      if(isDummy())
        return IndexedTopDUContext();
      else
        return IndexedTopDUContext(m_topContext);
    }

    ///The following functions allow storing 2 integers in this object and marking it as a dummy,
    ///which makes the isValid() function always return false for this object, and use the integers
    ///for other purposes
    ///Clears the contained data
    void setIsDummy(bool dummy) {
      if(isDummy() == dummy)
        return;
      if(dummy)
        m_topContext = 1u << 31u;
      else
        m_topContext = 0;
      m_declarationIndex = 0;
    }

    inline bool isDummy() const {
      //We use the second highest bit to mark dummies, because the highest is used for the sign bit of stored
      //integers
      return (bool)(m_topContext & static_cast<uint>(1u << 31u));
    }

    inline QPair<uint, uint> dummyData() const {
      Q_ASSERT(isDummy());
      return qMakePair(m_topContext & (~(1u<<31u)), m_declarationIndex);
    }

    ///Do not call this when this object is valid. The first integer loses one bit of precision.
    void setDummyData(QPair<uint, uint> data) {
      Q_ASSERT(isDummy());

      m_topContext = data.first;
      m_declarationIndex = data.second;
      Q_ASSERT(!isDummy());
      m_topContext |= (1u << 31u); //Mark as dummy
      Q_ASSERT(isDummy());
      Q_ASSERT(dummyData() == data);
    }


  private:
  uint m_topContext;
  uint m_declarationIndex;
};

///Represents a declaration only by its index within the top-context
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedDeclaration {
  public:
    LocalIndexedDeclaration(Declaration* decl = 0);
    LocalIndexedDeclaration(uint declarationIndex);
    //Duchain must be read locked

    Declaration* data(TopDUContext* top) const;

    bool operator==(const LocalIndexedDeclaration& rhs) const {
      return m_declarationIndex == rhs.m_declarationIndex;
    }
    uint hash() const {
      return m_declarationIndex * 23;
    }

    bool isValid() const {
      return m_declarationIndex != 0;
    }

    bool operator<(const LocalIndexedDeclaration& rhs) const {
      return m_declarationIndex < rhs.m_declarationIndex;
    }

    ///Index of the Declaration within the top context
    uint localIndex() const {
      return m_declarationIndex;
    }

    bool isLoaded(TopDUContext* top) const;

  private:
  uint m_declarationIndex;
};

}
Q_DECLARE_METATYPE(KDevelop::IndexedTopDUContext);

#endif // INDEXEDITEMS_H
