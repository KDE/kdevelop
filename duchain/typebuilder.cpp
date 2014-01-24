/*
 * Copyright 2014  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "typebuilder.h"
#include "clangtypes.h"
#include "templatehelpers.h"
#include "cursorkindtraits.h"

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/typealiastype.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include "debug.h"

using namespace KDevelop;
using namespace TypeBuilder;

namespace {

template<CXTypeKind TK>
void setTypeModifiers(AbstractType *type)
{
    
}

template<CXTypeKind TK, EnableIf<CursorKindTraits::integralType(TK) != -1> = dummy>
AbstractType *makeType(CXType type)
{
    return new IntegralType(CursorKindTraits::integralType(TK));
}

template<CXTypeKind TK, EnableIf<TK == CXType_Pointer> = dummy>
AbstractType *makeType(CXType type)
{
    auto ptr = new PointerType;
    ptr->setBaseType(build(clang_getPointeeType(type), includes));
    return ptr;
}

template<CXTypeKind TK, EnableIf<TK == CXType_ConstantArray> = dummy>
AbstractType *makeType(CXType type)
{
    auto arr = new ArrayType;
    arr->setDimension(clang_getArraySize(type));
    arr->setElementType(build(clang_getArrayElementType(type), includes));
    return arr;
}

template<CXTypeKind TK, EnableIf<TK == CXType_RValueReference || TK == CXType_LValueReference> = dummy>
AbstractType *makeType(CXType type)
{
    auto ref = new ReferenceType;
    ref->setIsRValue(type.kind == CXType_RValueReference);
    ref->setBaseType(build(clang_getPointeeType(type), includes));
    return ref;
}

template<CXTypeKind TK, EnableIf<TK == CXType_FunctionProto> = dummy>
AbstractType *makeType(CXType type)
{
    auto func = new FunctionType;
    func->setReturnType(build(clang_getResultType(type), includes));
    const int numArgs = clang_getNumArgTypes(type);
    for (int i = 0; i < numArgs; ++i) {
        func->addArgument(build(clang_getArgType(type, i), includes));
    }
    /// TODO: variadic functions
    return func;
}

template<CXTypeKind TK, EnableIf<TK == CXType_Record> = dummy>
AbstractType *makeType(CXType type)
{
    auto st = new StructureType;
    DeclarationPointer decl = findDeclaration(clang_getTypeDeclaration(type), includes);
    DUChainReadLocker lock;
    if (decl) {
        st->setDeclaration(decl.data());
    }
    return st;
}

template<CXTypeKind TK, EnableIf<TK == CXType_Enum> = dummy>
AbstractType *makeType(CXType type)
{
    auto t = new EnumerationType;
    DeclarationPointer decl = findDeclaration(clang_getTypeDeclaration(type), includes);
    DUChainReadLocker lock;
    if (decl) {
        t->setDeclaration(decl.data());
    }
    return t;
}

template<CXTypeKind TK, EnableIf<TK == CXType_Typedef> = dummy>
AbstractType *makeType(CXType type)
{
    auto t = new TypeAliasType;
    CXCursor location = clang_getTypeDeclaration(type);
    t->setType(build(clang_getTypedefDeclUnderlyingType(location), includes));
    DeclarationPointer decl = findDeclaration(location, includes);
    DUChainReadLocker lock;
    if (decl) {
        t->setDeclaration(decl.data());
    }
    return t;
}

template<CXTypeKind TK, EnableIf<TK == CXType_Int128 || TK == CXType_UInt128 || TK == CXType_Vector || TK == CXType_Unexposed> = dummy>
AbstractType *makeType(CXType type)
{
    auto t = new DelayedType;
    t->setIdentifier(IndexedTypeIdentifier(QString::fromUtf8(ClangString(clang_getTypeSpelling(type)))));
    return t;
}

template<CXTypeKind TK>
AbstractType *dispatchType(CXType type, const IncludeFileContexts& includes)
{
    auto type = makeType<TK>(type);
    setTypeModifiers<TK>(type);
    return type;
}

AbstractType* createType(CXType type, const IncludeFileContexts& includes)
{
    #define UseKind(TypeKind) case TypeKind: return dispatchType<TypeKind>()
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
    case CXType_Invalid:
        return nullptr;
    default:
        debug() << "Unhandled type: " << type.kind << ClangString(clang_getTypeSpelling(type));
        return nullptr;
    }
}

}

namespace TypeBuilder {

AbstractType::Ptr build(CXType type, const IncludeFileContexts& includes)
{
    auto kdevType = createType(type, includes);
    return AbstractType::Ptr(kdevType);
}

}
