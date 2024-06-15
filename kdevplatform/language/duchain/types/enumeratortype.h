/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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

    size_t hash() const override;

    WhichType whichType() const override;

    QString toString() const override;

    enum {
        Identity = 20
    };

    using Data = EnumeratorTypeData;

protected:
    TYPE_DECLARE_DATA(EnumeratorType);
};
}

#endif // KDEVPLATFORM_ENUMERATORTYPE_H
