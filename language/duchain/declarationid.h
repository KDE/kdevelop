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

#ifndef KDEVPLATFORM_DECLARATION_ID_H
#define KDEVPLATFORM_DECLARATION_ID_H

#include "../editor/cursorinrevision.h"

#include "indexeddeclaration.h"
#include "identifier.h"
#include "instantiationinformation.h"
#include <language/util/kdevhash.h>

//krazy:excludeall=dpointer

class QString;

namespace KDevelop {

class Declaration;
class TopDUContext;

/**
 * \short Allows clearly identifying a Declaration.
 *
 * DeclarationId is needed to uniquely address Declarations that are in another top-context,
 * because there may be multiple parsed versions of a file.
 *
 * There are two forms of DeclarationId, one indirect and one direct.  The direct form
 * holds a reference to the Declaration instance, whereas the indirect form stores the qualified
 * identifier and an additional index to disambiguate instances of multiple declarations with the same
 * identifier.
 *
 * Both forms also have a specialization index. It can be used in a language-specific way to pick other
 * versions of the declaration. When the declaration is found, Declaration::specialize() is called on
 * the found declaration with this value, and the returned value is the actually found declaration.
 *
 * \note This only works when the Declaration is in the symbol table.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT DeclarationId {
  public:
    /**
     * Constructor for indirect access to a declaration.  The resulting DeclarationId will not
     * have a direct reference to the Declaration, but will look it up as needed.
     *
     * \param id Identifier for this declaration id.
     * \param additionalId Additional index to disambiguate
     * \param specialization Specialization index (see class documentation).
     */
    explicit DeclarationId(const IndexedQualifiedIdentifier& id = IndexedQualifiedIdentifier(),
                           uint additionalId = 0,
                           const IndexedInstantiationInformation& specialization = IndexedInstantiationInformation());

    /**
     * Constructor for direct access to a declaration.  The resulting DeclarationId will
     * directly reference the Declaration
     *
     * \param decl Declaration to reference.
     * \param specialization Specialization index (see class documentation).
     */
    explicit DeclarationId(const IndexedDeclaration& decl,
                           const IndexedInstantiationInformation& specialization = IndexedInstantiationInformation());

    /**
     * Equality operator.
     *
     * \param rhs declaration identifier to compare.
     * \returns true if equal, otherwise false.
     */
    bool operator==(const DeclarationId& rhs) const {
      if(m_direct != rhs.m_direct)
        return false;

      if(!m_direct)
        return indirect.m_identifier == rhs.indirect.m_identifier && indirect.m_additionalIdentity == rhs.indirect.m_additionalIdentity && m_specialization == rhs.m_specialization;
      else
        return direct == rhs.direct && m_specialization == rhs.m_specialization;
    }

    /**
     * Not equal operator.
     *
     * \param rhs declaration identifier to compare.
     * \returns true if not equal, otherwise false.
     */
    bool operator!=(const DeclarationId& rhs) const {
      return !operator==(rhs);
    }

    /**
     * Determine whether this declaration identifier references a valid declaration.
     */
    bool isValid() const {
      return (m_direct && direct.isValid()) || indirect.m_identifier.isValid();
    }

    /**
     * Hash function for this declaration identifier.
     *
     * \warning This may return different hashes for the same declaration,
     *          depending on whether the id is direct or indirect,
     *          and thus you cannot compare hashes for declaration equality (use operator==() instead)
     */
    uint hash() const {
      if(m_direct)
        return KDevHash() << direct.hash() << m_specialization.index();
      else
        return KDevHash() << indirect.m_identifier.getIndex() << indirect.m_additionalIdentity << m_specialization.index();
    }

    /**
     * Retrieve the declaration, from the perspective of \a context.
     * In order to be retrievable, the declaration must be in the symbol table.
     *
     * \param context Context in which to search for the Declaration.
     * \param instantiateIfRequired Whether the declaration should be instantiated if required
     * \returns the referenced Declaration, or null if none was found.
     * */
    Declaration* getDeclaration(const TopDUContext* context, bool instantiateIfRequired = true) const;

    /**
     * Same as getDeclaration(..), but returns all matching declarations if there are multiple.
     * This also returns found forward-declarations.
     */
    KDevVarLengthArray<Declaration*> getDeclarations(const TopDUContext* context) const;

    /**
     * Set the specialization index (see class documentation).
     *
     * \param specializationIndex the new specialization index.
     */
    void setSpecialization(const IndexedInstantiationInformation& spec);

    /**
     * Retrieve the specialization index (see class documentation).
     *
     * \returns the specialization index.
     */
    IndexedInstantiationInformation specialization() const;

    /**
     * Determine whether this DeclarationId directly references a Declaration by indices,
     * or if it uses identifiers and other data to reference the Declaration.
     *
     * \returns true if direct, false if indirect.
     */
    bool isDirect() const;

    /**
     * Return the qualified identifier for this declaration.
     *
     * \warning This is relatively expensive, and not 100% correct in all cases(actually a top-context would be needed to resolve this correctly),
     *          so avoid using this, except for debugging purposes.
     */
    QualifiedIdentifier qualifiedIdentifier() const;

  private:
    struct Indirect {
      IndexedQualifiedIdentifier m_identifier;
      // Hash from signature, or similar.
      // Used to disambiguate multiple declarations of the same name.
      uint m_additionalIdentity;
    } ;

    //union {
      //An indirect reference to the declaration, which uses the symbol-table for lookup. Should be preferred for all
      //declarations that are in the symbol-table
      Indirect indirect;
      IndexedDeclaration direct;
    //};
    bool m_direct;
    // Can be used in a language-specific way to pick other versions of the declaration.
    // When the declaration is found, pickSpecialization is called on the found declaration
    // with this value, and the returned value is the actually found declaration.
    IndexedInstantiationInformation m_specialization;
};

inline uint qHash(const KDevelop::DeclarationId& id) {
  return id.hash();
}

}

Q_DECLARE_TYPEINFO(KDevelop::DeclarationId, Q_MOVABLE_TYPE);

#endif
