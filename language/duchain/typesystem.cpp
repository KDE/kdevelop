/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

namespace KDevelop
{

class AbstractTypePrivate
{
public:
  AbstractTypePrivate() : m_registered( false )
  {
  }
  AbstractTypePrivate( const AbstractTypePrivate& rhs )
    : m_registered( rhs.m_registered )
  {
  }
  bool m_registered;
};

class IntegralTypePrivate : public AbstractTypePrivate
{
public:
  IntegralTypePrivate()
  {
  }
  IntegralTypePrivate( const IntegralTypePrivate& rhs )
    : AbstractTypePrivate(rhs), m_name( rhs.m_name )
  {
  }
  QString m_name;
};

class PointerTypePrivate : public AbstractTypePrivate
{
public:
  PointerTypePrivate() : m_baseType(0)
  {
  }
  PointerTypePrivate( const PointerTypePrivate& rhs )
    : AbstractTypePrivate(rhs), m_baseType( rhs.m_baseType )
  {
  }
  AbstractType::Ptr m_baseType;
};

class ReferenceTypePrivate : public AbstractTypePrivate
{
public:
  ReferenceTypePrivate() : m_baseType(0)
  {
  }
  ReferenceTypePrivate( const ReferenceTypePrivate& rhs )
    : AbstractTypePrivate( rhs ), m_baseType( rhs.m_baseType )
  {
  }
  AbstractType::Ptr m_baseType;
};

class FunctionTypePrivate : public AbstractTypePrivate
{
public:
  FunctionTypePrivate()
  {}
  FunctionTypePrivate( const FunctionTypePrivate& rhs )
    : AbstractTypePrivate( rhs ), m_returnType( rhs.m_returnType),
      m_arguments( rhs.m_arguments)
  {
  }
  AbstractType::Ptr m_returnType;
  QList<AbstractType::Ptr> m_arguments;
};

class StructureTypePrivate : public AbstractTypePrivate
{
public:
  StructureTypePrivate()
  {
  }
  StructureTypePrivate( const StructureTypePrivate& rhs )
    : AbstractTypePrivate( rhs ), m_elements( rhs.m_elements )
  {
  }
  QList<AbstractType::Ptr> m_elements;
};

class ArrayTypePrivate : public AbstractTypePrivate
{
public:
  ArrayTypePrivate()
  {
  }
  ArrayTypePrivate( const ArrayTypePrivate& rhs )
    : AbstractTypePrivate( rhs ), m_dimension( rhs.m_dimension ),
      m_elementType( rhs.m_elementType )
  {
  }
  int m_dimension;
  AbstractType::Ptr m_elementType;
};

class DelayedTypePrivate : public AbstractTypePrivate
{
public:
  DelayedTypePrivate() : m_kind(DelayedType::Delayed) {
  }
  DelayedTypePrivate( const DelayedTypePrivate& rhs )
    : AbstractTypePrivate( rhs ), m_identifier( rhs.m_identifier ), 
      m_kind( rhs.m_kind )
  {
  }
  TypeIdentifier m_identifier;
  DelayedType::Kind m_kind;
};


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

AbstractType::AbstractType(const AbstractType& rhs) : KShared(), d_ptr(new AbstractTypePrivate(*rhs.d_ptr)) {
}

AbstractType::AbstractType( AbstractTypePrivate& dd )
  : d_ptr(&dd)
{
}

IntegralType::IntegralType(const IntegralType& rhs) : AbstractType(*new IntegralTypePrivate(*rhs.d_func())) {
}

PointerType::PointerType(const PointerType& rhs) : AbstractType(*new PointerTypePrivate(*rhs.d_func())) {
}

ReferenceType::ReferenceType(const ReferenceType& rhs) : AbstractType(*new ReferenceTypePrivate(*rhs.d_func())) {
}

FunctionType::FunctionType(const FunctionType& rhs) : AbstractType(*new FunctionTypePrivate(*rhs.d_func())) {
}

StructureType::StructureType(const StructureType& rhs) : AbstractType(*new StructureTypePrivate(*rhs.d_func())) {
}

ArrayType::ArrayType(const ArrayType& rhs) : AbstractType(*new ArrayTypePrivate(*rhs.d_func())) {
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

bool IntegralType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const IntegralType*>(_rhs))
    return false;
  const IntegralType* rhs = static_cast<const IntegralType*>(_rhs);
  
  return rhs->d_func()->m_name == d_func()->m_name;
}

bool PointerType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const PointerType*>(_rhs))
    return false;
  const PointerType* rhs = static_cast<const PointerType*>(_rhs);

  Q_D(const PointerType);
  if( (bool)rhs->d_func()->m_baseType != (bool)d->m_baseType )
    return false;
  
  if( !d->m_baseType )
    return true;
  
  return rhs->d_func()->m_baseType->equals(d->m_baseType.data());
}

bool ReferenceType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const ReferenceType*>(_rhs))
    return false;
  const ReferenceType* rhs = static_cast<const ReferenceType*>(_rhs);

  Q_D(const ReferenceType);
  if( (bool)rhs->d_func()->m_baseType != (bool)d->m_baseType )
    return false;
  
  if( !d->m_baseType )
    return true;
  
  return rhs->d_func()->m_baseType->equals(d->m_baseType.data());
}

bool FunctionType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const FunctionType*>(_rhs))
    return false;
  const FunctionType* rhs = static_cast<const FunctionType*>(_rhs);

  Q_D(const FunctionType);
  if( d->m_arguments.count() != rhs->d_func()->m_arguments.count() )
    return false;
  
  if( (bool)rhs->d_func()->m_returnType != (bool)d->m_returnType )
    return false;
  
  if( d->m_returnType )
    if( !rhs->d_func()->m_returnType->equals(d->m_returnType.data()) )
      return false;
  
  QList<AbstractType::Ptr>::const_iterator it1 = d->m_arguments.begin();
  QList<AbstractType::Ptr>::const_iterator it2 = rhs->d_func()->m_arguments.begin();
  
  for( ;it1 != d->m_arguments.end(); ++it1, ++it2 ) {
    if( (bool)*it1 != (bool)*it2 )
      return false;
    
    if( !*it1)
      continue;

    if( !(*it1)->equals( (*it2).data() ) )
      return false;
  }

  return true;
}

bool StructureType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const StructureType*>(_rhs))
    return false;
  const StructureType* rhs = static_cast<const StructureType*>(_rhs);
  Q_D(const StructureType);
  if( d->m_elements.count() != rhs->d_func()->m_elements.count() )
    return false;
  
  QList<AbstractType::Ptr>::const_iterator it1 = d->m_elements.begin();
  QList<AbstractType::Ptr>::const_iterator it2 = rhs->d_func()->m_elements.begin();
  
  for( ;it1 != d->m_elements.end(); ++it1, ++it2 ) {
    if( (bool)*it1 != (bool)*it2 )
      return false;
    
    if( !*it1)
      continue;

    if( !(*it1)->equals( (*it2).data() ) )
      return false;
  }

  return true;
}

bool ArrayType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const ArrayType*>(_rhs))
    return false;
  const ArrayType* rhs = static_cast<const ArrayType*>(_rhs);
  Q_D(const ArrayType);
  if( d->m_dimension != rhs->d_func()->m_dimension )
    return false;
  
  if( (bool)rhs->d_func()->m_elementType != (bool)d->m_elementType )
    return false;
  
  if( !d->m_elementType )
    return true;
  
  return rhs->d_func()->m_elementType->equals(d->m_elementType.data());
}

bool DelayedType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const DelayedType*>(_rhs))
    return false;

  const DelayedType* rhs = static_cast<const DelayedType*>(_rhs);

  return d_func()->m_identifier == rhs->d_func()->m_identifier;
}

AbstractType::AbstractType()
  : d_ptr(new AbstractTypePrivate)
{
}

AbstractType::~AbstractType()
{
//  Q_ASSERT(!m_registered);
  delete d_ptr;
}

uint AbstractType::hash() const
{
  return static_cast<uint>(reinterpret_cast<long>(this));
}

QString AbstractType::mangled() const
{
  return QString();
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

IntegralType::IntegralType(const QString & name)
  : AbstractType(*new IntegralTypePrivate)
{
  d_func()->m_name = name;
}

IntegralType::IntegralType()
  : AbstractType(*new IntegralTypePrivate)
{
}

IntegralType::~IntegralType()
{
}

const QString& IntegralType::name() const
{
  return d_func()->m_name;
}

void IntegralType::setName(const QString& name)
{
  d_func()->m_name = name;
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
  return d_func()->m_name;
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
  : AbstractType(*new PointerTypePrivate)
{
}

void PointerType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d_func()->m_baseType, v);

  v->endVisit (this);
}

void PointerType::exchangeTypes( TypeExchanger* exchanger ) {
  d_func()->m_baseType = exchanger->exchange( d_func()->m_baseType.data() );
}

PointerType::~PointerType()
{
}

AbstractType::Ptr PointerType::baseType () const
{
  return d_func()->m_baseType;
}

void PointerType::setBaseType(AbstractType::Ptr type)
{
  d_func()->m_baseType = type;
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
  : AbstractType(*new ReferenceTypePrivate)
{
}

ReferenceType::~ReferenceType()
{
}

AbstractType::Ptr ReferenceType::baseType () const
{
  return d_func()->m_baseType;
}

void ReferenceType::setBaseType(AbstractType::Ptr type)
{
  d_func()->m_baseType = type;
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
    acceptType (d_func()->m_baseType, v);

  v->endVisit (this);
}

void ReferenceType::exchangeTypes( TypeExchanger* exchanger )
{
  d_func()->m_baseType = exchanger->exchange( d_func()->m_baseType.data() );
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
  : AbstractType(*new FunctionTypePrivate)
{
}

FunctionType::~FunctionType()
{
}


void FunctionType::addArgument(AbstractType::Ptr argument)
{
  d_func()->m_arguments.append(argument);
}

void FunctionType::removeArgument(AbstractType::Ptr argument)
{
  d_func()->m_arguments.removeAll(argument);
}

void FunctionType::setReturnType(AbstractType::Ptr returnType)
{
  d_func()->m_returnType = returnType;
}

AbstractType::Ptr FunctionType::returnType () const
{
  return d_func()->m_returnType;
}

const QList<AbstractType::Ptr>& FunctionType::arguments () const
{
  return d_func()->m_arguments;
}

bool FunctionType::operator == (const FunctionType &other) const
{
  Q_D(const FunctionType);
  return d->m_returnType == other.d_func()->m_returnType && d->m_arguments == other.d_func()->m_arguments;
}

bool FunctionType::operator != (const FunctionType &other) const
{
  Q_D(const FunctionType);
  return d->m_returnType != other.d_func()->m_returnType || d->m_arguments != other.d_func()->m_arguments;
}

void AbstractType::exchangeTypes( TypeExchanger* /*exchanger */) {
}

void FunctionType::accept0 (TypeVisitor *v) const
{
  Q_D(const FunctionType);
  if (v->visit (this))
  {
    acceptType (d->m_returnType, v);

    for (int i = 0; i < d->m_arguments.count (); ++i)
      acceptType (d->m_arguments.at (i), v);
  }

  v->endVisit (this);
}

void FunctionType::exchangeTypes( TypeExchanger* exchanger )
{
  Q_D(FunctionType);
  for (int i = 0; i < d->m_arguments.count (); ++i)
    d->m_arguments[i] = exchanger->exchange( d->m_arguments[i].data() );
  d->m_returnType = exchanger->exchange(d->m_returnType.data());
}

QString FunctionType::partToString( SignaturePart sigPart ) const {
  QString args;
  Q_D(const FunctionType);
  if( sigPart == SignatureArguments || sigPart == SignatureWhole )
  {
    args += '(';
    bool first = true;
    foreach (const AbstractType::Ptr& type, d->m_arguments) {
      if (first)
        first = false;
      else
        args.append(", ");
      args.append(type ? type->toString() : QString("<notype>"));
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
  : AbstractType(*new StructureTypePrivate)
{
}

StructureType::~StructureType()
{
}

const QList<AbstractType::Ptr>& StructureType::elements () const
{
  return d_func()->m_elements;
}

bool StructureType::operator == (const StructureType &other) const
{
  return d_func()->m_elements == other.d_func()->m_elements;
}

bool StructureType::operator != (const StructureType &other) const
{
  return d_func()->m_elements != other.d_func()->m_elements;
}

void StructureType::addElement(AbstractType::Ptr element)
{
  d_func()->m_elements.append(element);
}

void StructureType::removeElement(AbstractType::Ptr element)
{
  d_func()->m_elements.removeAll(element);
}

void StructureType::clear() {
  d_func()->m_elements.clear();
}

void StructureType::accept0 (TypeVisitor *v) const
{
  Q_D(const StructureType);
  if (v->visit (this))
    {
      for (int i = 0; i < d->m_elements.count (); ++i)
        acceptType (d->m_elements.at (i), v);
    }

  v->endVisit (this);
}

void StructureType::exchangeTypes( TypeExchanger* exchanger )
{
  Q_D(StructureType);
  if( exchanger->exchangeMembers() )
    for (int i = 0; i < d->m_elements.count (); ++i)
      d->m_elements[i] = exchanger->exchange( d->m_elements[i].data() );
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
  : AbstractType(*new ArrayTypePrivate)
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
  d_func()->m_dimension = dimension;
}

AbstractType::Ptr ArrayType::elementType () const
{
  return d_func()->m_elementType;
}

void ArrayType::setElementType(AbstractType::Ptr type)
{
  d_func()->m_elementType = type;
}

bool ArrayType::operator == (const ArrayType &other) const
{
  Q_D(const ArrayType);
  return d->m_elementType == other.d_func()->m_elementType && d->m_dimension == other.d_func()->m_dimension;
}

bool ArrayType::operator != (const ArrayType &other) const
{
  Q_D(const ArrayType);
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
      acceptType (d_func()->m_elementType, v);
    }

  v->endVisit (this);
}

void ArrayType::exchangeTypes( TypeExchanger* exchanger )
{
  Q_D(ArrayType);
  d->m_elementType = exchanger->exchange( d->m_elementType.data() );
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
  d_func()->m_kind = kind;
}

DelayedType::DelayedType()
  : AbstractType(*new DelayedTypePrivate)
{
}

DelayedType::~DelayedType()
{
}

void DelayedType::setIdentifier(const TypeIdentifier& identifier)
{
  d_func()->m_identifier = identifier;
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
  return d_func()->m_identifier.hash();
}

uint PointerType::hash() const
{
  return baseType()->hash() * 13;
}

uint IntegralType::hash() const
{
  return qHash(d_func()->m_name) * 17;
}

uint ReferenceType::hash() const
{
  return baseType()->hash() * 29;
}

uint FunctionType::hash() const
{
  uint hash_value = returnType()->hash();

  foreach (const AbstractType::Ptr& t, d_func()->m_arguments)
    hash_value = (hash_value << 5) - hash_value + t->hash();

  return hash_value;
}

uint StructureType::hash() const
{
  uint hash_value = 101;

  foreach (const AbstractType::Ptr& t, d_func()->m_elements)
    hash_value = (hash_value << 3) - hash_value + t->hash();

  return hash_value;
}

uint ArrayType::hash() const
{
  return elementType()->hash() * 47 * dimension();
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
