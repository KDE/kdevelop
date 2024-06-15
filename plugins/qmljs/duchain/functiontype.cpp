/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "functiontype.h"

#include <language/duchain/types/typeregister.h>
#include <language/util/kdevhash.h>

namespace QmlJS {

FunctionType::FunctionType()
: FunctionTypeBase(createData<FunctionType>())
{
}

FunctionType::FunctionType(const FunctionType& rhs)
: FunctionTypeBase(copyData<FunctionType>(*static_cast<const Data*>(rhs.d_func())))
{
}

FunctionType::FunctionType(Data& data)
: FunctionTypeBase(data)
{
}

FunctionType::~FunctionType()
{
}

KDevelop::AbstractType* FunctionType::clone() const
{
    return new FunctionType(*this);
}

size_t FunctionType::hash() const
{
    return KDevHash(KDevelop::FunctionType::hash()) << KDevelop::IdentifiedType::hash();
}

QString FunctionType::toString() const
{
    return KDevelop::FunctionType::toString();
}

}
