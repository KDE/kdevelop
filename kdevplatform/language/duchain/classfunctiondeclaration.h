/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CLASSFUNCTIONDECLARATION_H
#define KDEVPLATFORM_CLASSFUNCTIONDECLARATION_H

#include "classmemberdeclaration.h"
#include "abstractfunctiondeclaration.h"
#include "classmemberdeclarationdata.h"

namespace KDevelop {
enum ClassFunctionFlag
{
    FunctionFlagNormal = 0,
    FunctionSignalFlag = 1 <<  1,
    FunctionSlotFlag = 1 << 2,
    AbstractFunctionFlag = 1 << 3,
    FinalFunctionFlag = 1 << 4
};
Q_DECLARE_FLAGS(ClassFunctionFlags, ClassFunctionFlag)

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(ClassFunctionDeclarationData, m_defaultParameters, IndexedString)

class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclarationData
    : public ClassMemberDeclarationData
    , public AbstractFunctionDeclarationData
{
public:
    ClassFunctionDeclarationData()
    {
        initializeAppendedLists();
        m_functionFlags = FunctionFlagNormal;
    }
    ClassFunctionDeclarationData(const ClassFunctionDeclarationData& rhs)
        : ClassMemberDeclarationData(rhs)
        , AbstractFunctionDeclarationData(rhs)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
        m_functionFlags = rhs.m_functionFlags;
    }
    ~ClassFunctionDeclarationData()
    {
        freeAppendedLists();
    }
    ClassFunctionDeclarationData& operator=(const ClassFunctionDeclarationData& rhs) = delete;
    ClassFunctionFlags m_functionFlags;
    START_APPENDED_LISTS_BASE(ClassFunctionDeclarationData, ClassMemberDeclarationData);
    APPENDED_LIST_FIRST(ClassFunctionDeclarationData, IndexedString, m_defaultParameters);
    END_APPENDED_LISTS(ClassFunctionDeclarationData, m_defaultParameters);
};
/**
 * Represents a single variable definition in a definition-use chain.
 */
using ClassFunctionDeclarationBase = MergeAbstractFunctionDeclaration<ClassMemberDeclaration, ClassFunctionDeclarationData>;
class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclaration
    : public ClassFunctionDeclarationBase
{
public:
    ClassFunctionDeclaration(const RangeInRevision& range, DUContext* context);
    ClassFunctionDeclaration(ClassFunctionDeclarationData& data, const RangeInRevision& range, DUContext* context);
    explicit ClassFunctionDeclaration(ClassFunctionDeclarationData& data);
    ~ClassFunctionDeclaration() override;

    ///Whether this function is a signal, for example a C++ Qt signal
    bool isSignal() const;
    void setIsSignal(bool);

    ///Whether this function is a slot, for example a C++ Qt slot
    bool isSlot() const;
    void setIsSlot(bool);

    ///Whether this function is abstract
    bool isAbstract() const;
    void setIsAbstract(bool);

    ///Whether this function is final
    bool isFinal() const;
    void setIsFinal(bool);

    virtual bool isConstructor() const;
    virtual bool isDestructor() const;

    bool isConversionFunction() const;

    bool isFunctionDeclaration() const override;

    QString toString() const override;

    void setAbstractType(AbstractType::Ptr type) override;

    Declaration* clonePrivate() const override;

    uint additionalIdentity() const override;

    const IndexedString* defaultParameters() const override;
    unsigned int defaultParametersSize() const override;
    void addDefaultParameter(const IndexedString& str) override;
    void clearDefaultParameters() override;

    enum {
        Identity = 14
    };

protected:
    ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs);

private:
    DUCHAIN_DECLARE_DATA(ClassFunctionDeclaration)
};
}

#endif // KDEVPLATFORM_CLASSFUNCTIONDECLARATION_H
