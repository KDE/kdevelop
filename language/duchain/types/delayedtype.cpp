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

#include "delayedtype.h"

#include "../indexedstring.h"
#include "../repositories/typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{
REGISTER_TYPE(DelayedType);

DelayedType::DelayedType(DelayedTypeData& data) : AbstractType(data) {
}

AbstractType* DelayedType::clone() const {
  return new DelayedType(*this);
}

bool DelayedType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!AbstractType::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const DelayedType*>(_rhs));

  const DelayedType* rhs = static_cast<const DelayedType*>(_rhs);

  return d_func()->m_identifier == rhs->d_func()->m_identifier && rhs->d_func()->m_kind == d_func()->m_kind;
}

AbstractType::WhichType DelayedType::whichType() const
{
  return AbstractType::TypeDelayed;
}

QString DelayedType::toString() const
{
  return AbstractType::toString(false) + identifier().toString();
}

DelayedType::Kind DelayedType::kind() const {
  return d_func()->m_kind;
}

void DelayedType::setKind(Kind kind) {
  d_func_dynamic()->m_kind = kind;
}

DelayedType::DelayedType()
  : AbstractType(createData<DelayedType>())
{
}

DelayedType::DelayedType(const DelayedType& rhs) : AbstractType(copyData<DelayedType>(*rhs.d_func())) {
}

DelayedType::~DelayedType()
{
}

void DelayedType::setIdentifier(const IndexedTypeIdentifier& identifier)
{
  d_func_dynamic()->m_identifier = identifier;
}

IndexedTypeIdentifier DelayedType::identifier() const
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
  return KDevHash(AbstractType::hash()) << d_func()->m_identifier.hash() << d_func()->m_kind;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
