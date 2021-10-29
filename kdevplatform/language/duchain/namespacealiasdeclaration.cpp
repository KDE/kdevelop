/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "namespacealiasdeclaration.h"

#include "ducontext.h"
#include "duchainregister.h"
#include "persistentsymboltable.h"

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(NamespaceAliasDeclaration);

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs)
    : Declaration(*new NamespaceAliasDeclarationData(*rhs.d_func()))
{
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const RangeInRevision& range, DUContext* context)
    : Declaration(*new NamespaceAliasDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    setKind(NamespaceAlias);
    if (context)
        setContext(context);
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(NamespaceAliasDeclarationData& data) : Declaration(data)
{
}

QualifiedIdentifier NamespaceAliasDeclaration::importIdentifier() const
{
    return d_func()->m_importIdentifier.identifier();
}

void NamespaceAliasDeclaration::setImportIdentifier(const QualifiedIdentifier& id)
{
    Q_ASSERT(!id.explicitlyGlobal());
    d_func_dynamic()->m_importIdentifier = id;
}

NamespaceAliasDeclaration::~NamespaceAliasDeclaration()
{
    if (persistentlyDestroying() && d_func()->m_inSymbolTable)
        unregisterAliasIdentifier();
}

void NamespaceAliasDeclaration::setInSymbolTable(bool inSymbolTable)
{
    if (d_func()->m_inSymbolTable && !inSymbolTable) {
        unregisterAliasIdentifier();
    } else if (!d_func()->m_inSymbolTable && inSymbolTable) {
        registerAliasIdentifier();
    }
    KDevelop::Declaration::setInSymbolTable(inSymbolTable);
}

void NamespaceAliasDeclaration::unregisterAliasIdentifier()
{
    if (indexedIdentifier() != globalIndexedImportIdentifier()) {
        QualifiedIdentifier aliasId = qualifiedIdentifier();
        aliasId.push(globalIndexedAliasIdentifier());
        KDevelop::PersistentSymbolTable::self().removeDeclaration(aliasId, this);
    }
}

void NamespaceAliasDeclaration::registerAliasIdentifier()
{
    if (indexedIdentifier() != globalIndexedImportIdentifier()) {
        QualifiedIdentifier aliasId = qualifiedIdentifier();
        aliasId.push(globalIndexedAliasIdentifier());
        KDevelop::PersistentSymbolTable::self().addDeclaration(aliasId, this);
    }
}

Declaration* NamespaceAliasDeclaration::clonePrivate() const
{
    return new NamespaceAliasDeclaration(*this);
}

void NamespaceAliasDeclaration::setAbstractType(AbstractType::Ptr type)
{
    //A namespace-import does not have a type, so ignore any set type
    Q_UNUSED(type);
}

QString NamespaceAliasDeclaration::toString() const
{
    DUCHAIN_D(NamespaceAliasDeclaration);
    if (indexedIdentifier() != globalIndexedImportIdentifier())
        return i18n("Import %1 as %2", d->m_importIdentifier.identifier().toString(), identifier().toString());
    else
        return i18n("Import %1", d->m_importIdentifier.identifier().toString());
}
}
