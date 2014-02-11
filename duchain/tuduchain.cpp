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

#include "tuduchain.h"
#include "debug.h"

#include <language/duchain/types/indexedtype.h>

//BEGIN IdType

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CursorKindTraits::isClass(CK)>::type>
{
    typedef StructureType Type;
};

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CK == CXCursor_TypedefDecl>::type>
{
    typedef TypeAliasType Type;
};

template<CXCursorKind CK>
struct IdType<CK, typename std::enable_if<CK == CXCursor_EnumDecl>::type>
{
    typedef EnumerationType Type;
};

//END IdType

//BEGIN DeclType

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevDeclaration(CK, isClassMember)>::type>
{
    typedef Declaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevForwardDeclaration(CK, isDefinition)>::type>
{
    typedef ForwardDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevClassDeclaration(CK, isDefinition)>::type>
{
    typedef ClassDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevClassFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef ClassFunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef FunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevFunctionDefinition(CK, isDefinition)>::type>
{
    typedef FunctionDefinition Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevNamespaceAliasDeclaration(CK, isDefinition)>::type>
{
    typedef NamespaceAliasDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevClassMemberDeclaration(CK, isClassMember)>::type>
{
    typedef ClassMemberDeclaration Type;
};
//END DeclType

//BEGIN CreateType

template<CXTypeKind TK>
void TUDUChain::setTypeModifiers(CXType type, AbstractType* kdevType) const
{
    quint64 modifiers = 0;
    if (clang_isConstQualifiedType(type)) {
        modifiers |= AbstractType::ConstModifier;
    }
    if (clang_isVolatileQualifiedType(type)) {
        modifiers |= AbstractType::VolatileModifier;
    }
    if (TK == CXType_Short || TK == CXType_UShort) {
        modifiers |= AbstractType::ShortModifier;
    }
    if (TK == CXType_Long || TK == CXType_LongDouble || TK == CXType_ULong) {
        modifiers |= AbstractType::LongModifier;
    }
    if (TK == CXType_LongLong || TK == CXType_ULongLong) {
        modifiers |= AbstractType::LongLongModifier;
    }
    if (TK == CXType_SChar) {
        modifiers |= AbstractType::SignedModifier;
    }
    if (TK == CXType_UChar || TK == CXType_UInt || TK == CXType_UShort
        || TK == CXType_UInt128 || TK == CXType_ULong || TK == CXType_ULongLong)
    {
        modifiers |= AbstractType::UnsignedModifier;
    }
    kdevType->setModifiers(modifiers);
}
//END CreateType

TUDUChain::TUDUChain(CXTranslationUnit tu, CXFile file, const IncludeFileContexts& includes, const bool update)
: m_file(file)
, m_includes(includes)
, m_parentContext(nullptr)
, m_update(update)
{
    CXCursor tuCursor = clang_getTranslationUnitCursor(tu);
    CurrentContext parent(includes[file]);
    m_parentContext = &parent;
    clang_visitChildren(tuCursor, &visitCursor, this);

    TopDUContext *top = m_parentContext->context->topContext();
    if (m_update) {
        DUChainWriteLocker lock;
        top->deleteUsesRecursively();
    }
    for (const auto &contextUses : m_uses) {
        for (const auto &cursor : contextUses.second) {
            auto referenced = clang_getCursorReferenced(cursor);
            auto used = findDeclaration(referenced, includes);
            if (!used) {
                continue;
            }
            auto useRange = ClangRange(clang_getCursorReferenceNameRange(cursor, CXNameRange_WantSinglePiece, 0)).toRangeInRevision();
            DUChainWriteLocker lock;
            auto usedIndex = top->indexForUsedDeclaration(used.data());
            contextUses.first->createUse(usedIndex, useRange);
        }
    }
}

CXChildVisitResult TUDUChain::visitCursor(CXCursor cursor, CXCursor parent, CXClientData data)
{
    TUDUChain *thisPtr = static_cast<TUDUChain*>(data);

    auto location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (file != thisPtr->m_file) {
        return CXChildVisit_Continue;
    }

#define UseCursorKind(CursorKind, ...) case CursorKind: return thisPtr->dispatchCursor<CursorKind>(__VA_ARGS__);
    switch (clang_getCursorKind(cursor))
    {
    UseCursorKind(CXCursor_UnexposedDecl, cursor, parent);
    UseCursorKind(CXCursor_StructDecl, cursor, parent);
    UseCursorKind(CXCursor_UnionDecl, cursor, parent);
    UseCursorKind(CXCursor_ClassDecl, cursor, parent);
    UseCursorKind(CXCursor_EnumDecl, cursor, parent);
    UseCursorKind(CXCursor_FieldDecl, cursor, parent);
    UseCursorKind(CXCursor_EnumConstantDecl, cursor, parent);
    UseCursorKind(CXCursor_FunctionDecl, cursor, parent);
    UseCursorKind(CXCursor_VarDecl, cursor, parent);
    UseCursorKind(CXCursor_ParmDecl, cursor, parent);
    UseCursorKind(CXCursor_TypedefDecl, cursor, parent);
    UseCursorKind(CXCursor_CXXMethod, cursor, parent);
    UseCursorKind(CXCursor_Namespace, cursor, parent);
    UseCursorKind(CXCursor_Constructor, cursor, parent);
    UseCursorKind(CXCursor_Destructor, cursor, parent);
    UseCursorKind(CXCursor_ConversionFunction, cursor, parent);
    UseCursorKind(CXCursor_TemplateTypeParameter, cursor, parent);
    UseCursorKind(CXCursor_NonTypeTemplateParameter, cursor, parent);
    UseCursorKind(CXCursor_TemplateTemplateParameter, cursor, parent);
    UseCursorKind(CXCursor_FunctionTemplate, cursor, parent);
    UseCursorKind(CXCursor_ClassTemplate, cursor, parent);
    UseCursorKind(CXCursor_ClassTemplatePartialSpecialization, cursor, parent);
    UseCursorKind(CXCursor_TypeRef, cursor);
    UseCursorKind(CXCursor_CXXBaseSpecifier, cursor);
    UseCursorKind(CXCursor_TemplateRef, cursor);
    UseCursorKind(CXCursor_NamespaceRef, cursor);
    UseCursorKind(CXCursor_MemberRef, cursor);
    UseCursorKind(CXCursor_LabelRef, cursor);
    UseCursorKind(CXCursor_OverloadedDeclRef, cursor);
    UseCursorKind(CXCursor_VariableRef, cursor);
    UseCursorKind(CXCursor_DeclRefExpr, cursor);
    UseCursorKind(CXCursor_MemberRefExpr, cursor);
    default: return CXChildVisit_Recurse;
    }
}

void TUDUChain::setIdTypeDecl(CXCursor typeCursor, IdentifiedType* idType) const
{
    DeclarationPointer decl = findDeclaration(typeCursor, m_includes);
    DUChainReadLocker lock;
    if (decl) {
        idType->setDeclaration(decl.data());
    }
}

KDevelop::RangeInRevision TUDUChain::makeContextRange(CXCursor cursor) const
{
    auto start = clang_getRangeEnd(clang_Cursor_getSpellingNameRange(cursor, 0, 0));
    auto end = clang_getRangeEnd(clang_getCursorExtent(cursor));
    return {ClangLocation(start), ClangLocation(end)};
}

KDevelop::Identifier TUDUChain::makeId(CXCursor cursor) const
{
    return Identifier(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
}

QByteArray TUDUChain::makeComment(CXComment comment) const
{
    auto kind = clang_Comment_getKind(comment);
    if (kind == CXComment_Text)
        return {ClangString(clang_TextComment_getText(comment))};

    QByteArray text;
    int numChildren = clang_Comment_getNumChildren(comment);
    for (int i = 0; i < numChildren; ++i)
        text += makeComment(clang_Comment_getChild(comment, i));
    return text;
}

AbstractType *TUDUChain::makeType(CXType type) const
{
    #define UseKind(TypeKind) case TypeKind: return dispatchType<TypeKind>(type)
    switch (type.kind) {
    UseKind(CXType_Void);
    UseKind(CXType_Bool);
    UseKind(CXType_Short);
    UseKind(CXType_UShort);
    UseKind(CXType_Int);
    UseKind(CXType_UInt);
    UseKind(CXType_Long);
    UseKind(CXType_ULong);
    UseKind(CXType_LongLong);
    UseKind(CXType_ULongLong);
    UseKind(CXType_Float);
    UseKind(CXType_LongDouble);
    UseKind(CXType_Double);
    UseKind(CXType_Char_U);
    UseKind(CXType_Char_S);
    UseKind(CXType_UChar);
    UseKind(CXType_SChar);
    UseKind(CXType_Char16);
    UseKind(CXType_Char32);
    UseKind(CXType_Pointer);
    UseKind(CXType_ConstantArray);
    UseKind(CXType_LValueReference);
    UseKind(CXType_RValueReference);
    UseKind(CXType_FunctionProto);
    UseKind(CXType_Record);
    UseKind(CXType_Enum);
    UseKind(CXType_Typedef);
    UseKind(CXType_Int128);
    UseKind(CXType_UInt128);
    UseKind(CXType_Vector);
    UseKind(CXType_Unexposed);
    UseKind(CXType_WChar);
    case CXType_Invalid:
        return nullptr;
    default:
        debug() << "Unhandled type: " << type.kind << ClangString(clang_getTypeSpelling(type));
        return nullptr;
    }
}

template<>
CXChildVisitResult TUDUChain::buildUse<CXCursor_CXXBaseSpecifier>(CXCursor cursor)
{
    m_uses[m_parentContext->context].push_back(cursor);

    // TODO: get access policy and virtual bits
    bool virtualInherited = false;
    Declaration::AccessPolicy access = Declaration::Public;

    auto type = clang_getCursorType(cursor);
    auto decl = findDeclaration(clang_getTypeDeclaration(type), m_includes);
    if (!decl) {
        // this happens for templates with template-dependent base classes e.g. - dunno whether we can/should do more here
        debug() << "failed to find declaration for base specifier:" << ClangString(clang_getCursorDisplayName(cursor));
        return CXChildVisit_Continue;
    }

    DUChainWriteLocker lock;
    auto currentContext = m_parentContext->context;
    auto top = currentContext->topContext();
    if (auto import = decl->logicalInternalContext(top)) {
        currentContext->addImportedParentContext(import);
    }
    auto classDecl = dynamic_cast<KDevelop::ClassDeclaration*>(currentContext->owner());
    Q_ASSERT(classDecl);

    classDecl->addBaseClass({decl->indexedType(), access, virtualInherited});
    return CXChildVisit_Continue;
}
