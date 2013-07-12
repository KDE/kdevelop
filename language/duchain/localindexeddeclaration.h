/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_LOCALINDEXEDDECLARATION_H
#define KDEVPLATFORM_LOCALINDEXEDDECLARATION_H

#include "../languageexport.h"

namespace KDevelop {

class TopDUContext;
class Declaration;

/**
 * Represents a declaration only by its index within the top-context
 */
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedDeclaration
{
  public:
    LocalIndexedDeclaration(Declaration* decl = 0);
    LocalIndexedDeclaration(uint declarationIndex);

    /**
     * \note Duchain must be read locked
     */
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

    /**
     * Index of the Declaration within the top context
     */
    uint localIndex() const {
      return m_declarationIndex;
    }

    bool isLoaded(TopDUContext* top) const;

  private:
    uint m_declarationIndex;
};

}

Q_DECLARE_TYPEINFO(KDevelop::LocalIndexedDeclaration, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_LOCALINDEXEDDECLARATION_H
