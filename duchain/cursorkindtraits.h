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
#include <language/duchain/types/integraltype.h>

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
    || CK == CXCursor_MemberRefExpr
    || CK == CXCursor_MacroExpansion;
}

constexpr bool isClassTemplate(CXCursorKind CK)
{
    return CK == CXCursor_ClassTemplate || CK == CXCursor_ClassTemplatePartialSpecialization;
}

constexpr bool isClass(CXCursorKind CK)
{
    return isClassTemplate(CK)
    || CK == CXCursor_StructDecl
    || CK == CXCursor_ClassDecl
    || CK == CXCursor_UnionDecl;
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
    return CK == CXCursor_Namespace || CK == CXCursor_MacroDefinition ?
        Decision::True :
        isClass(CK) || isFunction(CK) || CK == CXCursor_EnumDecl ?
        Decision::Maybe :
        Decision::False;
}

constexpr Decision isInClass(CXCursorKind CK)
{
    return CK == CXCursor_FieldDecl ?
        Decision::True :
           CK == CXCursor_Namespace
        || CK == CXCursor_TemplateTypeParameter
        || CK == CXCursor_FunctionDecl
        || CK == CXCursor_TemplateTemplateParameter
        || CK == CXCursor_NonTypeTemplateParameter
        || CK == CXCursor_MacroDefinition
        || CK == CXCursor_MacroExpansion ?
        Decision::False :
        Decision::Maybe;
}

constexpr DUContext::ContextType contextType(CXCursorKind CK)
{
    return CK == CXCursor_StructDecl                    ? DUContext::Class
    : CK == CXCursor_UnionDecl                          ? DUContext::Class
    : CK == CXCursor_ClassDecl                          ? DUContext::Class
    : CK == CXCursor_EnumDecl                           ? DUContext::Enum
    : CK == CXCursor_FunctionDecl                       ? DUContext::Function
    : CK == CXCursor_CXXMethod                          ? DUContext::Function
    : CK == CXCursor_Namespace                          ? DUContext::Namespace
    : CK == CXCursor_Constructor                        ? DUContext::Function
    : CK == CXCursor_Destructor                         ? DUContext::Function
    : CK == CXCursor_ConversionFunction                 ? DUContext::Function
    : CK == CXCursor_FunctionTemplate                   ? DUContext::Function
    : CK == CXCursor_ClassTemplate                      ? DUContext::Class
    : CK == CXCursor_ClassTemplatePartialSpecialization ? DUContext::Class
    : CK == CXCursor_MacroDefinition                    ? DUContext::Other
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

constexpr bool isKDevClassFunctionDeclaration(CXCursorKind CK, bool isInClass)
{
    return isInClass && isFunction(CK);
}

constexpr bool isKDevFunctionDeclaration(CXCursorKind CK, bool isDefinition, bool isInClass)
{
    return !isDefinition && !isInClass && isFunction(CK);
}

constexpr bool isKDevFunctionDefinition(CXCursorKind CK, bool isDefinition, bool isInClass)
{
    return isDefinition && !isInClass && isFunction(CK);
}

constexpr bool isKDevNamespaceAliasDeclaration(CXCursorKind CK, bool isDefinition)
{
    return !isDefinition && CK == CXCursor_NamespaceAlias;
}

constexpr bool isKDevClassMemberDeclaration(CXCursorKind CK, bool isInClass)
{
    return isInClass && isKDevDeclaration(CK, false);
}

constexpr Declaration::AccessPolicy kdevAccessPolicy(CX_CXXAccessSpecifier access)
{
    return access == CX_CXXPrivate ? Declaration::Private
    : access == CX_CXXProtected ?    Declaration::Protected
    : access == CX_CXXPublic ?       Declaration::Public
    :                                Declaration::DefaultAccess;
}

constexpr IntegralType::CommonIntegralTypes integralType(CXTypeKind TK)
{
    return TK == CXType_Void    ? IntegralType::TypeVoid
    : TK == CXType_Bool         ? IntegralType::TypeBoolean
    : TK == CXType_Float        ? IntegralType::TypeFloat
    : TK == CXType_Char16       ? IntegralType::TypeChar16_t
    : TK == CXType_Char32       ? IntegralType::TypeChar32_t
    : TK == CXType_WChar        ? IntegralType::TypeWchar_t
    : ( TK == CXType_LongDouble
      ||TK == CXType_Double)    ? IntegralType::TypeDouble
    : ( TK == CXType_Short
      ||TK == CXType_UShort
      ||TK == CXType_Int
      ||TK == CXType_UInt
      ||TK == CXType_Long
      ||TK == CXType_ULong
      ||TK == CXType_LongLong
      ||TK == CXType_ULongLong) ? IntegralType::TypeInt
    : ( TK == CXType_Char_U
      ||TK == CXType_Char_S
      ||TK == CXType_UChar
      ||TK == CXType_SChar)     ?  IntegralType::TypeChar
    : static_cast<IntegralType::CommonIntegralTypes>(-1);
}

constexpr bool isPointerType(CXTypeKind CK)
{
    return CK == CXType_Pointer
        || CK == CXType_BlockPointer
        || CK == CXType_ObjCObjectPointer
        || CK == CXType_MemberPointer;
}

constexpr bool isAliasType(CXCursorKind CK)
{
    return CK == CXCursor_TypedefDecl || CK == CXCursor_TypeAliasDecl;
}

constexpr bool isIdentifiedType(CXCursorKind CK)
{
    return isClass(CK) || isAliasType(CK) || CK == CXCursor_EnumDecl || CK == CXCursor_EnumConstantDecl;
}

}

#endif //CURSORKINDTRAITS_H
