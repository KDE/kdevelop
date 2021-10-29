/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "specializationstore.h"
#include "declarationid.h"
#include "ducontext.h"
#include "declaration.h"

namespace KDevelop {
SpecializationStore::SpecializationStore()
{
}

SpecializationStore::~SpecializationStore()
{
}

SpecializationStore& SpecializationStore::self()
{
    static SpecializationStore store;
    return store;
}

void SpecializationStore::set(const DeclarationId& declaration,
                              const IndexedInstantiationInformation& specialization)
{
    Q_ASSERT(specialization.index() >> 16);
    m_specializations[declaration] = specialization;
}

IndexedInstantiationInformation SpecializationStore::get(const DeclarationId& declaration)
{
    QHash<DeclarationId, IndexedInstantiationInformation>::const_iterator it = m_specializations.constFind(declaration);
    if (it != m_specializations.constEnd())
        return *it;
    else
        return IndexedInstantiationInformation();
}

void SpecializationStore::clear(const DeclarationId& declaration)
{
    QHash<DeclarationId, IndexedInstantiationInformation>::iterator it = m_specializations.find(declaration);
    if (it != m_specializations.end())
        m_specializations.erase(it);
}

void SpecializationStore::clear()
{
    m_specializations.clear();
}

Declaration* SpecializationStore::applySpecialization(Declaration* declaration, TopDUContext* source,
                                                      bool recursive)
{
    if (!declaration)
        return nullptr;

    IndexedInstantiationInformation specialization = get(declaration->id());
    if (specialization.index())
        return declaration->specialize(specialization, source);

    if (declaration->context() && recursive) {
        //Find a parent that has a specialization, and specialize this with the info and required depth
        int depth = 0;
        DUContext* ctx = declaration->context();
        IndexedInstantiationInformation specialization;
        while (ctx && !specialization.index()) {
            if (ctx->owner())
                specialization = get(ctx->owner()->id());
            ++depth;
            ctx = ctx->parentContext();
        }

        if (specialization.index())
            return declaration->specialize(specialization, source, depth);
    }

    return declaration;
}

DUContext* SpecializationStore::applySpecialization(DUContext* context, TopDUContext* source,
                                                    bool recursive)
{
    if (!context)
        return nullptr;

    if (Declaration* declaration = context->owner())
        return applySpecialization(declaration, source, recursive)->internalContext();

    if (context->parentContext() && recursive) {
        //Find a parent that has a specialization, and specialize this with the info and required depth
        int depth = 0;
        DUContext* ctx = context->parentContext();
        IndexedInstantiationInformation specialization;
        while (ctx && !specialization.index()) {
            if (ctx->owner())
                specialization = get(ctx->owner()->id());
            ++depth;
            ctx = ctx->parentContext();
        }

        if (specialization.index())
            return context->specialize(specialization, source, depth);
    }

    return context;
}
}
