/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
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

#include "abstracttype.h"

#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"
#include "typerepository.h"
#include "util/debug.h"

namespace KDevelop
{

//REGISTER_TYPE(AbstractType);

void AbstractType::makeDynamic() {
  if(d_ptr->m_dynamic)
    return;
  AbstractType::Ptr newType(clone()); //While cloning, all the data is cloned as well. So we use that mechanism and steal the cloned data.
  Q_ASSERT(newType->equals(this));
  AbstractTypeData* oldData = d_ptr;
  d_ptr = newType->d_ptr;
  newType->d_ptr = oldData;
  Q_ASSERT(d_ptr->m_dynamic);
}

AbstractType::AbstractType( AbstractTypeData& dd )
  : d_ptr(&dd)
{
}

quint64 AbstractType::modifiers() const
{
  return d_func()->m_modifiers;
}

void AbstractType::setModifiers(quint64 modifiers)
{
  d_func_dynamic()->m_modifiers = modifiers;
}

AbstractType::AbstractType()
  : d_ptr(&createData<AbstractType>())
{
}

AbstractType::~AbstractType()
{
  if(!d_ptr->inRepository) {
    TypeSystem::self().callDestructor(d_ptr);
    delete[] (char*)d_ptr;
  }
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

void AbstractType::exchangeTypes( TypeExchanger* /*exchanger */) {
}

IndexedType AbstractType::indexed() const
{
  return IndexedType(TypeRepository::indexForType(AbstractType::Ptr(const_cast<AbstractType*>(this))));
}

bool AbstractType::equals(const AbstractType* rhs) const
{
    //qCDebug(LANGUAGE) << this << rhs << modifiers() << rhs->modifiers();
    return d_func()->typeClassId == rhs->d_func()->typeClassId && modifiers() == rhs->modifiers();
}

uint AbstractType::hash() const
{
  return KDevHash() << d_func()->typeClassId << d_func()->m_modifiers;
}

QString AbstractType::toString() const
{
  return toString(false);
}

QString AbstractType::toString(bool spaceOnLeft) const
{
  // TODO complete
  if(!spaceOnLeft) {
    if(modifiers() & ConstModifier) {
      if(modifiers() & VolatileModifier) {
        return "const volatile ";
      }else{
        return "const ";
      }
    }else{
      if(modifiers() & VolatileModifier)
        return "volatile ";
      else
        return QString();
    }
  }else{
    if(modifiers() & ConstModifier) {
      if(modifiers() & VolatileModifier) {
        return " const volatile";
      }else{
        return " const";
      }
    }else{
      if(modifiers() & VolatileModifier)
        return " volatile";
      else
        return QString();
    }
  }
}

}
