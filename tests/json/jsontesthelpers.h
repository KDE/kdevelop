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

#ifndef KDEVPLATFORM_JSONTESTHELPERS_H
#define KDEVPLATFORM_JSONTESTHELPERS_H

#include "testsuite.h"

#define VERIFY_NOT_NULL(ptr)\
if (!ptr) \
    return JsonTestHelpers::INVALID_POINTER

#define VERIFY_TYPE(qvariantType)\
if (!value.canConvert<qvariantType>())\
    return JsonTestHelpers::INVALID_JSON_TYPE.arg(#qvariantType, QVariant::typeToName(value.type()))

#define __AddTest(testName, objType)\
bool testName##Added = KDevelop::TestSuite<objType>::get().addTest(#testName, &testName)

#define __DefineTest(testName, objType, objName)\
QString testName(const QVariant&, objType);\
__AddTest(testName, objType);\
QString testName(const QVariant &value, objType objName)

#define DeclarationTest(testName) __DefineTest(testName, KDevelop::Declaration*, decl)
#define TypeTest(testName) __DefineTest(testName, KDevelop::AbstractType::Ptr, type)
#define ContextTest(testName) __DefineTest(testName, KDevelop::DUContext*, ctxt)

namespace KDevelop
{

namespace JsonTestHelpers
{

const QString SUCCESS = QString();
const QString INVALID_JSON_TYPE = QString("Incorrect JSON type provided for test. Actual: %1, Expected: %2");
const QString INVALID_POINTER = QString("Null pointer passed to test.");

template <class Type>
inline QString compareValues(Type realValue, const QVariant &value, const QString &errorDesc)
{
  VERIFY_TYPE(Type);
  const QString ERROR_MESSAGE = "%1 (\"%2\") doesn't match test data (\"%3\").";
  return realValue == value.value<Type>() ?
      SUCCESS : ERROR_MESSAGE.arg(errorDesc).arg(realValue).arg(value.value<Type>());
}

template <class Object>
inline QString testObject(Object obj, const QVariant &value, const QString &errorDesc)
{
  VERIFY_TYPE(QVariantMap);
  const QString ERROR_MESSAGE = "%1 did not pass tests.";
  return KDevelop::TestSuite<Object>::get().runTests(value.toMap(), obj) ? SUCCESS : ERROR_MESSAGE.arg(errorDesc);
}

inline QString rangeStr(const KDevelop::RangeInRevision &range)
{
  return QString("[(%1, %2), (%3, %4)]")
    .arg(range.start.line)
    .arg(range.start.column)
    .arg(range.end.line)
    .arg(range.end.column);
}

}

}

#endif //KDEVPLATFORM_JSONTESTHELPERS_H
