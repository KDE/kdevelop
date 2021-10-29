/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_FUNCTIONDECLARATION_H
#define KDEVPLATFORM_FUNCTIONDECLARATION_H

#include "declaration.h"
#include "abstractfunctiondeclaration.h"
#include "declarationdata.h"

namespace KDevelop {
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(FunctionDeclarationData, m_defaultParameters, IndexedString)

class KDEVPLATFORMLANGUAGE_EXPORT FunctionDeclarationData
    : public DeclarationData
    , public AbstractFunctionDeclarationData
{
public:
    FunctionDeclarationData()
    {
        initializeAppendedLists();
    }
    FunctionDeclarationData(const FunctionDeclarationData& rhs)
        : DeclarationData(rhs)
        , AbstractFunctionDeclarationData(rhs)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
    }
    ~FunctionDeclarationData()
    {
        freeAppendedLists();
    }
    FunctionDeclarationData& operator=(const FunctionDeclarationData& rhs) = delete;

    START_APPENDED_LISTS_BASE(FunctionDeclarationData, DeclarationData);
    APPENDED_LIST_FIRST(FunctionDeclarationData, IndexedString, m_defaultParameters);
    END_APPENDED_LISTS(FunctionDeclarationData, m_defaultParameters);
};
/**
 * Represents a single variable definition in a definition-use chain.
 */
using FunctionDeclarationBase = MergeAbstractFunctionDeclaration<Declaration, FunctionDeclarationData>;
class KDEVPLATFORMLANGUAGE_EXPORT FunctionDeclaration
    : public FunctionDeclarationBase
{
public:
    FunctionDeclaration(const FunctionDeclaration& rhs);
    FunctionDeclaration(const RangeInRevision& range, DUContext* context);
    explicit FunctionDeclaration(FunctionDeclarationData& data);
    FunctionDeclaration(FunctionDeclarationData& data, const KDevelop::RangeInRevision&);
    ~FunctionDeclaration() override;

    FunctionDeclaration& operator=(const FunctionDeclaration& rhs) = delete;

    void setAbstractType(AbstractType::Ptr type) override;

    QString toString() const override;

    bool isFunctionDeclaration() const override;

    uint additionalIdentity() const override;

    const IndexedString* defaultParameters() const override;
    unsigned int defaultParametersSize() const override;
    void addDefaultParameter(const IndexedString& str) override;
    void clearDefaultParameters() override;

    enum {
        Identity = 12
    };

    using Base = Declaration;

private:
    Declaration* clonePrivate() const override;
    DUCHAIN_DECLARE_DATA(FunctionDeclaration)
};
}

#endif // KDEVPLATFORM_FUNCTIONDECLARATION_H
