/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef TUDUCHAIN_H
#define TUDUCHAIN_H

#include "duchainexport.h"
#include "includedfilecontexts.h"
#include "templatehelpers.h"
#include "cursorkindtraits.h"
#include "clangtypes.h"
#include "typebuilder.h"
#include <unordered_map>

#include <util/pushvalue.h>

template<CXCursorKind CK, bool isDefinition, bool isClassMember, class Enable = void>
struct DeclType;

class KDEVCLANGDUCHAIN_EXPORT TUDUChain
{
public:
    TUDUChain(CXTranslationUnit tu, CXFile file, const IncludeFileContexts& includes, const bool update);

private:
    static CXChildVisitResult visitCursor(CXCursor cursor, CXCursor parent, CXClientData data);

    KDevelop::RangeInRevision makeContextRange(CXCursor cursor) const;
    KDevelop::Identifier makeId(CXCursor cursor) const;
    QByteArray makeComment(CXComment comment) const;

//BEGIN setDeclData
    template<CXCursorKind CK>
    void setDeclData(CXCursor, Declaration *decl) const
    {
        if (CK == CXCursor_TypeAliasDecl || CXCursor_TypedefDecl)
            decl->setIsTypeAlias(true);
        if (CK == CXCursor_Namespace)
            decl->setKind(Declaration::Namespace);
        if (CK == CXCursor_EnumDecl || CK == CXCursor_EnumConstantDecl || CursorKindTraits::isClass(CK))
            decl->setKind(Declaration::Type);
    }

    template<CXCursorKind CK, EnableIf<CursorKindTraits::isClassTemplate(CK)> = dummy>
    void setDeclData(CXCursor cursor, ClassDeclaration* decl) const
    {
        CXCursorKind kind = clang_getTemplateCursorKind(cursor);
        switch (kind) {
        case CXCursor_UnionDecl: setDeclData<CXCursor_UnionDecl>(cursor, decl); break;
        case CXCursor_StructDecl: setDeclData<CXCursor_StructDecl>(cursor, decl); break;
        case CXCursor_ClassDecl: setDeclData<CXCursor_ClassDecl>(cursor, decl); break;
        default: Q_ASSERT(false); break;
        }
    }

    template<CXCursorKind CK, EnableIf<!CursorKindTraits::isClassTemplate(CK)> = dummy>
    void setDeclData(CXCursor cursor, ClassDeclaration* decl) const
    {
        setDeclData<CK>(cursor, static_cast<Declaration*>(decl));
        if (CK == CXCursor_UnionDecl)
            decl->setClassType(ClassDeclarationData::Union);
        if (CK == CXCursor_StructDecl)
            decl->setClassType(ClassDeclarationData::Struct);
    }
//END setDeclData

//BEGIN dispatchCursor
    template<CXCursorKind CK, EnableIf<CursorKindTraits::isUse(CK)> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor)
    {
        return buildUse<CK>(cursor);
    }

    template<
        CXCursorKind CK,
        Decision IsClassMember = CursorKindTraits::isClassMember(CK),
        Decision IsDefinition = CursorKindTraits::isDefinition(CK),
        EnableIf<IsClassMember == Decision::Maybe || IsDefinition == Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor parent)
    {
        if (IsDefinition == Decision::Maybe) {
            const bool decision = clang_isCursorDefinition(cursor);
            return decision ?
                dispatchCursor<CK, IsClassMember, Decision::True>(cursor, parent) :
                dispatchCursor<CK, IsClassMember, Decision::False>(cursor, parent);
        }
        if (IsClassMember == Decision::Maybe) {
            const bool decision = CursorKindTraits::isClass(clang_getCursorKind(parent));
            return decision ?
                dispatchCursor<CK, Decision::True, IsDefinition>(cursor, parent) :
                dispatchCursor<CK, Decision::False, IsDefinition>(cursor, parent);
        }
        Q_ASSERT(false);
        return CXChildVisit_Break;
    }

    template<
        CXCursorKind CK,
        Decision IsClassMember = CursorKindTraits::isClassMember(CK),
        Decision IsDefinition = CursorKindTraits::isDefinition(CK),
        EnableIf<IsClassMember != Decision::Maybe && IsDefinition != Decision::Maybe> = dummy>
    CXChildVisitResult dispatchCursor(CXCursor cursor, CXCursor /*parent*/)
    {
        constexpr bool isClassMember = IsClassMember == Decision::True;
        constexpr bool isDefinition = IsDefinition == Decision::True;
        //Currently, but not technically, hasContext and IsDefinition are synonyms
        constexpr bool hasContext = IsDefinition == Decision::True;
        return buildDeclaration<CK, typename DeclType<CK, isDefinition, isClassMember>::Type, hasContext>(cursor);
    }
//BEGIN dispatchCursor

//BEGIN build*
    template<CXCursorKind CK, class DeclType, bool hasContext>
    CXChildVisitResult buildDeclaration(CXCursor cursor)
    {
        auto id = makeId(cursor);
        if (hasContext) {
            auto context = createContext<CK, CursorKindTraits::contextType(CK)>(cursor, id);
            auto decl = createDeclaration<CK, DeclType>(cursor, id, context);
            AbstractType::Ptr type = TypeBuilder::build(clang_getCursorType(cursor), m_includes);
            {
                DUChainWriteLocker lock;
                decl->setAbstractType(type);
            }
            CurrentContext newParent(context);
            PushValue<CurrentContext*> pushCurrent(m_parentContext, &newParent);
            clang_visitChildren(cursor, &visitCursor, this);
            return CXChildVisit_Continue;
        }
        auto decl = createDeclaration<CK, DeclType>(cursor, id);
        AbstractType::Ptr type = TypeBuilder::build(clang_getCursorType(cursor), m_includes);
        {
            DUChainWriteLocker lock;
            decl->setAbstractType(type);
        }
        return CXChildVisit_Recurse;
    }

    template<CXCursorKind CK>
    CXChildVisitResult buildUse(CXCursor cursor)
    {
        m_uses[m_parentContext->context].push_back(cursor);
        return CK == CXCursor_DeclRefExpr || CK == CXCursor_MemberRefExpr ?
            CXChildVisit_Recurse : CXChildVisit_Continue;
    }
//END build*

//BEGIN create*
    template<CXCursorKind CK, class DeclType>
    KDevelop::Declaration* createDeclarationCommon(CXCursor cursor, const Identifier& id)
    {
        auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
        auto comment = makeComment(clang_Cursor_getParsedComment(cursor));
        if (m_update) {
            const IndexedIdentifier indexedId(id);
            DUChainWriteLocker lock;
            auto it = m_parentContext->previousChildDeclarations.begin();
            while (it != m_parentContext->previousChildDeclarations.end()) {
                auto decl = *it;
                if (dynamic_cast<DeclType*>(decl) && decl->indexedIdentifier() == indexedId) {
                    decl->setRange(range);
                    decl->setComment(comment);
                    setDeclData<CK>(cursor, decl);
                    m_parentContext->previousChildDeclarations.erase(it);
                    return decl;
                }
                ++it;
            }
        }
        auto decl = new DeclType(range, nullptr);
        decl->setIdentifier(id);
        decl->setComment(comment);
        setDeclData<CK>(cursor, decl);
        return decl;
    }

    template<CXCursorKind CK, class DeclType>
    KDevelop::Declaration* createDeclaration(CXCursor cursor, const KDevelop::Identifier& id)
    {
        auto decl = createDeclarationCommon<CK, DeclType>(cursor, id);
        DUChainWriteLocker lock;
        decl->setContext(m_parentContext->context);
        return decl;
    }

    template<CXCursorKind CK, class DeclType>
    KDevelop::Declaration* createDeclaration(CXCursor cursor, const KDevelop::Identifier& id, KDevelop::DUContext *context)
    {
        auto decl = createDeclarationCommon<CK, DeclType>(cursor, id);
        DUChainWriteLocker lock;
        decl->setContext(m_parentContext->context);
        decl->setInternalContext(context);
        return decl;
    }

    template<CXCursorKind CK, KDevelop::DUContext::ContextType Type>
    KDevelop::DUContext* createContext(CXCursor cursor, const KDevelop::Identifier& id)
    {
        DUChainWriteLocker lock;
        // wtf: why is the DUContext API requesting a QID when it needs a plain Id?!
        // see: testNamespace
        const QualifiedIdentifier scopeId(id);
        if (m_update) {
            const KDevelop::IndexedQualifiedIdentifier indexedScopeId(scopeId);
            auto it = m_parentContext->previousChildContexts.begin();
            while (it != m_parentContext->previousChildContexts.end()) {
                auto ctx = *it;
                if (ctx->type() == Type && ctx->indexedLocalScopeIdentifier() == indexedScopeId) {
                    ctx->setRange(makeContextRange(cursor));
                    m_parentContext->previousChildContexts.erase(it);
                    return ctx;
                }
                ++it;
            }
        }
        //TODO: (..type, id..) constructor for DUContext?
        auto context = new KDevelop::DUContext(makeContextRange(cursor), m_parentContext->context);
        context->setType(Type);
        context->setLocalScopeIdentifier(scopeId);
        if (Type == KDevelop::DUContext::Other)
            context->setInSymbolTable(false);
        return context;
    }
//END create*

private:
    struct CurrentContext
    {
        CurrentContext(KDevelop::DUContext* context)
            : context(context)
        {
            DUChainReadLocker lock;
            previousChildContexts = context->childContexts();
            previousChildDeclarations = context->localDeclarations();
        }
        ~CurrentContext()
        {
            DUChainWriteLocker lock;
            qDeleteAll(previousChildContexts);
            qDeleteAll(previousChildDeclarations);
        }

        KDevelop::DUContext* context;
        // when updatig, this contains child contexts of the current parent context
        QVector<KDevelop::DUContext*> previousChildContexts;
        // when updatig, this contains child declarations of the current parent context
        QVector<KDevelop::Declaration*> previousChildDeclarations;
    };
    friend CurrentContext;

    const CXFile m_file;
    const IncludeFileContexts &m_includes;

    std::unordered_map<DUContext*, std::vector<CXCursor>> m_uses;
    CurrentContext *m_parentContext;

    const bool m_update;
};

#endif //TUDUCHAIN_H
