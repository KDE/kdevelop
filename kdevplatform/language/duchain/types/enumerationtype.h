/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ENUMERATIONTYPE_H
#define KDEVPLATFORM_ENUMERATIONTYPE_H

#include "integraltype.h"
#include "identifiedtype.h"
#include "typesystemdata.h" // for IntegralTypeData (used in EnumerationTypeBase)

namespace KDevelop {
using EnumerationTypeBase = KDevelop::MergeIdentifiedType<IntegralType>;

using EnumerationTypeData = EnumerationTypeBase::Data;

class KDEVPLATFORMLANGUAGE_EXPORT EnumerationType
    : public EnumerationTypeBase
{
public:
    EnumerationType();

    EnumerationType(const EnumerationType& rhs);

    explicit EnumerationType(EnumerationTypeData& data);

    EnumerationType& operator=(const EnumerationType& rhs) = delete;

    using Ptr = TypePtr<EnumerationType>;

    size_t hash() const override;

    KDevelop::AbstractType* clone() const override;

    bool equals(const KDevelop::AbstractType* rhs) const override;

    QString toString() const override;

    WhichType whichType() const override;

    enum {
        Identity = 21
    };

    using Data = EnumerationTypeData;

protected:
    TYPE_DECLARE_DATA(EnumerationType);
};
}

#endif // KDEVPLATFORM_ENUMERATIONTYPE_H
