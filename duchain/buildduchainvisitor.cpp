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

#include "buildduchainvisitor.h"
#include "declarationbuilder.h"
#include "contextbuilder.h"
#include "usebuilder.h"
#include "clangtypes.h"

QDebug &operator<<(QDebug &dbg, CXCursor cursor)
{
    if (clang_Cursor_isNull(cursor))
        dbg << "CXCursor (NULL)";
    else
        dbg << "CXCursor"
            << ClangString(clang_getCursorKindSpelling(clang_getCursorKind(cursor)))
            << ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toDocumentRange()
            << ClangString(clang_getCursorSpelling(cursor));
    return dbg;
}

using namespace KDevelop;

namespace {

enum CursorBuildType
{
    CBT_Declaration,
    CBT_Context,
    CBT_Use,
    CBT_CtxtDecl,
    CBT_IdCtxtDecl
};

template<CXCursorKind, CursorBuildType> struct CursorBuilder
{
    static CXChildVisitResult build(CXCursor cursor, DUContext *parentContext) = delete;
};

template<CXCursorKind kind> struct CursorBuilder<kind, CBT_Declaration>
{
    static CXChildVisitResult build(CXCursor cursor, DUContext *parentContext)
    {
        Identifier id(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
        DeclarationBuilder::build<kind>(cursor, id, parentContext);
        return CXChildVisit_Recurse;
    }
};

template<CXCursorKind kind> struct CursorBuilder<kind, CBT_Context>
{
    static CXChildVisitResult build(CXCursor cursor, DUContext *parentContext)
    {
        ContextBuilder::build<kind>(cursor, parentContext);
        return CXChildVisit_Continue;
    }
};

template<CXCursorKind kind> struct CursorBuilder<kind, CBT_Use>
{
    static CXChildVisitResult build(CXCursor cursor, DUContext *parentContext)
    {
        return UseBuilder::build<kind>(cursor, parentContext);
    }
};

template<CXCursorKind kind> struct CursorBuilder<kind, CBT_CtxtDecl>
{
    static CXChildVisitResult build(CXCursor cursor, DUContext *parentContext)
    {
        Identifier id(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
        DeclarationBuilder::build<kind>(cursor, id,
            ContextBuilder::build<kind>(cursor, parentContext), parentContext);
        return CXChildVisit_Continue;
    }
};

template<CXCursorKind kind> struct CursorBuilder<kind, CBT_IdCtxtDecl>
{
    static CXChildVisitResult build(CXCursor cursor, DUContext *parentContext)
    {
        Identifier id(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
        DeclarationBuilder::build<kind>(cursor, id,
            ContextBuilder::build<kind>(cursor, id, parentContext), parentContext);
        return CXChildVisit_Continue;
    }
};

}

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d)
{
    auto parentContext = static_cast<DUContext*>(d);

    //Use to map cursor kinds to build profiles
    #define UseCursorKind(CursorKind, CursorBuildType)\
    case CursorKind: return CursorBuilder<CursorKind, CursorBuildType>::build(cursor, parentContext);

    //Use to map cursor kinds conditionally
    #define UseCursorCond(CursorKind, Cond, TrueCBT, FalseCBT)\
    case CursorKind: return Cond ?\
        CursorBuilder<CursorKind, TrueCBT>::build(cursor, parentContext) :\
        CursorBuilder<CursorKind, FalseCBT>::build(cursor, parentContext);

    switch (clang_getCursorKind(cursor))
    {
    UseCursorKind(CXCursor_UnexposedDecl, CBT_Declaration);
    UseCursorCond(CXCursor_StructDecl,
                  clang_isCursorDefinition(cursor),
                  CBT_IdCtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_UnionDecl,
                  clang_isCursorDefinition(cursor),
                  CBT_IdCtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_ClassDecl,
                  clang_isCursorDefinition(cursor),
                  CBT_IdCtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_EnumDecl,
                  clang_isCursorDefinition(cursor),
                  CBT_IdCtxtDecl, CBT_Declaration);
    UseCursorKind(CXCursor_FieldDecl, CBT_Declaration);
    UseCursorKind(CXCursor_EnumConstantDecl, CBT_Declaration);
    UseCursorCond(CXCursor_FunctionDecl,
                  clang_isCursorDefinition(cursor),
                  CBT_CtxtDecl, CBT_Declaration);
    UseCursorKind(CXCursor_VarDecl, CBT_Declaration);
    UseCursorKind(CXCursor_ParmDecl, CBT_Declaration);
    UseCursorKind(CXCursor_TypedefDecl, CBT_Declaration);
    UseCursorCond(CXCursor_CXXMethod,
                  clang_isCursorDefinition(cursor),
                  CBT_CtxtDecl, CBT_Declaration);
    UseCursorKind(CXCursor_Namespace, CBT_IdCtxtDecl);
    UseCursorCond(CXCursor_Constructor,
                  clang_isCursorDefinition(cursor),
                  CBT_CtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_Destructor,
                  clang_isCursorDefinition(cursor),
                  CBT_CtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_ConversionFunction,
                  clang_isCursorDefinition(cursor),
                  CBT_CtxtDecl, CBT_Declaration);
    UseCursorKind(CXCursor_TemplateTypeParameter, CBT_Declaration);
    UseCursorKind(CXCursor_NonTypeTemplateParameter, CBT_Declaration);
    UseCursorKind(CXCursor_TemplateTemplateParameter, CBT_Declaration);
    UseCursorCond(CXCursor_FunctionTemplate,
                  clang_isCursorDefinition(cursor),
                  CBT_CtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_ClassTemplate,
                  clang_isCursorDefinition(cursor),
                  CBT_IdCtxtDecl, CBT_Declaration);
    UseCursorCond(CXCursor_ClassTemplatePartialSpecialization,
                  clang_isCursorDefinition(cursor),
                  CBT_IdCtxtDecl, CBT_Declaration);
    UseCursorKind(CXCursor_NamespaceAlias, CBT_Declaration);
    //Should we make a declaration for these, or just a use?
    //UseCursorKind(CXCursor_UsingDirective, CBT_Declaration);
    //UseCursorKind(CXCursor_UsingDeclaration, CBT_Declaration);
    UseCursorKind(CXCursor_TypeAliasDecl, CBT_Declaration);
    UseCursorKind(CXCursor_TypeRef, CBT_Use)
    UseCursorKind(CXCursor_CXXBaseSpecifier, CBT_Use)
    UseCursorKind(CXCursor_TemplateRef, CBT_Use)
    UseCursorKind(CXCursor_NamespaceRef, CBT_Use)
    UseCursorKind(CXCursor_MemberRef, CBT_Use)
    UseCursorKind(CXCursor_LabelRef, CBT_Use)
    UseCursorKind(CXCursor_OverloadedDeclRef, CBT_Use)
    UseCursorKind(CXCursor_VariableRef, CBT_Use)
    UseCursorKind(CXCursor_DeclRefExpr, CBT_Use)
    UseCursorKind(CXCursor_MemberRefExpr, CBT_Use)
    default: return CXChildVisit_Recurse;
    }
}

void BuildDUChainVisitor::visit(CXTranslationUnit unit, const ReferencedTopDUContext& top)
{
    auto cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, &::visit, top.data());
}
