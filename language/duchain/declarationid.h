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

#ifndef DECLARATION_ID_H
#define DECLARATION_ID_H

#include "language/editor/hashedstring.h"
#include "language/editor/simplecursor.h"

#include "language/duchain/identifier.h"
#include "declaration.h"

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
    DeclarationId(const IndexedQualifiedIdentifier& id = IndexedQualifiedIdentifier(), uint additionalId = 0, uint specialization = 0);
    DeclarationId(const IndexedDeclaration& decl, uint specialization = 0);
    
    bool operator==(const DeclarationId& rhs) const {
      if(m_direct != rhs.m_direct)
        return false;
      
      if(!m_direct)
        return indirect.m_identifier == rhs.indirect.m_identifier && indirect.m_additionalIdentity == rhs.indirect.m_additionalIdentity && m_specialization == rhs.m_specialization;
      else
        return direct == rhs.direct && m_specialization == rhs.m_specialization;
    }

    ///Warning: This may return different hashes for the same declaration, depending on whether the id is direct or indirect
    uint hash() const {
      if(m_direct)
        return direct.hash() + m_specialization * 101;
      else
        return indirect.m_identifier.index * 13 + indirect.m_additionalIdentity + m_specialization * 101;
    }

//     IndexedQualifiedIdentifier identifier() const;
//     uint additionalIdentity() const;

    /**
     * Tries to retrieve the declaration, from the perspective of @param context
     * In order to be retrievable, the declaration must be in the symbol table
     * */
    Declaration* getDeclaration(TopDUContext* context) const;
    
    void setSpecialization(uint);
    uint specialization() const;
    
    //Whether this DeclarationId directly references a declaration by indices. If false, it uses identifiers and other data.
    bool isDirect() const;
    
  private:
    struct Indirect {
      IndexedQualifiedIdentifier m_identifier;
      uint m_additionalIdentity; //Hash from signature, or similar. Used to disambiguate multiple declarations of the same name.
    } ;
    
    //union {
      //An indirect reference to the declaration, which uses the symbol-table for lookup. Should be preferred for all
      //declarations that are in the symbol-table
      Indirect indirect;
      IndexedDeclaration direct;
    //};
    bool m_direct;
    uint m_specialization; //Can be used in a language-specific way to pick other versions of the declaration.
                           //When the declaration is found, pickSpecialization is called on the found declaration with this value, and
                           //the returned value is the actually found declaration.
};

inline uint qHash(const KDevelop::DeclarationId& id) {
  return id.hash();
}

}

#endif
