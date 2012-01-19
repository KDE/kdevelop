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
#include <QMetaType>

#include "indexedtopducontext.h"

namespace KDevelop {

class Declaration;
class TopDUContext;
class IndexedString;

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
