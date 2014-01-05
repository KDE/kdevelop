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

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/referencetype.h>

using namespace KDevelop;

namespace {

AbstractType::Ptr type(CXType t);

AbstractType* createType(CXType t)
{
    switch (t.kind) {
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
            ptr->setBaseType(type(clang_getPointeeType(t)));
            return ptr;
        }
        case CXType_ConstantArray: {
            auto arr = new ArrayType;
            arr->setDimension(clang_getArraySize(t));
            arr->setElementType(type(clang_getArrayElementType(t)));
            return arr;
        }
        case CXType_LValueReference:
        case CXType_RValueReference: {
            auto ref = new ReferenceType;
            ref->setIsRValue(t.kind == CXType_RValueReference);
            ref->setBaseType(type(clang_getPointeeType(t)));
            return ref;
        }
        case CXType_FunctionProto: {
            auto func = new FunctionType;
            func->setReturnType(type(clang_getResultType(t)));
            const int numArgs = clang_getNumArgTypes(t);
            for (int i = 0; i < numArgs; ++i) {
                func->addArgument(type(clang_getArgType(t, i)));
            }
            /// TODO: variadic functions
            return func;
        }
        default:
            return nullptr;
    }
}

AbstractType::Ptr type(CXType t)
{
    AbstractType::Ptr ret(createType(t));
    if (!ret) {
        return ret;
    }
    quint64 modifiers = 0;
    if (clang_isConstQualifiedType(t)) {
        modifiers |= AbstractType::ConstModifier;
    }
    if (clang_isVolatileQualifiedType(t)) {
        modifiers |= AbstractType::VolatileModifier;
    }
    if (t.kind == CXType_Short || t.kind == CXType_UShort) {
        modifiers |= AbstractType::ShortModifier;
    }
    if (t.kind == CXType_Long || t.kind == CXType_LongDouble || t.kind == CXType_ULong) {
        modifiers |= AbstractType::LongModifier;
    }
    if (t.kind == CXType_LongLong || t.kind == CXType_ULongLong) {
        modifiers |= AbstractType::LongLongModifier;
    }
    if (t.kind == CXType_SChar) {
        modifiers |= AbstractType::SignedModifier;
    }
    if (t.kind == CXType_UChar || t.kind == CXType_UInt || t.kind == CXType_UShort
        || t.kind == CXType_UInt128 || t.kind == CXType_ULong || t.kind == CXType_ULongLong)
    {
        modifiers |= AbstractType::UnsignedModifier;
    }
    ret->setModifiers(modifiers);
    return ret;
}

}

namespace TypeBuilder {

AbstractType::Ptr build(CXCursor cursor)
{
    return type(clang_getCursorType(cursor));
}

}
