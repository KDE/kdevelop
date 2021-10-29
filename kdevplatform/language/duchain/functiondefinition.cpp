/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "functiondefinition.h"
#include "duchainregister.h"
#include "definitions.h"

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(FunctionDefinition);

FunctionDefinition::FunctionDefinition(FunctionDefinitionData& data) : FunctionDeclaration(data)
{
}

FunctionDefinition::FunctionDefinition(const RangeInRevision& range, DUContext* context)
    : FunctionDeclaration(*new FunctionDefinitionData, range)
{
    d_func_dynamic()->setClassId(this);
    setDeclarationIsDefinition(true);
    if (context)
        setContext(context);
}

FunctionDefinition::FunctionDefinition(const FunctionDefinition& rhs) : FunctionDeclaration(*new FunctionDefinitionData(
            *rhs.d_func()))
{
}

FunctionDefinition::~FunctionDefinition()
{
    if (!topContext()->isOnDisk())
        DUChain::definitions()->removeDefinition(d_func()->m_declaration, this);
}

Declaration* FunctionDefinition::declaration(const TopDUContext* topContext) const
{
    ENSURE_CAN_READ

    const KDevVarLengthArray<Declaration*> declarations = d_func()->m_declaration.declarations(
        topContext ? topContext : this->topContext());

    for (Declaration* decl : declarations) {
        if (!dynamic_cast<FunctionDefinition*>(decl))
            return decl;
    }

    return nullptr;
}

bool FunctionDefinition::hasDeclaration() const
{
    return d_func()->m_declaration.isValid();
}

void FunctionDefinition::setDeclaration(Declaration* declaration)
{
    ENSURE_CAN_WRITE

    if (declaration) {
        DUChain::definitions()->addDefinition(declaration->id(), this);
        d_func_dynamic()->m_declaration = declaration->id();
    } else {
        if (d_func()->m_declaration.isValid()) {
            DUChain::definitions()->removeDefinition(d_func()->m_declaration, this);
            d_func_dynamic()->m_declaration = DeclarationId();
        }
    }
}

Declaration* FunctionDefinition::definition(const Declaration* decl)
{
    ENSURE_CHAIN_READ_LOCKED
    if (!decl) {
        return nullptr;
    }

    if (decl->isFunctionDeclaration() && decl->isDefinition()) {
        return const_cast<Declaration*>(decl);
    }

    const KDevVarLengthArray<IndexedDeclaration> allDefinitions = DUChain::definitions()->definitions(decl->id());
    for (const IndexedDeclaration decl : allDefinitions) {
        if (decl.data()) ///@todo Find better ways of deciding which definition to use
            return decl.data();
    }

    return nullptr;
}

Declaration* FunctionDefinition::clonePrivate() const
{
    return new FunctionDefinition(*new FunctionDefinitionData(*d_func()));
}
}
