/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "identifiedtype.h"
#include "../declaration.h"
#include "../duchainpointer.h"
#include "../declarationid.h"
#include <debug.h>

namespace KDevelop {
IdentifiedType::~IdentifiedType()
{
}

void IdentifiedType::clear()
{
    idData()->m_id = DeclarationId();
}

bool IdentifiedType::equals(const IdentifiedType* rhs) const
{
    bool ret = false;
    if (idData()->m_id == rhs->idData()->m_id)
        ret = true;

    //qCDebug(LANGUAGE) << this << rhs << true;
    return ret;
}

// QualifiedIdentifier IdentifiedType::identifier() const
// {
//   return idData()->m_id ? idData()->m_iidData()->qualifiedIdentifier() : QualifiedIdentifier();
// }

QualifiedIdentifier IdentifiedType::qualifiedIdentifier() const
{
    return idData()->m_id.qualifiedIdentifier();
}

size_t IdentifiedType::hash() const
{
    return idData()->m_id.hash();
}

DeclarationId IdentifiedType::declarationId() const
{
    return idData()->m_id;
}

void IdentifiedType::setDeclarationId(const DeclarationId& id)
{
    idData()->m_id = id;
}

Declaration* IdentifiedType::declaration(const TopDUContext* top) const
{
    return idData()->m_id.declaration(top);
}

KDevelop::DUContext* IdentifiedType::internalContext(const KDevelop::TopDUContext* top) const
{
    Declaration* decl = declaration(top);
    if (decl)
        return decl->internalContext();
    else
        return nullptr;
}

void IdentifiedType::setDeclaration(Declaration* declaration)
{
    if (declaration)
        idData()->m_id = declaration->id();
    else
        idData()->m_id = DeclarationId();
}

// QString IdentifiedType::idMangled() const
// {
//   return identifier().mangled();
// }
}
