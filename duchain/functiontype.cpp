/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "functiontype.h"

#include <language/duchain/types/typeregister.h>
#include <language/util/kdevhash.h>

namespace QmlJS {

KDevelop::TypeSystemRegistrator<FunctionType, FunctionType::Data> registerFunctionType;

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

uint FunctionType::hash() const
{
    return KDevHash(KDevelop::FunctionType::hash()) << KDevelop::IdentifiedType::hash();
}

QString FunctionType::toString() const
{
    return KDevelop::FunctionType::toString();
}

}