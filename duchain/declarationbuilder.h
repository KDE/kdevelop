/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include "clangtypes.h"

#include "contextbuilder.h"

#include <language/duchain/identifier.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>

namespace DeclarationBuilder {

using namespace KDevelop;

inline QByteArray buildComment(CXComment comment)
{
    auto kind = clang_Comment_getKind(comment);
    if (kind == CXComment_Text)
        return {ClangString(clang_TextComment_getText(comment))};

    QByteArray text;
    int numChildren = clang_Comment_getNumChildren(comment);
    for (int i = 0; i < numChildren; ++i)
        text += buildComment(clang_Comment_getChild(comment, i));
    return text;
}

template<CXCursorKind CK>
void setData(CXCursor, Declaration*)
{
    // do nothing
}

template<CXCursorKind CK>
void setData(CXCursor cursor, ClassDeclaration* decl)
{
    CXCursorKind kind = CK;
    if (CK == CXCursor_ClassTemplate || CK == CXCursor_ClassTemplatePartialSpecialization) {
        kind = clang_getTemplateCursorKind(cursor);
    }
    if (kind == CXCursor_UnionDecl) {
        decl->setClassType(ClassDeclarationData::Union);
    } else if (kind == CXCursor_StructDecl) {
        decl->setClassType(ClassDeclarationData::Struct);
    }
}

template<>
void setData<CXCursor_TypeAliasDecl>(CXCursor /*cursor*/, Declaration* decl)
{
    decl->setIsTypeAlias(true);
}

template<>
void setData<CXCursor_TypedefDecl>(CXCursor /*cursor*/, Declaration* decl)
{
    decl->setIsTypeAlias(true);
}

template<>
void setData<CXCursor_Namespace>(CXCursor /*cursor*/, Declaration* decl)
{
    decl->setKind(Declaration::Namespace);
}

template<CXCursorKind CK>
void setData(CXCursor /*cursor*/, NamespaceAliasDeclaration* /*decl*/)
{
    static_assert(CK == CXCursor_NamespaceAlias, "Unexpected cursor kind");
    // FIXME: how to get the imported identifier from clang?
//     decl->setImportIdentifier(???);
}

template<CXCursorKind CK, class T>
T *createDeclarationCommon(CXCursor cursor, const Identifier& id)
{
    auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
    auto comment = buildComment(clang_Cursor_getParsedComment(cursor));
    auto decl = new T(range, nullptr);
    decl->setComment(comment);
    decl->setIdentifier(id);
    setData<CK>(cursor, decl);
    return decl;
}

Identifier getId(CXCursor cursor)
{
    return Identifier(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
}

template<CXCursorKind CK, class T>
T* createDeclOnly(CXCursor cursor, DUContext* parentContext)
{
    auto decl = createDeclarationCommon<CK, T>(cursor, getId(cursor));
    DUChainWriteLocker lock;
    decl->setContext(parentContext);
    return decl;
}

template<CXCursorKind CK, class T>
T* createDeclAndContext(CXCursor cursor, DUContext* parentContext)
{
    auto id = getId(cursor);
    auto decl = createDeclarationCommon<CK, T>(cursor, id);
    auto internalContext = ContextBuilder::build<CK>(cursor, id, parentContext);
    DUChainWriteLocker lock;
    decl->setContext(parentContext);
    decl->setInternalContext(internalContext);
    return decl;
}

constexpr bool isClassType(CXCursorKind CK)
{
    return CK == CXCursor_StructDecl
        || CK == CXCursor_ClassDecl
        || CK == CXCursor_UnionDecl
        || CK == CXCursor_ClassTemplate
        || CK == CXCursor_ClassTemplatePartialSpecialization;
}

constexpr bool isFunctionType(CXCursorKind CK)
{
    return CK == CXCursor_FunctionDecl
        || CK == CXCursor_CXXMethod
        || CK == CXCursor_Constructor
        || CK == CXCursor_Destructor
        || CK == CXCursor_ConversionFunction
        || CK == CXCursor_FunctionTemplate;
}

constexpr bool alwaysBuildDecl(CXCursorKind CK)
{
    return CK == CXCursor_UnexposedDecl
        || CK == CXCursor_FieldDecl
        || CK == CXCursor_EnumConstantDecl
        || CK == CXCursor_VarDecl
        || CK == CXCursor_ParmDecl
        || CK == CXCursor_TypedefDecl
        || CK == CXCursor_TemplateTypeParameter
        || CK == CXCursor_NonTypeTemplateParameter
        || CK == CXCursor_TemplateTemplateParameter
        || CK == CXCursor_NamespaceAlias
        || CK == CXCursor_UsingDirective
        || CK == CXCursor_UsingDeclaration
        || CK == CXCursor_TypeAliasDecl;
}

constexpr bool alwaysBuildDeclAndContext(CXCursorKind CK)
{
    return CK == CXCursor_Namespace;
}

constexpr bool mayBuildDeclOrDef(CXCursorKind CK)
{
    return isClassType(CK)
        || isFunctionType(CK)
        || CK == CXCursor_EnumDecl;
}

constexpr bool isKDevDeclaration(CXCursorKind CK)
{
    return CK == CXCursor_UnexposedDecl
        || CK == CXCursor_FieldDecl
        || CK == CXCursor_EnumConstantDecl
        || CK == CXCursor_VarDecl
        || CK == CXCursor_ParmDecl
        || CK == CXCursor_TypedefDecl
        || CK == CXCursor_TemplateTypeParameter
        || CK == CXCursor_NonTypeTemplateParameter
        || CK == CXCursor_TemplateTemplateParameter
        || CK == CXCursor_UsingDirective
        || CK == CXCursor_UsingDeclaration
        || CK == CXCursor_TypeAliasDecl
        || CK == CXCursor_Namespace
        || CK == CXCursor_EnumDecl;
}

constexpr bool isKDevClassDeclaration(CXCursorKind CK, bool isDefinition)
{
    return isDefinition && isClassType(CK);
}

constexpr bool isKDevForwardDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && isClassType(CK);
}

constexpr bool isKDevClassFunctionDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && CK == CXCursor_CXXMethod;
}

constexpr bool isKDevFunctionDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && isFunctionType(CK) && CK != CXCursor_CXXMethod;
}

constexpr bool isKDevFunctionDefinition(CXCursorKind CK, bool isDefinition)
{
    return isDefinition && isFunctionType(CK);
}

constexpr bool isKDevNamespaceAliasDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && CK == CXCursor_NamespaceAlias;
}

//BEGIN DeclType
template<CXCursorKind CK, bool isDefinition, class Enable = void>
struct DeclType;

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevDeclaration(CK)>::type>
{
    typedef Declaration Type;
};

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevForwardDeclaration(CK, isDefinition)>::type>
{
    typedef ForwardDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevClassDeclaration(CK, isDefinition)>::type>
{
    typedef ClassDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevClassFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef ClassFunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef FunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevFunctionDefinition(CK, isDefinition)>::type>
{
    typedef FunctionDefinition Type;
};

template<CXCursorKind CK, bool isDefinition>
struct DeclType<CK, isDefinition, typename std::enable_if<isKDevNamespaceAliasDeclaration(CK, isDefinition)>::type>
{
    typedef NamespaceAliasDeclaration Type;
};
//END DeclType

//BEGIN DeclBuilder
template<CXCursorKind CK, class Enable = void>
struct DeclBuilder;

template<CXCursorKind CK>
struct DeclBuilder<CK, typename std::enable_if<alwaysBuildDecl(CK)>::type>
{
    typedef typename DeclType<CK, false>::Type KDevType;
    static KDevType* build(CXCursor cursor, DUContext* parentContext)
    {
        return createDeclOnly<CK, KDevType>(cursor, parentContext);
    }
};

template<CXCursorKind CK>
struct DeclBuilder<CK, typename std::enable_if<alwaysBuildDeclAndContext(CK)>::type>
{
    typedef typename DeclType<CK, false>::Type KDevType;
    static KDevType* build(CXCursor cursor, DUContext* parentContext)
    {
        return createDeclAndContext<CK, KDevType>(cursor, parentContext);
    }
};

template<CXCursorKind CK>
struct DeclBuilder<CK, typename std::enable_if<mayBuildDeclOrDef(CK)>::type>
{
    static Declaration* build(CXCursor cursor, DUContext* parentContext)
    {
        if (clang_isCursorDefinition(cursor))
            return createDeclAndContext<CK, typename DeclType<CK, true>::Type>(cursor, parentContext);
        else
            return createDeclOnly<CK, typename DeclType<CK, false>::Type>(cursor, parentContext);
    }
};
//END DeclBuilder

template<CXCursorKind CK>
Declaration *build(CXCursor cursor, DUContext* parentContext)
{
    return DeclBuilder<CK>::build(cursor, parentContext);
}

}

#endif // DECLARATIONBUILDER_H
