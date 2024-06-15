/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "structuretype.h"

#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {
REGISTER_TYPE(StructureType);

StructureType::StructureType(const StructureType& rhs)
    : StructureTypeBase(copyData<StructureType>(*rhs.d_func()))
{
}

StructureType::StructureType(StructureTypeData& data)
    : StructureTypeBase(data)
{
}

AbstractType* StructureType::clone() const
{
    return new StructureType(*this);
}

bool StructureType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!StructureTypeBase::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const StructureType*>(_rhs));

    return true;
}

StructureType::StructureType()
    : StructureTypeBase(createData<StructureType>())
{
}

StructureType::~StructureType()
{
}

void StructureType::accept0(TypeVisitor* v) const
{
//   TYPE_D(StructureType);
    v->visit(this);

    v->endVisit(this);
}

QString StructureType::toString() const
{
    QualifiedIdentifier id = qualifiedIdentifier();
    if (!id.isEmpty()) {
        return AbstractType::toString() + id.toString();
    }

    return QLatin1String("<class>") + AbstractType::toString(true);
}

AbstractType::WhichType StructureType::whichType() const
{
    return TypeStructure;
}

size_t StructureType::hash() const
{
    return KDevHash(AbstractType::hash()) << IdentifiedType::hash();
}
}
