/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "functiondeclaration.h"
#include <language/duchain/duchainregister.h>

namespace QmlJS {

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

}

DUCHAIN_DEFINE_TYPE(QmlJS::FunctionDeclaration)
