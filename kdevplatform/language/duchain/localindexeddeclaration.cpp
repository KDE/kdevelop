/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "localindexeddeclaration.h"

#include "declaration.h"
#include "topducontextdynamicdata.h"
#include "topducontext.h"

using namespace KDevelop;

LocalIndexedDeclaration::LocalIndexedDeclaration(Declaration* decl)
    : m_declarationIndex(decl ? decl->m_indexInTopContext : 0)
{
}

LocalIndexedDeclaration::LocalIndexedDeclaration(uint declarationIndex)
    : m_declarationIndex(declarationIndex)
{
}

Declaration* LocalIndexedDeclaration::data(TopDUContext* top) const
{
    if (!m_declarationIndex)
        return nullptr;
    Q_ASSERT(top);
    return top->m_dynamicData->declarationForIndex(m_declarationIndex);
}

bool LocalIndexedDeclaration::isLoaded(TopDUContext* top) const
{
    if (!m_declarationIndex)
        return false;

    Q_ASSERT(top);
    return top->m_dynamicData->isDeclarationForIndexLoaded(m_declarationIndex);
}
