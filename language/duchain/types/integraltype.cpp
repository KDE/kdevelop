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

#include "integraltype.h"

#include "indexedstring.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{

REGISTER_TYPE(IntegralType);

IntegralType::IntegralType(const IntegralType& rhs) : AbstractType(copyData<IntegralTypeData>(*rhs.d_func())) {
}

IntegralType::IntegralType(IntegralTypeData& data) : AbstractType(data) {
}

uint IntegralType::dataType() const
{
  return d_func()->m_dataType;
}

void IntegralType::setDataType(uint dataType)
{
  d_func_dynamic()->m_dataType = dataType;
}

AbstractType* IntegralType::clone() const {
  return new IntegralType(*this);
}

bool IntegralType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const IntegralType*>(_rhs))
    return false;
  const IntegralType* rhs = static_cast<const IntegralType*>(_rhs);

  return rhs->d_func()->m_name == d_func()->m_name;
}

IntegralType::IntegralType(const IndexedString& name)
  : AbstractType(*new IntegralTypeData)
{
  d_func_dynamic()->setTypeClassId<IntegralType>();

  d_func_dynamic()->m_name = name;
}

IntegralType::IntegralType()
  : AbstractType(createData<IntegralTypeData>())
{
  d_func_dynamic()->setTypeClassId<IntegralType>();
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

uint IntegralType::hash() const
{
  return qHash(d_func()->m_name) * 17;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
