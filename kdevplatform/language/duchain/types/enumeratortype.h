/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
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

#ifndef KDEVPLATFORM_ENUMERATORTYPE_H
#define KDEVPLATFORM_ENUMERATORTYPE_H

#include "identifiedtype.h"
#include "constantintegraltype.h"

namespace KDevelop {
using EnumeratorTypeBase = KDevelop::MergeIdentifiedType<ConstantIntegralType>;

using EnumeratorTypeData = EnumeratorTypeBase::Data;

//The same as EnumerationType, with the difference that here the value is also known
class KDEVPLATFORMLANGUAGE_EXPORT EnumeratorType
    : public EnumeratorTypeBase
{
public:
    EnumeratorType(const EnumeratorType& rhs);

    explicit EnumeratorType(EnumeratorTypeData& data);

    EnumeratorType();

    EnumeratorType& operator=(const EnumeratorType& rhs) = delete;

    using Ptr = TypePtr<EnumeratorType>;

    bool equals(const KDevelop::AbstractType* rhs) const override;

    KDevelop::AbstractType* clone() const override;

    uint hash() const override;

    WhichType whichType() const override;

    QString toString() const override;

    enum {
        Identity = 20
    };

    using Data = EnumeratorTypeData;

protected:
    TYPE_DECLARE_DATA(EnumeratorType);
};

template <>
inline EnumeratorType* fastCast<EnumeratorType*>(AbstractType* from)
{
    if (!from || from->whichType() != KDevelop::AbstractType::TypeEnumerator)
        return nullptr;
    else
        return static_cast<EnumeratorType*>(from);
}
}

#endif // KDEVPLATFORM_ENUMERATORTYPE_H
