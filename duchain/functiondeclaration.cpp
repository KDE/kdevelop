/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "functiondeclaration.h"
#include <language/duchain/duchainregister.h>

namespace QmlJS {

REGISTER_DUCHAIN_ITEM(FunctionDeclaration);

FunctionDeclaration::FunctionDeclaration(const FunctionDeclaration& rhs)
: KDevelop::FunctionDeclaration(*new FunctionDeclarationData(*rhs.d_func()))
{
}

FunctionDeclaration::FunctionDeclaration(const KDevelop::RangeInRevision& range,
                                         KDevelop::DUContext* context)
: KDevelop::FunctionDeclaration(*new FunctionDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);

    if (context) {
        setContext(context);
    }
}

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data)
: KDevelop::FunctionDeclaration(data)
{
}

FunctionDeclaration::~FunctionDeclaration()
{
}

KDevelop::DUContext* FunctionDeclaration::prototypeContext() const
{
    return d_func()->m_prototypeContext.context();
}

void FunctionDeclaration::setPrototypeContext(KDevelop::DUContext* context)
{
    DUCHAIN_D_DYNAMIC(FunctionDeclaration);

    d->m_prototypeContext = KDevelop::IndexedDUContext(context);

    // HACK: Also set the internal function context of this function to "context",
    //       so that importing functions work (DUContext::Import::context(), when
    //       given a FunctionDeclaration, returns its internalFunctionContext)
    if (context->type() == KDevelop::DUContext::Function) {
        // NOTE: type != Function when the internal context of an object is assigned
        //       to one of its members. Skipping this hack is not a problem in that
        //       case because one never writes:
        //          o.member = function(){}; var v = new o.member();
        setInternalFunctionContext(context);
    }
}

};