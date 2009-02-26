/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef INDEXEDTYPE_H
#define INDEXEDTYPE_H

#include "abstracttype.h"

namespace KDevelop
{

  /**
 * \short Indexed type pointer.
 *
 * IndexedType is a class which references a type by an index.
 * This way the type can be stored to disk.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedType {
  public:
    /// Constructor.
    explicit IndexedType(uint index = 0) : m_index(index) {
    }

    /**
     * Access the type.
     *
     * \returns the type pointer, or null if this index is invalid.
     */
    AbstractType::Ptr type() const;

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

    /// Access the type's hash value. \returns the hash value.
    uint hash() const {
      return m_index>>1;
    }

    /// Access the type's index. \returns the index.
    uint index() const {
      return m_index;
    }

  private:
    uint m_index;
};

}

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
