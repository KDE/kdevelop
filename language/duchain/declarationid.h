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

#include "language/editor/hashedstring.h"
#include "language/editor/simplecursor.h"

#include "language/duchain/identifier.h"

#ifndef DECLARATION_ID_H
#define DECLARATION_ID_H

//krazy:excludeall=dpointer

namespace KDevelop {

class Declaration;
class TopDUContext;
  
/**
 * Allows clearly identifying a Declaration.
 *
 * DeclarationId is needed to uniquely address Declarations that are in another top-context,
 * because there may be multiple parsed versions of a file.
 *
 * 
 * This only works when the declaration is in the symbol table
 * */

class KDEVPLATFORMLANGUAGE_EXPORT DeclarationId {
  public:
    DeclarationId(const IndexedQualifiedIdentifier& id = IndexedQualifiedIdentifier(), uint additionalId = 0);
    
    bool operator==(const DeclarationId& rhs) const {
      return m_identifier == rhs.m_identifier && m_additionalIdentity == rhs.m_additionalIdentity;
    }

    uint hash() const {
      return m_identifier.index * 13 + m_additionalIdentity;
    }

    IndexedQualifiedIdentifier identifier() const;
    uint additionalIdentity() const;

    /**
     * Tries to retrieve the declaration, from the perspective of @param context
     * In order to be retrievable, the declaration must be in the symbol table
     * */
    Declaration* getDeclaration(TopDUContext* context) const;
    
  private:
    IndexedQualifiedIdentifier m_identifier;
    uint m_additionalIdentity; //Hash from signature, or similar.

};

inline uint qHash(const KDevelop::DeclarationId& id) {
  return id.hash();
}

}

#endif
