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

#include "hashedstring.h"
#include "identifier.h"
#include "../languageexport.h"
#include <editor/simplecursor.h>

#ifndef DECLARATION_ID_H
#define DECLARATION_ID_H

//krazy:excludeall=dpointer

namespace KDevelop {

class Declaration;
class DUContext;
  
/**
 * Allows clearly identifying a Declaration.
 *
 * DeclarationId is needed to uniquely address Declarations that are in another top-context,
 * because there may be multiple parsed versions of a file.
 *
 * For performance-reasons, we don't want d-pointers here. Above that, we must think about ways
 * to make this more efficient, since we nearly create an instance of DeclarationId for each Declaration we have.
 * */

class KDEVPLATFORMLANGUAGE_EXPORT DeclarationId {
  public:
    DeclarationId(const HashedString& url = HashedString(), const QualifiedIdentifier& id = QualifiedIdentifier(), uint additionalId = 0);
    
    bool operator==(const DeclarationId& rhs) const {
      return m_url == rhs.m_url && m_identifier == rhs.m_identifier && m_additionalIdentity == rhs.m_additionalIdentity;
    }

    uint hash() const {
      return m_url.hash() * 37 + m_identifier.hash() * 13 + m_additionalIdentity;
    }

    QualifiedIdentifier identifier() const;
    HashedString url() const;
    uint additionalIdentity() const;

    /**
     * Tries to retrieve the declaration, from the perspective of @param context
     * Never returns a declaration that has isDefinition(..) set.
     * */
    Declaration* getDeclaration(DUContext* context) const;
    
  private:
    HashedString m_url;
    QualifiedIdentifier m_identifier;
    uint m_additionalIdentity; //Hash from signature, or similar.

};

inline uint qHash(const KDevelop::DeclarationId& id) {
  return id.hash();
}

}

#endif
