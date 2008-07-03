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

namespace KDevelop
{

TypeSystem& TypeSystem::self() {
  static TypeSystem system;
  return system;
}

//REGISTER_TYPE(AbstractType);
REGISTER_TYPE(IntegralType);
REGISTER_TYPE(PointerType);
REGISTER_TYPE(ReferenceType);
REGISTER_TYPE(FunctionType);
REGISTER_TYPE(StructureType);
REGISTER_TYPE(ArrayType);
REGISTER_TYPE(DelayedType);

DEFINE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

AbstractTypeData::AbstractTypeData()
{
  initializeAppendedLists(true);
}

AbstractTypeData::AbstractTypeData( const AbstractTypeData& /*rhs*/ )
{
  initializeAppendedLists(true);
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
{
}

IntegralTypeData::IntegralTypeData( const IntegralTypeData& rhs )
  : AbstractTypeData(rhs), m_name( rhs.m_name )
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
  initializeAppendedLists(true);
}

FunctionTypeData::~FunctionTypeData() {
  freeAppendedLists();
}

FunctionTypeData::FunctionTypeData( const FunctionTypeData& rhs )
  : AbstractTypeData( rhs ), m_returnType( rhs.m_returnType)
{
  initializeAppendedLists(true);
  copyListsFrom(rhs);
}

void FunctionTypeData::operator=(const FunctionTypeData& rhs) {
}

StructureTypeData::StructureTypeData()
{
}

StructureTypeData::StructureTypeData( const StructureTypeData& rhs )
  : AbstractTypeData( rhs )
{
}

ArrayTypeData::ArrayTypeData()
{
}

ArrayTypeData::ArrayTypeData( const ArrayTypeData& rhs )
  : AbstractTypeData( rhs ), m_dimension( rhs.m_dimension ),
    m_elementType( rhs.m_elementType )
{
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

void AbstractType::makeDynamic() {
  ///@todo make dynamic here
}

TypeVisitor::~TypeVisitor()
{
}

AbstractType::AbstractType(const AbstractType& rhs) : KShared(), d_ptr(new AbstractTypeData(*rhs.d_ptr)) {
}

IntegralType::IntegralType(const IntegralType& rhs) : AbstractType(*new IntegralTypeData(*rhs.d_func())) {
}

PointerType::PointerType(const PointerType& rhs) : AbstractType(*new PointerTypeData(*rhs.d_func())) {
}

ReferenceType::ReferenceType(const ReferenceType& rhs) : AbstractType(*new ReferenceTypeData(*rhs.d_func())) {
}

FunctionType::FunctionType(const FunctionType& rhs) : AbstractType(*new FunctionTypeData(*rhs.d_func())) {
}

StructureType::StructureType(const StructureType& rhs) : AbstractType(*new StructureTypeData(*rhs.d_func())) {
}

ArrayType::ArrayType(const ArrayType& rhs) : AbstractType(*new ArrayTypeData(*rhs.d_func())) {
}

AbstractType::AbstractType( AbstractTypeData& dd )
  : d_ptr(&dd)
{
}

IntegralType::IntegralType(IntegralTypeData& data) : AbstractType(data) {
}

PointerType::PointerType(PointerTypeData& data) : AbstractType(data) {
}

ReferenceType::ReferenceType(ReferenceTypeData& data) : AbstractType(data) {
}

FunctionType::FunctionType(FunctionTypeData& data) : AbstractType(data) {
}

StructureType::StructureType(StructureTypeData& data) : AbstractType(data) {
}

ArrayType::ArrayType(ArrayTypeData& data) : AbstractType(data) {
}

DelayedType::DelayedType(DelayedTypeData& data) : AbstractType(data) {
}

AbstractType* IntegralType::clone() const {
  return new IntegralType(*this);
}

AbstractType* PointerType::clone() const {
  return new PointerType(*this);
}

AbstractType* ReferenceType::clone() const {
  return new ReferenceType(*this);
}

AbstractType* FunctionType::clone() const {
  return new FunctionType(*this);
}

AbstractType* StructureType::clone() const {
  return new StructureType(*this);
}

AbstractType* ArrayType::clone() const {
  return new ArrayType(*this);
}

AbstractType* DelayedType::clone() const {
  return new DelayedType(*this);
}

AbstractType::Ptr IndexedType::type() const {
  if(!m_index)
    return AbstractType::Ptr();
  return TypeRepository::typeForIndex(m_index);
}


IndexedType AbstractType::indexed() const {
  if(this == 0)
    return IndexedType();
  else
    return IndexedType(TypeRepository::indexForType(AbstractType::Ptr(const_cast<AbstractType*>(this))));
}


bool IntegralType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const IntegralType*>(_rhs))
    return false;
  const IntegralType* rhs = static_cast<const IntegralType*>(_rhs);
  
  return rhs->d_func()->m_name == d_func()->m_name;
}

bool PointerType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const PointerType*>(_rhs))
    return false;
  const PointerType* rhs = static_cast<const PointerType*>(_rhs);

  TYPE_D(PointerType);
  if( (bool)rhs->d_func()->m_baseType != (bool)d->m_baseType )
    return false;
  
  if( !d->m_baseType )
    return true;
  
  return rhs->d_func()->m_baseType.type()->equals(d->m_baseType.type().data());
}

bool ReferenceType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const ReferenceType*>(_rhs))
    return false;
  const ReferenceType* rhs = static_cast<const ReferenceType*>(_rhs);

  TYPE_D(ReferenceType);
  if( (bool)rhs->d_func()->m_baseType != (bool)d->m_baseType )
    return false;
  
  if( !d->m_baseType )
    return true;
  
  return rhs->d_func()->m_baseType.type()->equals(d->m_baseType.type().data());
}

bool FunctionType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const FunctionType*>(_rhs))
    return false;
  const FunctionType* rhs = static_cast<const FunctionType*>(_rhs);

  TYPE_D(FunctionType);
  if( d->m_argumentsSize() != rhs->d_func()->m_argumentsSize() )
    return false;
  
  if( (bool)rhs->d_func()->m_returnType != (bool)d->m_returnType )
    return false;
  
  if( d->m_returnType )
    if( !rhs->d_func()->m_returnType.type()->equals(d->m_returnType.type().data()) )
      return false;
  
  for(int a = 0; a < d->m_argumentsSize(); ++a) {
    if( (bool)d->m_arguments()[a] != (bool)rhs->d_func()->m_arguments()[a] )
      return false;
    
    if( !d->m_arguments()[a])
      continue;

    if( !d->m_arguments()[a].type()->equals( rhs->d_func()->m_arguments()[a].type().data() ) )
      return false;
  }

  return true;
}

bool StructureType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const StructureType*>(_rhs))
    return false;
//   const StructureType* rhs = static_cast<const StructureType*>(_rhs);
//   
//   TYPE_D(StructureType);

  return true;
}

bool ArrayType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const ArrayType*>(_rhs))
    return false;
  const ArrayType* rhs = static_cast<const ArrayType*>(_rhs);
  TYPE_D(ArrayType);
  if( d->m_dimension != rhs->d_func()->m_dimension )
    return false;
  
  if( (bool)rhs->d_func()->m_elementType != (bool)d->m_elementType )
    return false;

  if( rhs->d_func()->m_dimension != (bool)d->m_dimension )
    return false;
  
  if( !d->m_elementType )
    return true;
  
  return rhs->d_func()->m_elementType.type()->equals(d->m_elementType.type().data());
}

bool DelayedType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const DelayedType*>(_rhs))
    return false;

  const DelayedType* rhs = static_cast<const DelayedType*>(_rhs);

  return d_func()->m_identifier == rhs->d_func()->m_identifier && rhs->d_func()->m_kind == d_func()->m_kind;
}

AbstractType::AbstractType()
  : d_ptr(new AbstractTypeData)
{
}

AbstractType::~AbstractType()
{
  delete d_ptr;
}

void AbstractType::accept(TypeVisitor *v) const
{
  if (v->preVisit (this))
    this->accept0 (v);

  v->postVisit (this);
}

void AbstractType::acceptType(AbstractType::Ptr type, TypeVisitor *v)
{
  if (! type)
    return;

  type->accept (v);
}

AbstractType::WhichType AbstractType::whichType() const
{
  return TypeAbstract;
}

IntegralType::IntegralType(const IndexedString& name)
  : AbstractType(*new IntegralTypeData)
{
  d_func_dynamic()->m_name = name;
}

IntegralType::IntegralType()
  : AbstractType(*new IntegralTypeData)
{
}

IntegralType::~IntegralType()
{
}

const IndexedString& IntegralType::name() const
{
  return d_func()->m_name;
}

void IntegralType::setName(const IndexedString& name)
{
  d_func_dynamic()->m_name = name;
}

bool IntegralType::operator == (const IntegralType &other) const
{
  return d_func()->m_name == other.d_func()->m_name;
}

bool IntegralType::operator != (const IntegralType &other) const
{
  return d_func()->m_name != other.d_func()->m_name;
}

QString IntegralType::toString() const
{
  return d_func()->m_name.str();
}

void IntegralType::accept0(TypeVisitor *v) const
{
  v->visit (this);
}

AbstractType::WhichType IntegralType::whichType() const
{
  return TypeIntegral;
}

PointerType::PointerType()
  : AbstractType(*new PointerTypeData)
{
}

void PointerType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d_func()->m_baseType.type(), v);

  v->endVisit (this);
}

void PointerType::exchangeTypes( TypeExchanger* exchanger ) {
  d_func_dynamic()->m_baseType = exchanger->exchange( d_func()->m_baseType.type().data() )->indexed();
}

PointerType::~PointerType()
{
}

AbstractType::Ptr PointerType::baseType () const
{
  return d_func()->m_baseType.type();
}

void PointerType::setBaseType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_baseType = type->indexed();
}

bool PointerType::operator == (const PointerType &other) const
{
  return d_func()->m_baseType == other.d_func()->m_baseType;
}

bool PointerType::operator != (const PointerType &other) const
{
  return d_func()->m_baseType != other.d_func()->m_baseType;
}

QString PointerType::toString() const
{
  return baseType() ? QString("%1*").arg(baseType()->toString()) : QString("<notype>*");
}

AbstractType::WhichType PointerType::whichType() const
{
  return TypePointer;
}

ReferenceType::ReferenceType()
  : AbstractType(*new ReferenceTypeData)
{
}

ReferenceType::~ReferenceType()
{
}

AbstractType::Ptr ReferenceType::baseType () const
{
  return d_func()->m_baseType.type();
}

void ReferenceType::setBaseType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_baseType = type->indexed();
}

bool ReferenceType::operator == (const ReferenceType &other) const
{
  return d_func()->m_baseType == other.d_func()->m_baseType;
}

bool ReferenceType::operator != (const ReferenceType &other) const
{
  return d_func()->m_baseType != other.d_func()->m_baseType;
}

void ReferenceType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d_func()->m_baseType.type(), v);

  v->endVisit (this);
}

void ReferenceType::exchangeTypes( TypeExchanger* exchanger )
{
  d_func_dynamic()->m_baseType = exchanger->exchange( d_func()->m_baseType.type().data() )->indexed();
}

QString ReferenceType::toString() const
{
  return baseType() ? QString("%1&").arg(baseType()->toString()) : QString("<notype>&");
}

AbstractType::WhichType ReferenceType::whichType() const
{
  return TypeReference;
}

FunctionType::FunctionType()
  : AbstractType(*new FunctionTypeData)
{
}

FunctionType::~FunctionType()
{
}


void FunctionType::addArgument(AbstractType::Ptr argument)
{
  d_func_dynamic()->m_argumentsList().append(argument->indexed());
}

void FunctionType::removeArgument(AbstractType::Ptr argument)
{
  TYPE_D_DYNAMIC(FunctionType);
  
  IndexedType i = argument->indexed();
  uint shift = 0;
  for(int a = 0; a < d->m_argumentsSize(); ++a) {
    if(d->m_arguments()[a] == i) {
      ++shift;
    }else if(shift) {
      d->m_argumentsList()[a-shift] = d->m_argumentsList()[a];
    }
  }
  d->m_argumentsList().resize(d->m_argumentsSize()-shift);
}

void FunctionType::setReturnType(AbstractType::Ptr returnType)
{
  d_func_dynamic()->m_returnType = returnType->indexed();
}

AbstractType::Ptr FunctionType::returnType () const
{
  return d_func()->m_returnType.type();
}

QList<AbstractType::Ptr> FunctionType::arguments () const
{
  ///@todo Don't do the conversion
  QList<AbstractType::Ptr> ret;
  FOREACH_FUNCTION(IndexedType arg, d_func()->m_arguments)
    ret << arg.type();
  return ret;
}

bool FunctionType::operator == (const FunctionType &other) const
{
  TYPE_D(FunctionType);
  return d->m_returnType == other.d_func()->m_returnType && d->listsEqual(*other.d_func());
}

bool FunctionType::operator != (const FunctionType &other) const
{
  TYPE_D(FunctionType);
  return d->m_returnType != other.d_func()->m_returnType || !d->listsEqual(*other.d_func());
}

void AbstractType::exchangeTypes( TypeExchanger* /*exchanger */) {
}

void FunctionType::accept0 (TypeVisitor *v) const
{
  TYPE_D(FunctionType);
  if (v->visit (this))
  {
    acceptType (d->m_returnType.type(), v);

    for (int i = 0; i < d->m_argumentsSize (); ++i)
      acceptType (d->m_arguments()[i].type(), v);
  }

  v->endVisit (this);
}

void FunctionType::exchangeTypes( TypeExchanger* exchanger )
{
  TYPE_D_DYNAMIC(FunctionType);
  for (int i = 0; i < d->m_argumentsSize (); ++i)
    d->m_argumentsList()[i] = exchanger->exchange( d->m_arguments()[i].type().data() )->indexed();
  d->m_returnType = exchanger->exchange(d->m_returnType.type().data())->indexed();
}

QString FunctionType::partToString( SignaturePart sigPart ) const {
  QString args;
  TYPE_D(FunctionType);
  if( sigPart == SignatureArguments || sigPart == SignatureWhole )
  {
    args += '(';
    bool first = true;
    FOREACH_FUNCTION(const IndexedType& type, d->m_arguments) {
      if (first)
        first = false;
      else
        args.append(", ");
      args.append(type ? type.type()->toString() : QString("<notype>"));
    }
    args += ')';
  }
  
  if( sigPart == SignatureArguments )
    return args;
  else if( sigPart == SignatureWhole )
    return QString("function %1 %2").arg(returnType() ? returnType()->toString() : QString("<notype>")).arg(args);
  else if( sigPart == SignatureReturn )
    return returnType() ? returnType()->toString() : QString();

  return QString("ERROR");
}

QString FunctionType::toString() const
{
  return partToString(SignatureWhole);
}

AbstractType::WhichType FunctionType::whichType() const
{
  return TypeFunction;
}

StructureType::StructureType()
  : AbstractType(*new StructureTypeData)
{
}

StructureType::~StructureType()
{
}

void StructureType::accept0 (TypeVisitor *v) const
{
//   TYPE_D(StructureType);
  v->visit (this);

  v->endVisit (this);
}

QString StructureType::toString() const
{
  return "<structure>";
}

AbstractType::WhichType StructureType::whichType() const
{
  return TypeStructure;
}

ArrayType::ArrayType()
  : AbstractType(*new ArrayTypeData)
{
}

ArrayType::~ArrayType()
{
}

int ArrayType::dimension () const
{
  return d_func()->m_dimension;
}

void ArrayType::setDimension(int dimension)
{
  kDebug() << "setting dimension" << dimension;
  d_func_dynamic()->m_dimension = dimension;
  kDebug() << "dimension" << d_func()->m_dimension;
}

AbstractType::Ptr ArrayType::elementType () const
{
  return d_func()->m_elementType.type();
}

void ArrayType::setElementType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_elementType = type->indexed();
}

bool ArrayType::operator == (const ArrayType &other) const
{
  TYPE_D(ArrayType);
  return d->m_elementType == other.d_func()->m_elementType && d->m_dimension == other.d_func()->m_dimension;
}

bool ArrayType::operator != (const ArrayType &other) const
{
  TYPE_D(ArrayType);
  return d->m_elementType != other.d_func()->m_elementType || d->m_dimension != other.d_func()->m_dimension;
}

QString ArrayType::toString() const
{
  return QString("%1[%2]").arg(elementType() ? elementType()->toString() : QString("<notype>")).arg(d_func()->m_dimension);
}

void ArrayType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    {
      acceptType (d_func()->m_elementType.type(), v);
    }

  v->endVisit (this);
}

void ArrayType::exchangeTypes( TypeExchanger* exchanger )
{
  TYPE_D_DYNAMIC(ArrayType);
  d->m_elementType = exchanger->exchange( d->m_elementType.type().data() )->indexed();
}

AbstractType::WhichType ArrayType::whichType() const
{
  return TypeArray;
}

AbstractType::WhichType DelayedType::whichType() const
{
  return AbstractType::TypeDelayed;
}

QString DelayedType::toString() const
{
  return (d_func()->m_kind == Delayed ? "<delayed> " : "<unresolved> ") + identifier().toString();
}

DelayedType::Kind DelayedType::kind() const {
  return d_func()->m_kind;
}

void DelayedType::setKind(Kind kind) {
  d_func_dynamic()->m_kind = kind;
}

DelayedType::DelayedType()
  : AbstractType(*new DelayedTypeData)
{
}

DelayedType::DelayedType(const DelayedType& rhs) : AbstractType(*new DelayedTypeData(*rhs.d_func())) {
}

DelayedType::~DelayedType()
{
}

void DelayedType::setIdentifier(const TypeIdentifier& identifier)
{
  d_func_dynamic()->m_identifier = identifier;
}

TypeIdentifier DelayedType::identifier() const
{
  return d_func()->m_identifier;
}

void DelayedType::accept0 (KDevelop::TypeVisitor *v) const
{
    v->visit(this);
/*    v->endVisit(this);*/
}

uint DelayedType::hash() const
{
  return 37*(d_func()->m_identifier.identifier().hash() + (uint)d_func()->m_kind);
}

uint PointerType::hash() const
{
  return (baseType() ? baseType()->hash() : 0) * 13;
}

uint IntegralType::hash() const
{
  return qHash(d_func()->m_name) * 17;
}

uint ReferenceType::hash() const
{
  return (baseType() ? baseType()->hash() : 1) * 29;
}

uint FunctionType::hash() const
{
  uint hash_value = 0;
  if(returnType())
    hash_value += returnType()->hash();

  FOREACH_FUNCTION(IndexedType t, d_func()->m_arguments) {
    hash_value = (hash_value << 5) - hash_value + t.hash();
  }

  return hash_value;
}

uint StructureType::hash() const
{
  uint hash_value = 101;

  return hash_value;
}

uint ArrayType::hash() const
{
  return (elementType() ? elementType()->hash() : 0) * 47 + 117* dimension();
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
