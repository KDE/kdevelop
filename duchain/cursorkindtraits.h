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

#ifndef CURSORKINDTRAITS_H
#define CURSORKINDTRAITS_H

#include <clang-c/Index.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>

#include "templatehelpers.h"

namespace CursorKindTraits {

using namespace KDevelop;

constexpr bool isUse(CXCursorKind CK)
{
    return CK == CXCursor_TypeRef
    || CK == CXCursor_CXXBaseSpecifier
    || CK == CXCursor_TemplateRef
    || CK == CXCursor_NamespaceRef
    || CK == CXCursor_MemberRef
    || CK == CXCursor_LabelRef
    || CK == CXCursor_OverloadedDeclRef
    || CK == CXCursor_VariableRef
    || CK == CXCursor_DeclRefExpr
    || CK == CXCursor_MemberRefExpr;
}

constexpr bool isClass(CXCursorKind CK)
{
    return CK == CXCursor_StructDecl
    || CK == CXCursor_ClassDecl
    || CK == CXCursor_UnionDecl
    || CK == CXCursor_ClassTemplate
    || CK == CXCursor_ClassTemplatePartialSpecialization;
}

constexpr bool isFunction(CXCursorKind CK)
{
    return CK == CXCursor_FunctionDecl
    || CK == CXCursor_CXXMethod
    || CK == CXCursor_Constructor
    || CK == CXCursor_Destructor
    || CK == CXCursor_ConversionFunction
    || CK == CXCursor_FunctionTemplate;
}

constexpr bool isDeclaration(CXCursorKind CK)
{
    return isClass(CK) || isFunction(CK)
    || CK == CXCursor_UnexposedDecl
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

constexpr Decision isDefinition(CXCursorKind CK)
{
    return CK == CXCursor_Namespace ? Decision::True :
        isClass(CK) || isFunction(CK) || CK == CXCursor_EnumDecl ? Decision::Maybe :
        Decision::False;
}

constexpr Decision isClassMember(CXCursorKind CK)
{
    return CK == CXCursor_FieldDecl
        || CK == CXCursor_CXXMethod ?
        Decision::True :
           CK == CXCursor_Namespace
        || CK == CXCursor_TemplateTypeParameter
        || CK == CXCursor_TemplateTemplateParameter
        || CK == CXCursor_NonTypeTemplateParameter ?
        Decision::False :
        Decision::Maybe;
}

constexpr DUContext::ContextType contextType(CXCursorKind CK)
{
    return CK == CXCursor_StructDecl                    ? DUContext::Class
    : CK == CXCursor_UnionDecl                          ? DUContext::Class
    : CK == CXCursor_StructDecl                         ? DUContext::Class
    : CK == CXCursor_UnionDecl                          ? DUContext::Class
    : CK == CXCursor_ClassDecl                          ? DUContext::Class
    : CK == CXCursor_EnumDecl                           ? DUContext::Enum
    : CK == CXCursor_FunctionDecl                       ? DUContext::Other
    : CK == CXCursor_CXXMethod                          ? DUContext::Other
    : CK == CXCursor_Namespace                          ? DUContext::Namespace
    : CK == CXCursor_Constructor                        ? DUContext::Other
    : CK == CXCursor_Destructor                         ? DUContext::Other
    : CK == CXCursor_ConversionFunction                 ? DUContext::Other
    : CK == CXCursor_FunctionTemplate                   ? DUContext::Other
    : CK == CXCursor_ClassTemplate                      ? DUContext::Class
    : CK == CXCursor_ClassTemplatePartialSpecialization ? DUContext::Class
    : static_cast<DUContext::ContextType>(-1);
}

constexpr bool isKDevDeclaration(CXCursorKind CK, bool isClassMember)
{
    return !isClassMember &&
    (CK == CXCursor_UnexposedDecl
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
    || CK == CXCursor_EnumDecl);
}

constexpr bool isKDevClassDeclaration(CXCursorKind CK, bool isDefinition)
{
    return isDefinition && isClass(CK);
}

constexpr bool isKDevForwardDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && isClass(CK);
}

constexpr bool isKDevClassFunctionDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && CK == CXCursor_CXXMethod;
}

constexpr bool isKDevFunctionDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && isFunction(CK) && CK != CXCursor_CXXMethod;
}

constexpr bool isKDevFunctionDefinition(CXCursorKind CK, bool isDefinition)
{
    return isDefinition && isFunction(CK);
}

constexpr bool isKDevNamespaceAliasDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && CK == CXCursor_NamespaceAlias;
}

constexpr bool isKDevClassMemberDeclaration(CXCursorKind CK, bool isClassMember)
{
    return isClassMember && isKDevDeclaration(CK, false);
}

//BEGIN DeclType
template<CXCursorKind CK, bool isDefinition, bool isClassMember, class Enable = void>
struct DeclType;

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevDeclaration(CK, isClassMember)>::type>
{
    typedef Declaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevForwardDeclaration(CK, isDefinition)>::type>
{
    typedef ForwardDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevClassDeclaration(CK, isDefinition)>::type>
{
    typedef ClassDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevClassFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef ClassFunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef FunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevFunctionDefinition(CK, isDefinition)>::type>
{
    typedef FunctionDefinition Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevNamespaceAliasDeclaration(CK, isDefinition)>::type>
{
    typedef NamespaceAliasDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember, typename std::enable_if<isKDevClassMemberDeclaration(CK, isClassMember)>::type>
{
    typedef ClassMemberDeclaration Type;
};
//END DeclType

}

#endif //CURSORKINDTRAITS_H
