/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CLASSMEMBERDECLARATIONDATA_H
#define KDEVPLATFORM_CLASSMEMBERDECLARATIONDATA_H

#include "declarationdata.h"
#include <language/languageexport.h>

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT ClassMemberDeclarationData
    : public DeclarationData
{
public:
    ClassMemberDeclarationData();
    ClassMemberDeclarationData(const ClassMemberDeclarationData& rhs) = default;

    Declaration::AccessPolicy m_accessPolicy;

    bool m_isStatic : 1;
    bool m_isAuto : 1;
    bool m_isFriend : 1;
    bool m_isRegister : 1;
    bool m_isExtern : 1;
    bool m_isMutable : 1;

    /// Stores bitWidth in bits or a ClassMemberDeclaration::BitWidthSpecialValue.
    int16_t m_bitWidth;

    /// Stores bitOffsetOf in bits or -1 if unknown.
    int64_t m_bitOffsetOf;
};
}

#endif
