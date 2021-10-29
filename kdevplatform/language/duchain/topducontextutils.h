/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TOPDUCONTEXTUTILS_H
#define KDEVPLATFORM_TOPDUCONTEXTUTILS_H

#include "topducontext.h"

namespace KDevelop {
/// \todo move data to private d pointer classes
struct KDEVPLATFORMLANGUAGE_EXPORT TopDUContext::DeclarationChecker
{
    DeclarationChecker(const TopDUContext* _top, const CursorInRevision& _position, const AbstractType::Ptr& _dataType,
                       DUContext::SearchFlags _flags,
                       KDevVarLengthArray<IndexedDeclaration>* _createVisibleCache = nullptr);
    bool operator()(const Declaration* dec) const;

    mutable KDevVarLengthArray<IndexedDeclaration>* createVisibleCache;
    const TopDUContext* top;
    const TopDUContextData* topDFunc;
    const CursorInRevision& position;
    const AbstractType::Ptr dataType;
    DUContext::SearchFlags flags;
};
}

#endif // KDEVPLATFORM_TOPDUCONTEXTUTILS_H
