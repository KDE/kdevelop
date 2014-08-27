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

#include "typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{

REGISTER_TYPE(StructureType);

StructureType::StructureType(const StructureType& rhs)
  : StructureTypeBase(copyData<StructureType>(*rhs.d_func()))
{
}

StructureType::StructureType(StructureTypeData& data)
  : StructureTypeBase(data)
{
}

AbstractType* StructureType::clone() const {
  return new StructureType(*this);
}

bool StructureType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!StructureTypeBase::equals(_rhs))
    return false;

//   Q_ASSERT(fastCast<const StructureType*>(_rhs));

//   const StructureType* rhs = static_cast<const StructureType*>(_rhs);

  return true;
}

StructureType::StructureType()
  : StructureTypeBase(createData<StructureType>())
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
  QualifiedIdentifier id = qualifiedIdentifier();
  if (!id.isEmpty()) {
    return AbstractType::toString() + id.toString();
  }

  QString type = "class";

  return QString("<%1>").arg(type) + AbstractType::toString(true);
}

AbstractType::WhichType StructureType::whichType() const
{
  return TypeStructure;
}

uint StructureType::hash() const
{
  return KDevHash(AbstractType::hash()) << IdentifiedType::hash();
}

}
