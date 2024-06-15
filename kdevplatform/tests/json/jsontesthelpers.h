/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_JSONTESTHELPERS_H
#define KDEVPLATFORM_JSONTESTHELPERS_H

#include "testsuite.h"

#define VERIFY_NOT_NULL(ptr) \
    if (!ptr) \
        return JsonTestHelpers::INVALID_POINTER()

#define VERIFY_TYPE(qvariantType)                                                                                      \
    if (!value.canConvert<qvariantType>())                                                                             \
    return JsonTestHelpers::INVALID_JSON_TYPE().arg(QStringLiteral(#qvariantType),                                     \
                                                    QString::fromUtf8(value.metaType().name()))

#define __AddTest(testName, objType) \
    bool testName ## Added = KDevelop::TestSuite<objType>::get().addTest(QStringLiteral(# testName), &testName)

#define __DefineTest(testName, objType, objName) \
    QString testName(const QVariant&, objType); \
    __AddTest(testName, objType); \
    QString testName(const QVariant &value, objType objName)

#define DeclarationTest(testName) __DefineTest(testName, KDevelop::Declaration*, decl)
#define TypeTest(testName) __DefineTest(testName, KDevelop::AbstractType::Ptr, type)
#define ContextTest(testName) __DefineTest(testName, KDevelop::DUContext*, ctxt)

namespace KDevelop {
namespace JsonTestHelpers {
inline QString SUCCESS() { return QString(); }
inline QString INVALID_JSON_TYPE()
{
    return QStringLiteral("Incorrect JSON type provided for test. Actual: %1, Expected: %2");
}
inline QString INVALID_POINTER() { return QStringLiteral("Null pointer passed to test."); }

template<class Type>
inline QString compareValues(Type realValue, const QVariant& value, const QString& errorDesc)
{
    VERIFY_TYPE(Type);
    const QString ERROR_MESSAGE = QStringLiteral("%1 (\"%2\") doesn't match test data (\"%3\").");
    return realValue == value.value<Type>() ?
           SUCCESS() : ERROR_MESSAGE.arg(errorDesc).arg(realValue).arg(value.value<Type>());
}

template<class Object>
inline QString testObject(Object obj, const QVariant& value, const QString& errorDesc)
{
    VERIFY_TYPE(QVariantMap);
    const QString ERROR_MESSAGE = QStringLiteral("%1 did not pass tests.");
    return KDevelop::TestSuite<Object>::get().runTests(value.toMap(), obj) ? SUCCESS() : ERROR_MESSAGE.arg(errorDesc);
}

inline QString rangeStr(const KDevelop::RangeInRevision& range)
{
    return QStringLiteral("[(%1, %2), (%3, %4)]")
           .arg(range.start.line)
           .arg(range.start.column)
           .arg(range.end.line)
           .arg(range.end.column);
}
}
}

#endif //KDEVPLATFORM_JSONTESTHELPERS_H
