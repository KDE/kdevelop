/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "aliasdeclaration.h"

#include "ducontext.h"
#include "duchainregister.h"
#include "types/delayedtype.h"
#include <editor/rangeinrevision.h>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(AliasDeclaration);

AliasDeclaration::AliasDeclaration(const AliasDeclaration& rhs)
    : ClassMemberDeclaration(*new AliasDeclarationData(*rhs.d_func()))
{
}

AliasDeclaration::AliasDeclaration(const RangeInRevision& range, DUContext* context)
    : ClassMemberDeclaration(*new AliasDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    setKind(Alias);
    if (context)
        setContext(context);
}

AliasDeclaration::AliasDeclaration(AliasDeclarationData& data) : ClassMemberDeclaration(data)
{
}

AliasDeclaration::~AliasDeclaration()
{
}

Declaration* AliasDeclaration::clonePrivate() const
{
    return new AliasDeclaration(*this);
}

QString AliasDeclaration::toString() const
{
    if (aliasedDeclaration().isValid())
        return i18n("Alias %1 as %2", aliasedDeclaration().declaration()->qualifiedIdentifier().toString(),
                    identifier().toString());
    else
        return i18n("Lost alias %1", identifier().toString());
}

void AliasDeclaration::setAliasedDeclaration(const IndexedDeclaration& decl)
{
    d_func_dynamic()->m_aliasedDeclaration = decl;
    Declaration* aliased = decl.data();
    if (aliased)
        Declaration::setAbstractType(aliased->abstractType());
}

IndexedDeclaration AliasDeclaration::aliasedDeclaration() const
{
    return d_func()->m_aliasedDeclaration;
}

void AliasDeclaration::setAbstractType(AbstractType::Ptr type)
{
    Declaration::setAbstractType(type);
}
}
