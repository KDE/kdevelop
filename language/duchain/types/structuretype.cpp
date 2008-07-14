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

#include "structuretype.h"

#include "indexedstring.h"
#include "repositories/typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{

REGISTER_TYPE(StructureType);

StructureType::StructureType(const StructureType& rhs) : AbstractType(copyData<StructureTypeData>(*rhs.d_func())) {
}

StructureType::StructureType(StructureTypeData& data) : AbstractType(data) {
}

AbstractType* StructureType::clone() const {
  return new StructureType(*this);
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

StructureType::StructureType()
  : AbstractType(createData<StructureTypeData>())
{
  d_func_dynamic()->setTypeClassId<StructureType>();
}

StructureType::~StructureType()
{
}

void StructureType::setClassType(uint type)
{
}

uint StructureType::classType() const
{
}

bool StructureType::isClosed() const
{
  return d_func()->m_closed;
}

void StructureType::close()
{
  d_func_dynamic()->m_closed = true;
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

uint StructureType::hash() const
{
  uint hash_value = 101;

  return hash_value;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
