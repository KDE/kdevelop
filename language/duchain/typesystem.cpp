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

class AbstractTypePrivate
{
public:
  bool m_registered;
};

class IntegralTypePrivate
{
public:
  QString m_name;
};

class PointerTypePrivate
{
public:
  AbstractType::Ptr m_baseType;
};

class ReferenceTypePrivate
{
public:
  AbstractType::Ptr m_baseType;
};

class FunctionTypePrivate
{
public:
  AbstractType::Ptr m_returnType;
  QList<AbstractType::Ptr> m_arguments;
};

class StructureTypePrivate
{
public:
  QList<AbstractType::Ptr> m_elements;
};

class ArrayTypePrivate
{
public:
  int m_dimension;
  AbstractType::Ptr m_elementType;
};

class DelayedTypePrivate
{
public:
  DelayedTypePrivate() : m_kind(DelayedType::Delayed) {
  }
  
  QualifiedIdentifier m_identifier;
  DelayedType::Kind m_kind;
};

AbstractType::AbstractType(const AbstractType& rhs) : KShared(), d(new AbstractTypePrivate(*rhs.d)) {
}

IntegralType::IntegralType(const IntegralType& rhs) : AbstractType(rhs), d(new IntegralTypePrivate(*rhs.d)) {
}

PointerType::PointerType(const PointerType& rhs) : AbstractType(rhs), d(new PointerTypePrivate(*rhs.d)) {
}

ReferenceType::ReferenceType(const ReferenceType& rhs) : AbstractType(rhs), d(new ReferenceTypePrivate(*rhs.d)) {
}

FunctionType::FunctionType(const FunctionType& rhs) : AbstractType(rhs), d(new FunctionTypePrivate(*rhs.d)) {
}

StructureType::StructureType(const StructureType& rhs) : AbstractType(rhs), d(new StructureTypePrivate(*rhs.d)) {
}

ArrayType::ArrayType(const ArrayType& rhs) : AbstractType(rhs), d(new ArrayTypePrivate(*rhs.d)) {
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
  
  return rhs->d->m_name == d->m_name;
}

bool PointerType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const PointerType*>(_rhs))
    return false;
  const PointerType* rhs = static_cast<const PointerType*>(_rhs);

  if( (bool)rhs->d->m_baseType != (bool)d->m_baseType )
    return false;
  
  if( !d->m_baseType )
    return true;
  
  return rhs->d->m_baseType->equals(d->m_baseType.data());
}

bool ReferenceType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const ReferenceType*>(_rhs))
    return false;
  const ReferenceType* rhs = static_cast<const ReferenceType*>(_rhs);

  if( (bool)rhs->d->m_baseType != (bool)d->m_baseType )
    return false;
  
  if( !d->m_baseType )
    return true;
  
  return rhs->d->m_baseType->equals(d->m_baseType.data());
}

bool FunctionType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const FunctionType*>(_rhs))
    return false;
  const FunctionType* rhs = static_cast<const FunctionType*>(_rhs);

  if( d->m_arguments.count() != rhs->d->m_arguments.count() )
    return false;
  
  if( (bool)rhs->d->m_returnType != (bool)d->m_returnType )
    return false;
  
  if( d->m_returnType )
    if( !rhs->d->m_returnType->equals(d->m_returnType.data()) )
      return false;
  
  QList<AbstractType::Ptr>::const_iterator it1 = d->m_arguments.begin();
  QList<AbstractType::Ptr>::const_iterator it2 = rhs->d->m_arguments.begin();
  
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

  if( d->m_elements.count() != rhs->d->m_elements.count() )
    return false;
  
  QList<AbstractType::Ptr>::const_iterator it1 = d->m_elements.begin();
  QList<AbstractType::Ptr>::const_iterator it2 = rhs->d->m_elements.begin();
  
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

  if( d->m_dimension != rhs->d->m_dimension )
    return false;
  
  if( (bool)rhs->d->m_elementType != (bool)d->m_elementType )
    return false;
  
  if( !d->m_elementType )
    return true;
  
  return rhs->d->m_elementType->equals(d->m_elementType.data());
}

bool DelayedType::equals(const AbstractType* _rhs) const
{
  if( !dynamic_cast<const DelayedType*>(_rhs))
    return false;

  const DelayedType* rhs = static_cast<const DelayedType*>(_rhs);

  return d->m_identifier == rhs->d->m_identifier;
}

AbstractType::AbstractType()
  : d(new AbstractTypePrivate)
{
  d->m_registered = false;
}

AbstractType::~AbstractType()
{
//  Q_ASSERT(!m_registered);
  delete d;
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
  : d(new IntegralTypePrivate)
{
  d->m_name = name;
}

IntegralType::IntegralType()
  : d(new IntegralTypePrivate)
{
}

IntegralType::~IntegralType()
{
  delete d;
}

const QString& IntegralType::name() const
{
  return d->m_name;
}

void IntegralType::setName(const QString& name)
{
  d->m_name = name;
}

bool IntegralType::operator == (const IntegralType &other) const
{
  return d->m_name == other.d->m_name;
}

bool IntegralType::operator != (const IntegralType &other) const
{
  return d->m_name != other.d->m_name;
}

QString IntegralType::toString() const
{
  return d->m_name;
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
  : d(new PointerTypePrivate)
{
  d->m_baseType = 0;
}

void PointerType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d->m_baseType, v);

  v->endVisit (this);
}

void PointerType::exchangeTypes( TypeExchanger* exchanger ) {
  d->m_baseType = exchanger->exchange( d->m_baseType.data() );
}

PointerType::~PointerType()
{
  delete d;
}

AbstractType::Ptr PointerType::baseType () const
{
  return d->m_baseType;
}

void PointerType::setBaseType(AbstractType::Ptr type)
{
  d->m_baseType = type;
}

bool PointerType::operator == (const PointerType &other) const
{
  return d->m_baseType == other.d->m_baseType;
}

bool PointerType::operator != (const PointerType &other) const
{
  return d->m_baseType != other.d->m_baseType;
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
  : d(new ReferenceTypePrivate)
{
  d->m_baseType = 0;
}

ReferenceType::~ReferenceType()
{
  delete d;
}

AbstractType::Ptr ReferenceType::baseType () const
{
  return d->m_baseType;
}

void ReferenceType::setBaseType(AbstractType::Ptr type)
{
  d->m_baseType = type;
}

bool ReferenceType::operator == (const ReferenceType &other) const
{
  return d->m_baseType == other.d->m_baseType;
}

bool ReferenceType::operator != (const ReferenceType &other) const
{
  return d->m_baseType != other.d->m_baseType;
}

void ReferenceType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d->m_baseType, v);

  v->endVisit (this);
}

void ReferenceType::exchangeTypes( TypeExchanger* exchanger )
{
  d->m_baseType = exchanger->exchange( d->m_baseType.data() );
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
  : d(new FunctionTypePrivate)
{
}

FunctionType::~FunctionType()
{
  delete d;
}


void FunctionType::addArgument(AbstractType::Ptr argument)
{
  d->m_arguments.append(argument);
}

void FunctionType::removeArgument(AbstractType::Ptr argument)
{
  d->m_arguments.removeAll(argument);
}

void FunctionType::setReturnType(AbstractType::Ptr returnType)
{
  d->m_returnType = returnType;
}

AbstractType::Ptr FunctionType::returnType () const
{
  return d->m_returnType;
}

const QList<AbstractType::Ptr>& FunctionType::arguments () const
{
  return d->m_arguments;
}

bool FunctionType::operator == (const FunctionType &other) const
{
  return d->m_returnType == other.d->m_returnType && d->m_arguments == other.d->m_arguments;
}

bool FunctionType::operator != (const FunctionType &other) const
{
  return d->m_returnType != other.d->m_returnType || d->m_arguments != other.d->m_arguments;
}

void AbstractType::exchangeTypes( TypeExchanger* /*exchanger */) {
}

void FunctionType::accept0 (TypeVisitor *v) const
{
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
  for (int i = 0; i < d->m_arguments.count (); ++i)
    d->m_arguments[i] = exchanger->exchange( d->m_arguments[i].data() );
  d->m_returnType = exchanger->exchange(d->m_returnType.data());
}

QString FunctionType::toString( SignaturePart sigPart ) const {
  QString args;
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
  return toString(SignatureWhole);
}

AbstractType::WhichType FunctionType::whichType() const
{
  return TypeFunction;
}

StructureType::StructureType()
  : d(new StructureTypePrivate)
{
}

StructureType::~StructureType()
{
  delete d;
}

const QList<AbstractType::Ptr>& StructureType::elements () const
{
  return d->m_elements;
}

bool StructureType::operator == (const StructureType &other) const
{
  return d->m_elements == other.d->m_elements;
}

bool StructureType::operator != (const StructureType &other) const
{
  return d->m_elements != other.d->m_elements;
}

void StructureType::addElement(AbstractType::Ptr element)
{
  d->m_elements.append(element);
}

void StructureType::removeElement(AbstractType::Ptr element)
{
  d->m_elements.removeAll(element);
}

void StructureType::clear() {
  d->m_elements.clear();
}

void StructureType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    {
      for (int i = 0; i < d->m_elements.count (); ++i)
        acceptType (d->m_elements.at (i), v);
    }

  v->endVisit (this);
}

void StructureType::exchangeTypes( TypeExchanger* exchanger )
{
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
  : d(new ArrayTypePrivate)
{
}

ArrayType::~ArrayType()
{
  delete d;
}

int ArrayType::dimension () const
{
  return d->m_dimension;
}

void ArrayType::setDimension(int dimension)
{
  d->m_dimension = dimension;
}

AbstractType::Ptr ArrayType::elementType () const
{
  return d->m_elementType;
}

void ArrayType::setElementType(AbstractType::Ptr type)
{
  d->m_elementType = type;
}

bool ArrayType::operator == (const ArrayType &other) const
{
  return d->m_elementType == other.d->m_elementType && d->m_dimension == other.d->m_dimension;
}

bool ArrayType::operator != (const ArrayType &other) const
{
  return d->m_elementType != other.d->m_elementType || d->m_dimension != other.d->m_dimension;
}

QString ArrayType::toString() const
{
  return QString("%1[%2]").arg(elementType() ? elementType()->toString() : QString("<notype>")).arg(d->m_dimension);
}

void ArrayType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    {
      acceptType (d->m_elementType, v);
    }

  v->endVisit (this);
}

void ArrayType::exchangeTypes( TypeExchanger* exchanger )
{
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
  return (d->m_kind == Delayed ? "<delayed> " : "<unresolved> ") + qualifiedIdentifier().toString();
}

DelayedType::Kind DelayedType::kind() const {
  return d->m_kind;
}

void DelayedType::setKind(Kind kind) {
  d->m_kind = kind;
}

DelayedType::DelayedType()
  : d(new DelayedTypePrivate)
{
}

DelayedType::~DelayedType()
{
  delete d;
}

void DelayedType::setQualifiedIdentifier(const QualifiedIdentifier& identifier)
{
  d->m_identifier = identifier;
}

QualifiedIdentifier DelayedType::qualifiedIdentifier() const
{
  return d->m_identifier;
}

void DelayedType::accept0 (KDevelop::TypeVisitor *v) const
{
    v->visit(this);
/*    v->endVisit(this);*/
}


/*uint PointerType::hash() const
{
  return baseType()->hash() * 13;
}

uint ReferenceType::hash() const
{
  return baseType()->hash() * 29;
}

uint FunctionType::hash() const
{
  uint hash_value = returnType()->hash();

  foreach (const AbstractType::Ptr& t, m_arguments)
    hash_value = (hash_value << 5) - hash_value + t->hash();

  return hash_value;
}

uint StructureType::hash() const
{
  uint hash_value = 101;

  foreach (const AbstractType::Ptr& t, m_elements)
    hash_value = (hash_value << 3) - hash_value + t->hash();

  return hash_value;
}

uint ArrayType::hash() const
{
  return elementType()->hash() * 47 * dimension();
}*/

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
