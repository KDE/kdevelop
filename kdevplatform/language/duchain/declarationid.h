/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DECLARATION_ID_H
#define KDEVPLATFORM_DECLARATION_ID_H

#include "indexeddeclaration.h"
#include "identifier.h"
#include "instantiationinformation.h"
#include <language/util/kdevhash.h>

//krazy:excludeall=dpointer

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
class KDEVPLATFORMLANGUAGE_EXPORT DeclarationId
{
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

    DeclarationId(const DeclarationId& rhs);

    ~DeclarationId();

    DeclarationId& operator=(const DeclarationId& rhs);

    /**
     * Equality operator.
     *
     * \param rhs declaration identifier to compare.
     * \returns true if equal, otherwise false.
     */
    bool operator==(const DeclarationId& rhs) const
    {
        if (m_isDirect != rhs.m_isDirect)
            return false;

        if (!m_isDirect)
            return m_indirectData.identifier == rhs.m_indirectData.identifier
                   && m_indirectData.additionalIdentity == rhs.m_indirectData.additionalIdentity
                   && m_specialization == rhs.m_specialization;
        else
            return m_directData == rhs.m_directData && m_specialization == rhs.m_specialization;
    }

    /**
     * Not equal operator.
     *
     * \param rhs declaration identifier to compare.
     * \returns true if not equal, otherwise false.
     */
    bool operator!=(const DeclarationId& rhs) const
    {
        return !operator==(rhs);
    }

    /**
     * Determine whether this declaration identifier references a valid declaration.
     */
    bool isValid() const
    {
        return (m_isDirect && m_directData.isValid()) || m_indirectData.identifier.isValid();
    }

    /**
     * Hash function for this declaration identifier.
     *
     * \warning This may return different hashes for the same declaration,
     *          depending on whether the id is direct or indirect,
     *          and thus you cannot compare hashes for declaration equality (use operator==() instead)
     */
    uint hash() const
    {
        if (m_isDirect)
            return KDevHash() << m_directData.hash() << m_specialization.index();
        else
            return KDevHash() << m_indirectData.identifier.index() << m_indirectData.additionalIdentity <<
                   m_specialization.index();
    }

    /**
     * Retrieve the declaration, from the perspective of \a context.
     * In order to be retrievable, the declaration must be in the symbol table.
     *
     * \param context Context in which to search for the Declaration.
     * \param instantiateIfRequired Whether the declaration should be instantiated if required
     * \returns the referenced Declaration, or null if none was found.
     * */
    Declaration* declaration(const TopDUContext* context, bool instantiateIfRequired = true) const;

    /**
     * Same as declaration(..), but returns all matching declarations if there are multiple.
     * This also returns found forward-declarations.
     */
    KDevVarLengthArray<Declaration*> declarations(const TopDUContext* context) const;

    /**
     * Set the specialization index (see class documentation).
     *
     * \param spec the new specialization index.
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
    /// An indirect reference to the declaration, which uses the symbol-table for lookup. Should be preferred for all
    /// declarations that are in the symbol-table
    struct Indirect
    {
        IndexedQualifiedIdentifier identifier;
        /// Hash from signature, or similar. Used to disambiguate multiple declarations of the same name.
        uint additionalIdentity;

        Indirect& operator=(const Indirect& rhs) = default;
    };

    union {
        Indirect m_indirectData;
        IndexedDeclaration m_directData;
    };
    bool m_isDirect;

    // Can be used in a language-specific way to pick other versions of the declaration.
    // When the declaration is found, pickSpecialization is called on the found declaration
    // with this value, and the returned value is the actually found declaration.
    IndexedInstantiationInformation m_specialization;
};

inline size_t qHash(const KDevelop::DeclarationId& id)
{
    return id.hash();
}
}

Q_DECLARE_TYPEINFO(KDevelop::DeclarationId, Q_MOVABLE_TYPE);

#endif
