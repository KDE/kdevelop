/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "topducontextutils.h"

#include "abstractfunctiondeclaration.h"
#include "declaration.h"

using namespace KDevelop;

///Takes a set of conditions in the constructors, and checks with each call to operator() whether these conditions are fulfilled on the given declaration.
///The import-structure needs to be constructed and locked when this is used
TopDUContext::DeclarationChecker::DeclarationChecker(const TopDUContext* _top, const CursorInRevision& _position,
                                                     const AbstractType::Ptr& _dataType, DUContext::SearchFlags _flags,
                                                     KDevVarLengthArray<IndexedDeclaration>* _createVisibleCache)
    : createVisibleCache(_createVisibleCache)
    , top(_top)
    , topDFunc(_top->d_func())
    , position(_position)
    , dataType(_dataType)
    , flags(_flags)
{
}

bool TopDUContext::DeclarationChecker::operator()(const Declaration* decl) const
{
    if (!decl)
        return false;

    if (top != decl->topContext()) {
        if (( flags& DUContext::OnlyFunctions ) && !dynamic_cast<const AbstractFunctionDeclaration*>(decl))
            return false;

        if (dataType && decl->abstractType()->indexed() != dataType->indexed())
            // The declaration doesn't match the type filter we are applying
            return false;
    } else {
        if (( flags& DUContext::OnlyFunctions ) && !dynamic_cast<const AbstractFunctionDeclaration*>(decl))
            return false;

        if (dataType && decl->abstractType() != dataType)
            // The declaration doesn't match the type filter we are applying
            return false;

        if (decl->range().start >= position)
            if (!decl->context() || decl->context()->type() != DUContext::Class)
                return false; // The declaration is behind the position we're searching from, therefore not accessible
    }
    // Success, this declaration is accessible
    return true;
}
