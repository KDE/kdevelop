/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ALIASDECLARATION_H
#define KDEVPLATFORM_ALIASDECLARATION_H

#include "classmemberdeclaration.h"
#include "classmemberdeclarationdata.h"
#include "duchainpointer.h"
#include "declarationdata.h"

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT AliasDeclarationData
    : public ClassMemberDeclarationData
{
public:
    AliasDeclarationData() {}
    AliasDeclarationData(const AliasDeclarationData& rhs)
        : ClassMemberDeclarationData(rhs)
        , m_aliasedDeclaration(rhs.m_aliasedDeclaration)
    {
    }
    ~AliasDeclarationData() = default;
    AliasDeclarationData& operator=(const AliasDeclarationData& rhs) = delete;

    IndexedDeclaration m_aliasedDeclaration;
};
/**
 * An alias declaration maps one declaration to another.
 * While searching in the duchain, an AliasDeclaration is transparently
 * replaced by its aliased declaration.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AliasDeclaration
    : public ClassMemberDeclaration
{
public:
    /// Copy constructor \param rhs declaration to copy
    AliasDeclaration(const AliasDeclaration& rhs);
    /**
     * Constructs an AliasDeclaration. The default value for isNamespaceAlias is true.
     *
     * \param range range of the alias declaration's identifier
     * \param context context in which this declaration occurred
     */
    AliasDeclaration(const RangeInRevision& range, DUContext* context);

    explicit AliasDeclaration(AliasDeclarationData& data);
    /// Destructor
    ~AliasDeclaration() override;

    AliasDeclaration& operator=(const AliasDeclaration& rhs) = delete;

    /**
     * An AliasDeclaration cannot have a type, so setAbstractType does nothing here.
     *
     * \param type ignored type
     */
    void setAbstractType(AbstractType::Ptr type) override;

    /**
     * Set the declaration that is aliased by this declaration.
     *
     * \param decl the declaration that this declaration references
     */
    void setAliasedDeclaration(const IndexedDeclaration& decl);

    /**
     * Access the declaration that is aliased by this declaration.
     *
     * \returns the aliased declaration
     */
    IndexedDeclaration aliasedDeclaration() const;

    QString toString() const override;

    enum {
        Identity = 6
    };

private:
    Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(AliasDeclaration)
};
}

#endif // KDEVPLATFORM_FUNCTIONDECLARATION_H
