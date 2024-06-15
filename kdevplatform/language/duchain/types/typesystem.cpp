/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "typesystem.h"
#include "typeregister.h"
#include "integraltype.h"
#include "structuretype.h"

namespace KDevelop {
AbstractTypeData::AbstractTypeData()
    : m_alignOfExponent(AbstractTypeData::MaxAlignOfExponent)
    , inRepository(false)
{
    initializeAppendedLists(true);
}

uint AbstractTypeData::classSize() const
{
    return TypeSystem::self().dataClassSize(*this);
}

unsigned int AbstractTypeData::itemSize() const
{
    return TypeSystem::self().dynamicSize(*this);
}

size_t AbstractTypeData::hash() const
{
    AbstractType::Ptr type(TypeSystem::self().create(const_cast<AbstractTypeData*>(this)));
    return type->hash();
}

AbstractTypeData::AbstractTypeData(const AbstractTypeData& rhs)
    : m_sizeOf(rhs.m_sizeOf)
    , m_alignOfExponent(rhs.m_alignOfExponent)
    , m_modifiers(rhs.m_modifiers)
    , refCount(0)
    , inRepository(false)
{
    initializeAppendedLists(!rhs.m_dynamic); //This type will be dynamic exactly if the copied one is not.
    typeClassId = rhs.typeClassId;
}

AbstractTypeData::~AbstractTypeData()
{
    freeAppendedLists();
}

IntegralTypeData::IntegralTypeData()

{
}

IntegralTypeData::IntegralTypeData(const IntegralTypeData& rhs)
    : AbstractTypeData(rhs)
    , m_dataType(rhs.m_dataType)
{
}

PointerTypeData::PointerTypeData()
{
}

PointerTypeData::PointerTypeData(const PointerTypeData& rhs)
    : AbstractTypeData(rhs)
    , m_baseType(rhs.m_baseType)
{
}

ReferenceTypeData::ReferenceTypeData() : m_isRValue(false)
{
}

ReferenceTypeData::ReferenceTypeData(const ReferenceTypeData& rhs)
    : AbstractTypeData(rhs)
    , m_baseType(rhs.m_baseType)
    , m_isRValue(rhs.m_isRValue)
{
}

FunctionTypeData::FunctionTypeData()
{
    initializeAppendedLists(m_dynamic);
}

FunctionTypeData::~FunctionTypeData()
{
    freeAppendedLists();
}

FunctionTypeData::FunctionTypeData(const FunctionTypeData& rhs)
    : AbstractTypeData(rhs)
    , m_returnType(rhs.m_returnType)
{
    initializeAppendedLists(m_dynamic);
    copyListsFrom(rhs);
}

void FunctionTypeData::operator=(const FunctionTypeData& rhs)
{
    Q_UNUSED(rhs)
}

StructureTypeData::StructureTypeData()
{
}

StructureTypeData::StructureTypeData(const StructureTypeData& rhs)
    : MergeIdentifiedType<AbstractType>::Data(rhs)
{
}

ConstantIntegralTypeData::ConstantIntegralTypeData()

{
}

ArrayTypeData::ArrayTypeData()

{
}

ArrayTypeData::ArrayTypeData(const ArrayTypeData& rhs)
    : AbstractTypeData(rhs)
    , m_dimension(rhs.m_dimension)
    , m_elementType(rhs.m_elementType)
{
    Q_ASSERT(m_dimension == rhs.m_dimension);
}

DelayedTypeData::DelayedTypeData()
{
}

DelayedTypeData::DelayedTypeData(const DelayedTypeData& rhs)
    : AbstractTypeData(rhs)
    , m_identifier(rhs.m_identifier)
    , m_kind(rhs.m_kind)
{
}

bool SimpleTypeVisitor::preVisit(const AbstractType*)
{
    return true;
}

void SimpleTypeVisitor::postVisit(const AbstractType*)
{
}

void SimpleTypeVisitor::visit(const IntegralType* type)
{
    visit(( AbstractType* )type);
}

bool SimpleTypeVisitor::visit(const PointerType* type)
{
    return visit(( AbstractType* )type);
}

void SimpleTypeVisitor::endVisit(const PointerType* type)
{
    visit(( AbstractType* )type);
}

bool SimpleTypeVisitor::visit(const ReferenceType* type)
{
    return visit(( AbstractType* )type);
}

void SimpleTypeVisitor::endVisit(const ReferenceType* type)
{
    visit(( AbstractType* )type);
}

bool SimpleTypeVisitor::visit(const FunctionType* type)
{
    return visit(( AbstractType* )type);
}

void SimpleTypeVisitor::endVisit(const FunctionType* type)
{
    visit(( AbstractType* )type);
}

bool SimpleTypeVisitor::visit(const StructureType* type)
{
    return visit(( AbstractType* )type);
}

void SimpleTypeVisitor::endVisit(const StructureType* type)
{
    visit(( AbstractType* )type);
}

bool SimpleTypeVisitor::visit(const ArrayType* type)
{
    return visit(( AbstractType* )type);
}

void SimpleTypeVisitor::endVisit(const ArrayType* type)
{
    visit(( AbstractType* )type);
}

TypeVisitor::~TypeVisitor()
{
}

TypePtr<KDevelop::AbstractType> TypeExchanger::exchange(const TypePtr<KDevelop::AbstractType>& type)
{
    const_cast<AbstractType*>(type.data())->exchangeTypes(this);
    return type;
}

TypePtr<KDevelop::AbstractType> SimpleTypeExchanger::exchange(const TypePtr<KDevelop::AbstractType>& type)
{
    if (type && type->equals(m_replace.data()))
        return m_replaceWith;
    else
        return TypeExchanger::exchange(type);
}

SimpleTypeExchanger::SimpleTypeExchanger(const TypePtr<KDevelop::AbstractType>& replace,
                                         const TypePtr<KDevelop::AbstractType>& replaceWith)
    : m_replace(replace)
    , m_replaceWith(replaceWith)
{
}
}
