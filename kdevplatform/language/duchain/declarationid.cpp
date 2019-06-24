/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "declarationid.h"

#include "ducontext.h"
#include "topducontext.h"
#include "duchain.h"
#include "declaration.h"
#include "persistentsymboltable.h"
#include "instantiationinformation.h"

#include <util/convenientfreelist.h>

namespace KDevelop {
DeclarationId::DeclarationId(const IndexedQualifiedIdentifier& id, uint additionalId,
                             const IndexedInstantiationInformation& specialization)
    : m_indirectData{id, additionalId}
    , m_isDirect(false)
    , m_specialization(specialization)
{
}

DeclarationId::DeclarationId(const IndexedDeclaration& decl,
                             const IndexedInstantiationInformation& specialization)
    : m_directData(decl)
    , m_isDirect(true)
    , m_specialization(specialization)
{
}

DeclarationId::DeclarationId(const DeclarationId& rhs)
    : m_isDirect(rhs.m_isDirect)
    , m_specialization(rhs.m_specialization)
{
    if (!m_isDirect) {
        // IndexedQualifiedIdentifier doesn't like zero-initialization...
        new (&m_indirectData.identifier) IndexedQualifiedIdentifier(rhs.m_indirectData.identifier);
        m_indirectData.additionalIdentity = rhs.m_indirectData.additionalIdentity;
    } else {
        m_directData = rhs.m_directData;
    }
}

DeclarationId::~DeclarationId()
{
    if (!m_isDirect) {
        m_indirectData.~Indirect();
    }
}

DeclarationId& DeclarationId::operator=(const DeclarationId& rhs)
{
    if (&rhs == this)
        return *this;

    m_isDirect = rhs.m_isDirect;
    m_specialization = rhs.m_specialization;
    if (!m_isDirect) {
        m_indirectData = rhs.m_indirectData;
    } else {
        m_directData = rhs.m_directData;
    }
    return *this;
}

bool DeclarationId::isDirect() const
{
    return m_isDirect;
}

void DeclarationId::setSpecialization(const IndexedInstantiationInformation& spec)
{
    m_specialization = spec;
}

IndexedInstantiationInformation DeclarationId::specialization() const
{
    return m_specialization;
}

KDevVarLengthArray<Declaration*> DeclarationId::declarations(const TopDUContext* top) const
{
    KDevVarLengthArray<Declaration*> ret;

    if (m_isDirect == false) {
        //Find the declaration by its qualified identifier and additionalIdentity
        QualifiedIdentifier id(m_indirectData.identifier);

        if (top) {
            //Do filtering
            PersistentSymbolTable::FilteredDeclarationIterator filter =
                PersistentSymbolTable::self().filteredDeclarations(id, top->recursiveImportIndices());
            for (; filter; ++filter) {
                Declaration* decl = filter->data();
                if (decl && m_indirectData.additionalIdentity == decl->additionalIdentity()) {
                    //Hit
                    ret.append(decl);
                }
            }
        } else {
            //Just accept anything
            PersistentSymbolTable::Declarations decls = PersistentSymbolTable::self().declarations(id);
            PersistentSymbolTable::Declarations::Iterator decl = decls.iterator();
            for (; decl; ++decl) {
                const IndexedDeclaration& iDecl(*decl);

                ///@todo think this over once we don't pull in all imported top-context any more
                //Don't trigger loading of top-contexts from here, it will create a lot of problems
                if ((!DUChain::self()->isInMemory(iDecl.topContextIndex())))
                    continue;

                Declaration* decl = iDecl.data();
                if (decl && m_indirectData.additionalIdentity == decl->additionalIdentity()) {
                    //Hit
                    ret.append(decl);
                }
            }
        }
    } else {
        Declaration* decl = m_directData.declaration();
        if (decl)
            ret.append(decl);
    }

    if (!ret.isEmpty() && m_specialization.index()) {
        KDevVarLengthArray<Declaration*> newRet;
        for (Declaration* decl : qAsConst(ret)) {
            Declaration* specialized = decl->specialize(m_specialization, top ? top : decl->topContext());
            if (specialized)
                newRet.append(specialized);
        }

        return newRet;
    }
    return ret;
}

Declaration* DeclarationId::declaration(const TopDUContext* top, bool instantiateIfRequired) const
{
    Declaration* ret = nullptr;

    if (m_isDirect == false) {
        //Find the declaration by its qualified identifier and additionalIdentity
        QualifiedIdentifier id(m_indirectData.identifier);

        if (top) {
            //Do filtering
            PersistentSymbolTable::FilteredDeclarationIterator filter =
                PersistentSymbolTable::self().filteredDeclarations(id, top->recursiveImportIndices());
            for (; filter; ++filter) {
                Declaration* decl = filter->data();
                if (decl && m_indirectData.additionalIdentity == decl->additionalIdentity()) {
                    //Hit
                    ret = decl;
                    if (!ret->isForwardDeclaration())
                        break;
                }
            }
        } else {
            //Just accept anything
            PersistentSymbolTable::Declarations decls = PersistentSymbolTable::self().declarations(id);
            PersistentSymbolTable::Declarations::Iterator decl = decls.iterator();
            for (; decl; ++decl) {
                const IndexedDeclaration& iDecl(*decl);

                ///@todo think this over once we don't pull in all imported top-context any more
                //Don't trigger loading of top-contexts from here, it will create a lot of problems
                if ((!DUChain::self()->isInMemory(iDecl.topContextIndex())))
                    continue;

                Declaration* decl = iDecl.data();
                if (decl && m_indirectData.additionalIdentity == decl->additionalIdentity()) {
                    //Hit
                    ret = decl;
                    if (!ret->isForwardDeclaration())
                        break;
                }
            }
        }
    } else {
        //Find the declaration by m_topContext and m_declaration
        ret = m_directData.declaration();
    }

    if (ret) {
        if (m_specialization.isValid()) {
            const TopDUContext* topContextForSpecialization = top;
            if (!instantiateIfRequired)
                topContextForSpecialization = nullptr; //If we don't want to instantiate new declarations, set the top-context to zero, so specialize(..) will only look-up
            else if (!topContextForSpecialization)
                topContextForSpecialization = ret->topContext();

            return ret->specialize(m_specialization, topContextForSpecialization);
        } else {
            return ret;
        }
    } else
        return nullptr;
}

QualifiedIdentifier DeclarationId::qualifiedIdentifier() const
{
    if (!m_isDirect) {
        QualifiedIdentifier baseIdentifier = m_indirectData.identifier.identifier();
        if (!m_specialization.index())
            return baseIdentifier;
        return m_specialization.information().applyToIdentifier(baseIdentifier);
    } else {
        Declaration* decl = declaration(nullptr);
        if (decl)
            return decl->qualifiedIdentifier();

        return QualifiedIdentifier(i18n("(unknown direct declaration)"));
    }

    return QualifiedIdentifier(i18n("(missing)")) + m_indirectData.identifier.identifier();
}
}
