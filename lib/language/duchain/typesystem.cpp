/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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


ReferenceType::ReferenceType()
  : d(new ReferenceTypePrivate)
{
  d->m_baseType = 0;
}

ReferenceType::~ReferenceType()
{
  delete d;
}

const AbstractType::Ptr ReferenceType::baseType () const
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

QString ReferenceType::toString() const
{
  return baseType() ? QString("%1&").arg(baseType()->toString()) : QString("<notype>&");
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

const AbstractType::Ptr FunctionType::returnType () const
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

QString FunctionType::toString() const
{
  QString args;
  bool first = true;
  foreach (const AbstractType::Ptr& type, d->m_arguments) {
    if (first)
      first = false;
    else
      args.append(", ");
    args.append(type ? type->toString() : QString("<notype>"));
  }

  return QString("%1 (%2)").arg(returnType() ? returnType()->toString() : QString("<notype>")).arg(args);
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

void StructureType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    {
      for (int i = 0; i < d->m_elements.count (); ++i)
        acceptType (d->m_elements.at (i), v);
    }

  v->endVisit (this);
}

QString StructureType::toString() const
{
  return "<structure>";
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

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
