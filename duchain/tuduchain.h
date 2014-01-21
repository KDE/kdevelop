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

class KDEVCLANGDUCHAIN_EXPORT TUDUChain
{
public:
    TUDUChain(CXTranslationUnit tu, CXFile file, const IncludeFileContexts& includes);

private:
    static CXChildVisitResult visitCursor(CXCursor cursor, CXCursor parent, CXClientData data);

    KDevelop::RangeInRevision makeContextRange(CXCursor cursor) const;
    KDevelop::Identifier makeId(CXCursor cursor) const;
    QByteArray makeComment(CXComment comment) const;

    template<CXCursorKind> void setDeclData(CXCursor, KDevelop::Declaration*) const;
    template<CXCursorKind> void setDeclData(CXCursor, KDevelop::ClassDeclaration*) const;

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
        return buildDeclaration<CK, typename CursorKindTraits::DeclType<CK, isDefinition, isClassMember>::Type, hasContext>(cursor);
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
            std::swap(context, m_parentContext);
            clang_visitChildren(cursor, &visitCursor, this);
            std::swap(context, m_parentContext);
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
        m_uses[m_parentContext].push_back(cursor);
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
        auto decl = new DeclType(range, nullptr);
        decl->setComment(comment);
        decl->setIdentifier(id);
        setDeclData<CK>(cursor, decl);
        return decl;
    }

    template<CXCursorKind CK, class DeclType>
    KDevelop::Declaration* createDeclaration(CXCursor cursor, const KDevelop::Identifier& id)
    {
        auto decl = createDeclarationCommon<CK, DeclType>(cursor, id);
        DUChainWriteLocker lock;
        decl->setContext(m_parentContext);
        return decl;
    }

    template<CXCursorKind CK, class DeclType>
    KDevelop::Declaration* createDeclaration(CXCursor cursor, const KDevelop::Identifier& id, KDevelop::DUContext *context)
    {
        auto decl = createDeclarationCommon<CK, DeclType>(cursor, id);
        DUChainWriteLocker lock;
        decl->setContext(m_parentContext);
        decl->setInternalContext(context);
        return decl;
    }

    template<CXCursorKind CK, KDevelop::DUContext::ContextType Type>
    KDevelop::DUContext* createContext(CXCursor cursor, const KDevelop::Identifier& id)
    {
        auto context = new KDevelop::DUContext(makeContextRange(cursor), m_parentContext);
        DUChainWriteLocker lock; //TODO: (..type, id..) constructor for DUContext?
        context->setType(Type);
        context->setLocalScopeIdentifier(m_parentContext->localScopeIdentifier() + id);
        if (Type == KDevelop::DUContext::Other)
            context->setInSymbolTable(false);
        return context;
    }
//END create*

private:
    const CXFile m_file;
    const IncludeFileContexts &m_includes;

    std::unordered_map<DUContext*, std::vector<CXCursor>> m_uses;
    KDevelop::DUContext *m_parentContext;
};

#endif //TUDUCHAIN_H
