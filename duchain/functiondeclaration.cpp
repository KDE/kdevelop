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

void FunctionDeclaration::setPrototypeContext(KDevelop::DUContext* context, bool own)
{
    DUCHAIN_D_DYNAMIC(FunctionDeclaration);

    d_func_dynamic()->m_prototypeContext = KDevelop::IndexedDUContext(context);

    if (own) {
        // DUContext::setOwner(decl) calls decl->setInternalContext, which is not
        // wanted here. Save and restore m_internalContext
        KDevelop::IndexedDUContext saveInternalContext = d->m_internalContext;
        context->setOwner(this);
        d->m_internalContext = saveInternalContext;
    }
}

};