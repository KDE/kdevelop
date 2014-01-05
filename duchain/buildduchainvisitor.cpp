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
#include "parsesession.h"

#include <language/backgroundparser/urlparselock.h>

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

struct ClientData
{
    DUContext* parent;
    const IncludeFileContexts& includeContexts;
    CXFile file;
};

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d);

CXChildVisitResult recurse(CXCursor cursor, ClientData* data, DUContext* context)
{
    ClientData childData{context, data->includeContexts, data->file};
    clang_visitChildren(cursor, &::visit, &childData);
    return CXChildVisit_Continue;
}

template<CXCursorKind kind>
CXChildVisitResult buildDeclaration(CXCursor cursor, ClientData* data)
{
    auto decl = DeclarationBuilder::build<kind>(cursor, data->parent);
    //TODO: This should be done earlier based on the cursorkind and decl/def status
    return decl->internalContext() ?
        recurse(cursor, data, decl->internalContext()) :
        CXChildVisit_Recurse;
}

template<CXCursorKind kind>
CXChildVisitResult buildUse(CXCursor cursor, ClientData* data)
{
    return UseBuilder::build<kind>(cursor, data->parent, data->includeContexts);
}

CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d)
{
    auto data = static_cast<ClientData*>(d);

    auto location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (file != data->file) {
        const auto& include = data->includeContexts.value(file);
        if (!include.needsUpdate) {
            return CXChildVisit_Continue;
        } else {
            data->file = file;
            data->parent = include.topContext;
        }
    }

    //Use to map cursor kinds to build profiles
    #define UseCursorKind(CursorKind, CursorBuilder)\
    case CursorKind: return CursorBuilder<CursorKind>(cursor, data);

    switch (clang_getCursorKind(cursor))
    {
    UseCursorKind(CXCursor_UnexposedDecl, buildDeclaration);
    UseCursorKind(CXCursor_StructDecl, buildDeclaration);
    UseCursorKind(CXCursor_UnionDecl, buildDeclaration);
    UseCursorKind(CXCursor_ClassDecl, buildDeclaration);
    UseCursorKind(CXCursor_EnumDecl, buildDeclaration);
    UseCursorKind(CXCursor_FieldDecl, buildDeclaration);
    UseCursorKind(CXCursor_EnumConstantDecl, buildDeclaration);
    UseCursorKind(CXCursor_FunctionDecl, buildDeclaration);
    UseCursorKind(CXCursor_VarDecl, buildDeclaration);
    UseCursorKind(CXCursor_ParmDecl, buildDeclaration);
    UseCursorKind(CXCursor_TypedefDecl, buildDeclaration);
    UseCursorKind(CXCursor_CXXMethod, buildDeclaration);
    UseCursorKind(CXCursor_Namespace, buildDeclaration);
    UseCursorKind(CXCursor_Constructor, buildDeclaration);
    UseCursorKind(CXCursor_Destructor, buildDeclaration);
    UseCursorKind(CXCursor_ConversionFunction, buildDeclaration);
    UseCursorKind(CXCursor_TemplateTypeParameter, buildDeclaration);
    UseCursorKind(CXCursor_NonTypeTemplateParameter, buildDeclaration);
    UseCursorKind(CXCursor_TemplateTemplateParameter, buildDeclaration);
    UseCursorKind(CXCursor_FunctionTemplate, buildDeclaration);
    UseCursorKind(CXCursor_ClassTemplate, buildDeclaration);
    UseCursorKind(CXCursor_ClassTemplatePartialSpecialization, buildDeclaration);
    UseCursorKind(CXCursor_NamespaceAlias, buildDeclaration);
    UseCursorKind(CXCursor_TypeAliasDecl, buildDeclaration);
    UseCursorKind(CXCursor_TypeRef, buildUse)
    UseCursorKind(CXCursor_CXXBaseSpecifier, buildUse)
    UseCursorKind(CXCursor_TemplateRef, buildUse)
    UseCursorKind(CXCursor_NamespaceRef, buildUse)
    UseCursorKind(CXCursor_MemberRef, buildUse)
    UseCursorKind(CXCursor_LabelRef, buildUse)
    UseCursorKind(CXCursor_OverloadedDeclRef, buildUse)
    UseCursorKind(CXCursor_VariableRef, buildUse)
    UseCursorKind(CXCursor_DeclRefExpr, buildUse)
    UseCursorKind(CXCursor_MemberRefExpr, buildUse)
    default: return CXChildVisit_Recurse;
    }
}

}

void BuildDUChainVisitor::visit(ParseSession* session, const ReferencedTopDUContext& top, const IncludeFileContexts& includes)
{
    ClientData data{top, includes, session->file()};
    auto cursor = clang_getTranslationUnitCursor(session->unit());
    clang_visitChildren(cursor, &::visit, &data);
}
