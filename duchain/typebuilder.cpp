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

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>

#include "debug.h"

using namespace KDevelop;
using namespace TypeBuilder;

namespace {

AbstractType* createType(CXType type, const IncludeFileContexts& includes)
{
    switch (type.kind) {
        case CXType_Void:
            return new IntegralType(IntegralType::TypeVoid);
        case CXType_Bool:
            return new IntegralType(IntegralType::TypeBoolean);
        case CXType_Short:
        case CXType_UShort:
        case CXType_Int:
        case CXType_UInt:
        case CXType_Long:
        case CXType_ULong:
        case CXType_LongLong:
        case CXType_ULongLong:
            return new IntegralType(IntegralType::TypeInt);
        case CXType_Float:
            return new IntegralType(IntegralType::TypeFloat);
        case CXType_LongDouble:
        case CXType_Double:
            return new IntegralType(IntegralType::TypeDouble);
        case CXType_Char_U:
        case CXType_Char_S:
        case CXType_UChar:
        case CXType_SChar:
            return new IntegralType(IntegralType::TypeChar);
        case CXType_Char16:
            return new IntegralType(IntegralType::TypeChar16_t);
        case CXType_Char32:
            return new IntegralType(IntegralType::TypeChar32_t);
        case CXType_Pointer: {
            auto ptr = new PointerType;
            ptr->setBaseType(build(clang_getPointeeType(type), includes));
            return ptr;
        }
        case CXType_ConstantArray: {
            auto arr = new ArrayType;
            arr->setDimension(clang_getArraySize(type));
            arr->setElementType(build(clang_getArrayElementType(type), includes));
            return arr;
        }
        case CXType_LValueReference:
        case CXType_RValueReference: {
            auto ref = new ReferenceType;
            ref->setIsRValue(type.kind == CXType_RValueReference);
            ref->setBaseType(build(clang_getPointeeType(type), includes));
            return ref;
        }
        case CXType_FunctionProto: {
            auto func = new FunctionType;
            func->setReturnType(build(clang_getResultType(type), includes));
            const int numArgs = clang_getNumArgTypes(type);
            for (int i = 0; i < numArgs; ++i) {
                func->addArgument(build(clang_getArgType(type, i), includes));
            }
            /// TODO: variadic functions
            return func;
        }
        case CXType_Record: {
            auto st = new StructureType;
            DeclarationPointer decl = findDeclaration(clang_getTypeDeclaration(type), includes);
            DUChainReadLocker lock;
            if (decl) {
                st->setDeclaration(decl.data());
            }
            return st;
        }
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
    AbstractType::Ptr ret(createType(type, includes));
    if (!ret) {
        return ret;
    }
    quint64 modifiers = 0;
    if (clang_isConstQualifiedType(type)) {
        modifiers |= AbstractType::ConstModifier;
    }
    if (clang_isVolatileQualifiedType(type)) {
        modifiers |= AbstractType::VolatileModifier;
    }
    if (type.kind == CXType_Short || type.kind == CXType_UShort) {
        modifiers |= AbstractType::ShortModifier;
    }
    if (type.kind == CXType_Long || type.kind == CXType_LongDouble || type.kind == CXType_ULong) {
        modifiers |= AbstractType::LongModifier;
    }
    if (type.kind == CXType_LongLong || type.kind == CXType_ULongLong) {
        modifiers |= AbstractType::LongLongModifier;
    }
    if (type.kind == CXType_SChar) {
        modifiers |= AbstractType::SignedModifier;
    }
    if (type.kind == CXType_UChar || type.kind == CXType_UInt || type.kind == CXType_UShort
        || type.kind == CXType_UInt128 || type.kind == CXType_ULong || type.kind == CXType_ULongLong)
    {
        modifiers |= AbstractType::UnsignedModifier;
    }
    ret->setModifiers(modifiers);
    return ret;
}

}
