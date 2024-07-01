/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CLASSMEMBERDECLARATION_H
#define KDEVPLATFORM_CLASSMEMBERDECLARATION_H

#include "declaration.h"

#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

namespace KDevelop {
class ClassMemberDeclarationData;
/**
 * Represents a single class member definition in a definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassMemberDeclaration
    : public Declaration
{
public:
    ClassMemberDeclaration(const ClassMemberDeclaration& rhs);
    ClassMemberDeclaration(const RangeInRevision& range, DUContext* context);
    explicit ClassMemberDeclaration(ClassMemberDeclarationData& dd);
    ~ClassMemberDeclaration() override;

    ClassMemberDeclaration& operator=(const ClassMemberDeclaration& rhs) = delete;

    AccessPolicy accessPolicy() const;
    void setAccessPolicy(AccessPolicy accessPolicy);

    enum StorageSpecifier {
        StaticSpecifier   = 0x1 /**< indicates static member */,
        AutoSpecifier     = 0x2 /**< indicates automatic determination of member access */,
        FriendSpecifier   = 0x4 /**< indicates friend member */,
        ExternSpecifier   = 0x8 /**< indicates external declaration */,
        RegisterSpecifier = 0x10 /**< indicates register */,
        MutableSpecifier  = 0x20/**< indicates a mutable member */
    };
    Q_DECLARE_FLAGS(StorageSpecifiers, StorageSpecifier)

    void setStorageSpecifiers(StorageSpecifiers specifiers);

    bool isStatic() const;
    void setStatic(bool isStatic);

    bool isAuto() const;
    void setAuto(bool isAuto);

    bool isFriend() const;
    void setFriend(bool isFriend);

    bool isRegister() const;
    void setRegister(bool isRegister);

    bool isExtern() const;
    void setExtern(bool isExtern);

    bool isMutable() const;
    void setMutable(bool isMutable);

    /**
     * \returns The offset of the field in bits or -1 if unknown or not applicable.
     */
    int64_t bitOffsetOf() const;

    /**
     * Set the offset to given number of bits. Use -1 to represent unknown offset.
     */
    void setBitOffsetOf(int64_t bitOffsetOf);

    enum BitWidthSpecialValue {
        NotABitField = -1, ///< this member is not a bit-field or a parsing error occurred
        ValueDependentBitWidth = -2 ///< bit width depends on a template parameter
    };

    /**
     * \returns The width in bits or a BitWidthSpecialValue.
     */
    int bitWidth() const;

    /**
     * Set the width to a given number of bits or to a BitWidthSpecialValue.
     */
    void setBitWidth(int bitWidth);

    enum {
        Identity = 9
    };

protected:
    ClassMemberDeclaration(ClassMemberDeclarationData& dd, const RangeInRevision& range);

    DUCHAIN_DECLARE_DATA(ClassMemberDeclaration)

private:
    Declaration* clonePrivate() const override;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ClassMemberDeclaration::StorageSpecifiers)
}

#endif // KDEVPLATFORM_CLASSMEMBERDECLARATION_H
