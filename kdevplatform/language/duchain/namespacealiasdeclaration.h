/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_NAMESPACEALIASDECLARATION_H
#define KDEVPLATFORM_NAMESPACEALIASDECLARATION_H

#include "declaration.h"
#include "declarationdata.h"

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT NamespaceAliasDeclarationData
    : public DeclarationData
{
public:
    NamespaceAliasDeclarationData() {}
    NamespaceAliasDeclarationData(const NamespaceAliasDeclarationData& rhs)
        : DeclarationData(rhs)
        , m_importIdentifier(rhs.m_importIdentifier)
    {
    }
    ~NamespaceAliasDeclarationData() = default;
    NamespaceAliasDeclarationData& operator=(const NamespaceAliasDeclarationData& rhs) = delete;
    IndexedQualifiedIdentifier m_importIdentifier; //The identifier that was imported
};
/**
 * A class which represents a "using namespace" statement, or a "namespace A = B" statement.
 *
 * This class is used by the duchain search process to transparently transform the search according to
 * namespace aliases and namespace imports.
 *
 * A namespace import declaration must have an identifier that equals globalImportIdentifier.
 *
 * If the identifier of the declaration does not equal globalImportIdentifier, then the declaration
 * represents a namespace alias, where the name of the alias equals the declaration. In that case,
 * the declaration is additionally added to the persistent symbol table with its real scope and globalAliasIdentifer
 * appended, to allow an efficient lookup.
 */
class KDEVPLATFORMLANGUAGE_EXPORT NamespaceAliasDeclaration
    : public Declaration
{
public:
    NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs);
    NamespaceAliasDeclaration(const RangeInRevision& range, DUContext* context);
    explicit NamespaceAliasDeclaration(NamespaceAliasDeclarationData& data);

    ~NamespaceAliasDeclaration() override;

    NamespaceAliasDeclaration& operator=(const NamespaceAliasDeclaration& rhs) = delete;

    ///A NamespaceAliasDeclaration cannot have a type, so setAbstractType does nothing here.
    void setAbstractType(AbstractType::Ptr type) override;

    /**The identifier that was imported.*/
    QualifiedIdentifier importIdentifier() const;
    /**
     * The identifier must be absolute (Resolve it before setting it!)
     * Although the identifier is global, the explicitlyGlobal() member must not be set
     */
    void setImportIdentifier(const QualifiedIdentifier& id);

    void setInSymbolTable(bool inSymbolTable) override;

    enum {
        Identity = 13
    };

    using BaseClass = Declaration;

    QString toString() const override;

private:
    void unregisterAliasIdentifier();
    void registerAliasIdentifier();
    Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(NamespaceAliasDeclaration)
};
}

#endif // KDEVPLATFORM_NAMESPACEALIASDECLARATION_H
