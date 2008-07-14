/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "typesystem.h"
#include "indexedstring.h"
#include "repositories/typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"

#include "integraltype.h"
#include "structuretype.h"

namespace KDevelop
{

AbstractTypeData::AbstractTypeData()
  : m_modifiers(CommonModifiers::NoModifiers)
  , inRepository(false)
{
  initializeAppendedLists(true);
}

uint AbstractTypeData::classSize() const {
  return TypeSystem::self().dataClassSize(*this);
}

AbstractTypeData::AbstractTypeData( const AbstractTypeData& rhs )
  : m_modifiers(rhs.m_modifiers)
  , inRepository(false)
{
  initializeAppendedLists(!rhs.m_dynamic); //This type will be dynamic exactly if the copied one is not.
  typeClassId = rhs.typeClassId;
  m_modifiers = rhs.m_modifiers;
}

AbstractTypeData::~AbstractTypeData()
{
  freeAppendedLists();
}

AbstractTypeData& AbstractTypeData::operator=(const AbstractTypeData&) {
  Q_ASSERT(0);
  return *this;
}

IntegralTypeData::IntegralTypeData()
  : m_dataType(CommonIntegralTypes::TypeNone)
{
}

IntegralTypeData::IntegralTypeData( const IntegralTypeData& rhs )
  : AbstractTypeData(rhs), m_name( rhs.m_name ), m_dataType(CommonIntegralTypes::TypeNone)
{
}

PointerTypeData::PointerTypeData() : m_baseType(0)
{
}

PointerTypeData::PointerTypeData( const PointerTypeData& rhs )
  : AbstractTypeData(rhs), m_baseType( rhs.m_baseType )
{
}

ReferenceTypeData::ReferenceTypeData() : m_baseType(0)
{
}

ReferenceTypeData::ReferenceTypeData( const ReferenceTypeData& rhs )
  : AbstractTypeData( rhs ), m_baseType( rhs.m_baseType )
{
}

FunctionTypeData::FunctionTypeData()
{
  initializeAppendedLists(m_dynamic);
}

FunctionTypeData::~FunctionTypeData() {
  freeAppendedLists();
}

FunctionTypeData::FunctionTypeData( const FunctionTypeData& rhs )
  : AbstractTypeData( rhs ), m_returnType( rhs.m_returnType)
{
  initializeAppendedLists(m_dynamic);
  copyListsFrom(rhs);
}

void FunctionTypeData::operator=(const FunctionTypeData& rhs) {
}

StructureTypeData::StructureTypeData()
  : m_classType(CommonClassTypes::Class)
  , m_closed(false)
{
}

StructureTypeData::StructureTypeData( const StructureTypeData& rhs )
  : AbstractTypeData( rhs )
  , m_classType(rhs.m_classType)
  , m_closed(rhs.m_closed)
{
}

ArrayTypeData::ArrayTypeData()
{
}

ArrayTypeData::ArrayTypeData( const ArrayTypeData& rhs )
  : AbstractTypeData( rhs ), m_dimension( rhs.m_dimension ),
    m_elementType( rhs.m_elementType )
{
  Q_ASSERT(m_dimension == rhs.m_dimension);
}

DelayedTypeData::DelayedTypeData() : m_kind(DelayedType::Delayed) {
}

DelayedTypeData::DelayedTypeData( const DelayedTypeData& rhs )
  : AbstractTypeData( rhs ), m_identifier( rhs.m_identifier ),
    m_kind( rhs.m_kind )
{
}

bool SimpleTypeVisitor::preVisit (const AbstractType *) {
  return true;
}

void SimpleTypeVisitor::postVisit (const AbstractType *) {
}

void SimpleTypeVisitor::visit (const IntegralType * type) {
  visit( (AbstractType*)type );
}

bool SimpleTypeVisitor::visit (const PointerType * type) {
  return visit( (AbstractType*)type );
}

void SimpleTypeVisitor::endVisit (const PointerType * type) {
  visit( (AbstractType*)type );
}

bool SimpleTypeVisitor::visit (const ReferenceType * type) {
  return visit( (AbstractType*)type );
}

void SimpleTypeVisitor::endVisit (const ReferenceType * type) {
  visit( (AbstractType*)type );
}

bool SimpleTypeVisitor::visit (const FunctionType * type) {
  return visit( (AbstractType*)type );
}

void SimpleTypeVisitor::endVisit (const FunctionType * type) {
  visit( (AbstractType*)type );
}

bool SimpleTypeVisitor::visit (const StructureType * type) {
  return visit( (AbstractType*)type );
}

void SimpleTypeVisitor::endVisit (const StructureType * type) {
  visit( (AbstractType*)type );
}

bool SimpleTypeVisitor::visit (const ArrayType * type) {
  return visit( (AbstractType*)type );
}

void SimpleTypeVisitor::endVisit (const ArrayType * type) {
  visit( (AbstractType*)type );
}

TypeVisitor::~TypeVisitor()
{
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
