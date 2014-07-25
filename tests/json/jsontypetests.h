/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

#ifndef KDEVPLATFORM_JSONTYPETESTS_H
#define KDEVPLATFORM_JSONTYPETESTS_H

#include "language/duchain/types/abstracttype.h"
#include "language/duchain/types/constantintegraltype.h"
#include "language/duchain/types/delayedtype.h"
#include "language/duchain/types/functiontype.h"
#include "jsontesthelpers.h"

/**
 * Quick Reference:
 *   findDeclarations : FindDeclObject
 *   childCount : int
 */

namespace KDevelop
{

namespace TypeTests
{

using namespace JsonTestHelpers;

///JSON type: string
///@returns whether the type toString matches the given value
TypeTest(toString)
{
  QString typeStr = type ? type->toString() : "<no type>";
  return compareValues(typeStr, value, "Type's toString");
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
    return typeIsConst ? "Type is constant, but test data expects non-const." :
                         "Type is non-const, but test data expects constant.";

  return SUCCESS;
}

TypeTest(plainValue)
{
    VERIFY_TYPE(qint64);
    auto constantIntegralType = type.cast<ConstantIntegralType>();
    VERIFY_NOT_NULL(constantIntegralType);
    return compareValues(constantIntegralType->plainValue(), value, "ConstantIntegralType's plainValue");
}

}

}

#endif //KDEVPLATFORM_JSONTYPETESTS_H
