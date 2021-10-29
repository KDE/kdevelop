/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_JSONTYPETESTS_H
#define KDEVPLATFORM_JSONTYPETESTS_H

#include "language/duchain/types/abstracttype.h"
#include "language/duchain/types/constantintegraltype.h"
#include "language/duchain/types/delayedtype.h"
#include "language/duchain/types/functiontype.h"
#include "jsontesthelpers.h"

/**
 * Quick Reference:
 *   toString : string
 *   isConst : bool
 *   plainValue : qint64
 */

namespace KDevelop {
namespace TypeTests {
using namespace JsonTestHelpers;

///JSON type: string
///@returns whether the type toString matches the given value
TypeTest(toString)
{
    QString typeStr = type ? type->toString() : QStringLiteral("<no type>");
    return compareValues(typeStr, value, QStringLiteral("Type's toString"));
}
///JSON type: bool
///@returns whether type's constness matches the given value
TypeTest(isConst)
{
    VERIFY_TYPE(bool);
    bool typeIsConst = false;
    if (DelayedType::Ptr delayed = type.cast<DelayedType>())
        typeIsConst = delayed->identifier().isConstant();
    else
        typeIsConst = (type->modifiers() & AbstractType::ConstModifier);

    if (typeIsConst != value.toBool())
        return typeIsConst ? QStringLiteral("Type is constant, but test data expects non-const.") :
               QStringLiteral("Type is non-const, but test data expects constant.");

    return SUCCESS();
}

///JSON type: qint64
///@returns Whether ConstantIntegralType's plainValue matches the given value
TypeTest(plainValue)
{
    VERIFY_TYPE(qint64);
    auto constantIntegralType = type.cast<ConstantIntegralType>();
    VERIFY_NOT_NULL(constantIntegralType);
    return compareValues(constantIntegralType->plainValue(), value,
                         QStringLiteral("ConstantIntegralType's plainValue"));
}
}
}

#endif //KDEVPLATFORM_JSONTYPETESTS_H
