/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_FORWARDDECLARATION_H
#define KDEVPLATFORM_FORWARDDECLARATION_H

#include "declaration.h"
#include "declarationdata.h"

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT ForwardDeclarationData
    : public DeclarationData
{
public:
    ForwardDeclarationData()
    {
    }
    ForwardDeclarationData(const ForwardDeclarationData& rhs)
        : DeclarationData(rhs)
    {
    }
    ~ForwardDeclarationData() = default;
    ForwardDeclarationData& operator=(const ForwardDeclarationData& rhs) = delete;
};
/**
 * Represents a forward declaration
 */
class KDEVPLATFORMLANGUAGE_EXPORT ForwardDeclaration
    : public Declaration
{
public:
    /**
     * Constructor.
     *
     * If \a context is in the symbol table, the declaration will automatically be added into the symbol table.
     *
     * \param range range of the alias declaration's identifier
     * \param context context in which this declaration occurred
     * */
    ForwardDeclaration(const RangeInRevision& range, DUContext* context);
    explicit ForwardDeclaration(ForwardDeclarationData& data);

    ///Copy-constructor for cloning
    ForwardDeclaration(const ForwardDeclaration& rhs);

    /// Destructor.
    ~ForwardDeclaration() override;

    ForwardDeclaration& operator=(const ForwardDeclaration& rhs) = delete;

    bool isForwardDeclaration() const override;

    /**
     * Resolved the forward-declaration using the given import-trace.
     * The topcontext is needed for correct functionality, and may only be
     * zero when the declaration is resolved starting from the top-context
     * the forward-declaration is contained in.
     *
     * If this forward-declaration has a type assigned that is not derived from ForwardDeclarationType,
     * and that is derived from IdentifiedType, the declaration of that type is returned here.
     * */
    virtual Declaration* resolve(const TopDUContext* topContext) const;

    DUContext* logicalInternalContext(const TopDUContext* topContext) const override;

    QString toString() const override;

    enum {
        Identity = 10
    };

    using BaseClass = Declaration;

private:
    Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(ForwardDeclaration)
};
}

#endif // KDEVPLATFORM_DECLARATION_H
