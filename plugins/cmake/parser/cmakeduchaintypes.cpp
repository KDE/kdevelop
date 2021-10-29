/*
    SPDX-FileCopyrightText: 2007-2012 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakeduchaintypes.h"
#include <language/duchain/types/typeregister.h>

namespace KDevelop {
REGISTER_TYPE(TargetType);
}

TargetType::TargetType()
    : AbstractType(createData<TargetType>())
{}

TargetType::TargetType(const TargetType& rhs)
    : AbstractType(copyData<TargetType>(*rhs.d_func()))
{}

TargetType::TargetType(KDevelop::AbstractTypeData& dd): AbstractType(dd)
{}


KDevelop::AbstractType* TargetType::clone() const
{
    return new TargetType;
}

void TargetType::accept0(KDevelop::TypeVisitor* ) const
{}

bool TargetType::equals(const KDevelop::AbstractType* rhs) const
{
    return dynamic_cast<const TargetType*>(rhs)!=nullptr;
}
