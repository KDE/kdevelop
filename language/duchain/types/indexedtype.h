/* This file is part of KDevelop
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_INDEXEDTYPE_H
#define KDEVPLATFORM_INDEXEDTYPE_H

#include "abstracttype.h"
#include <language/duchain/referencecounting.h>

namespace KDevelop
{

  /**
 * \short Indexed type pointer.
 *
 * IndexedType is a class which references a type by an index.
 * This way the type can be stored to disk.
 *
 * This class does "disk reference counting"
 * @warning Do not use this after QCoreApplication::aboutToQuit() has been emitted, items that are not disk-referenced will be invalid at that point
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedType : public ReferenceCountManager {
  public:
    /// Constructor.
    IndexedType(const IndexedType& rhs);
    explicit IndexedType(uint index = 0);
    
    ~IndexedType();
    
    IndexedType& operator=(const IndexedType& rhs);

    /**
     * Access the type.
     *
     * \returns the type pointer, or null if this index is invalid.
     */
    AbstractType::Ptr abstractType() const;

    /**
     * Access the type, dynamically casted to the type you provide.
     *
     * \returns the type pointer, or null if this index is invalid.
     */
    template<class T>
    TypePtr<T> type() const { return TypePtr<T>::dynamicCast(abstractType()); }

    /// Determine if the type is valid. \returns true if valid, otherwise false.
    bool isValid() const {
      return (bool)m_index;
    }

    /// \copydoc
    operator bool() const {
      return (bool)m_index;
    }

    /// Equivalence operator. \param rhs indexed type to compare. \returns true if equal (or both invalid), otherwise false.
    bool operator==(const IndexedType& rhs) const {
      return m_index == rhs.m_index;
    }

    /// Not equal operator. \param rhs indexed type to compare. \returns true if types are not the same, otherwise false.
    bool operator!=(const IndexedType& rhs) const {
      return m_index != rhs.m_index;
    }

    /// Less than operator, \param rhs indexed type to compare.
    /// \returns true if integral index value of this type is lower than the index of \p rhs.
    bool operator<(const IndexedType& rhs) const {
      return m_index < rhs.m_index;
    }

    /// Access the type's hash value. \returns the hash value.
    uint hash() const {
      return m_index>>1;
    }

    /// Access the type's index. \returns the index.
    uint index() const {
      return m_index;
    }

  private:
    ///This class must _never_ hold more than one unsigned integer
    uint m_index;
};

}

Q_DECLARE_TYPEINFO(KDevelop::IndexedType, Q_MOVABLE_TYPE);

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
