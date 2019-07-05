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

#ifndef KDEVPLATFORM_TESTSUITE_H
#define KDEVPLATFORM_TESTSUITE_H

#include <QVariantMap>
#include "delayedoutput.h"
#include <language/duchain/types/abstracttype.h>
#include <QDebug>

namespace KDevelop {
class DUContext;

class Declaration;

inline QString EXPECT_FAIL()    { return QStringLiteral("EXPECT_FAIL"); }
inline QString FAILED_TO_FAIL() { return QStringLiteral("\"%1\" FAILED TO FAIL AS EXPECTED: \"%2\" %3"); }
inline QString EXPECTED_FAIL()  { return QStringLiteral("\"%1\" FAILED (expected): %2 %3"); }
inline QString FAIL()           { return QStringLiteral("\"%1\" FAILED: %2 %3"); }
inline QString TEST_NOT_FOUND() { return QStringLiteral("Test not found"); }

template<class T> class TestSuite;

KDEVPLATFORMTESTS_EXPORT TestSuite<KDevelop::Declaration*>& declarationTestSuite();
KDEVPLATFORMTESTS_EXPORT TestSuite<KDevelop::DUContext*>& contextTestSuite();
KDEVPLATFORMTESTS_EXPORT TestSuite<KDevelop::AbstractType::Ptr>& typeTestSuite();

template<class T>
class KDEVPLATFORMTESTS_EXPORT TestSuite
{
public:
    using TestFunction = QString (*)(const QVariant&, T);
    static TestSuite& get();
    bool addTest(const QString& testName, TestFunction testFunc)
    {
        m_testFunctions.insert(testName, testFunc);
        return true;
    }
    bool runTests(const QVariantMap& testData, T object)
    {
        QVariantMap expectedFails = expectedFailures(testData);
        QVariantMap::const_iterator it;
        DelayedOutput::Delay delay(&DelayedOutput::self());
        for (it = testData.begin(); it != testData.end(); ++it) {
            if (it.key() == EXPECT_FAIL())
                continue;

            QString result = m_testFunctions.value(it.key(), &TestSuite<T>::noSuchTest)(it.value(), object);
            QString expectedFailure = expectedFails.value(it.key(), QString()).toString();

            //Either ("expected failure" & "no result failure") or ("no expected failure" & "result failure")
            if (expectedFailure.isEmpty() ^ result.isEmpty()) {
                DelayedOutput::self().push(result.isEmpty() ? FAILED_TO_FAIL().arg(it.key(), expectedFailure,
                                                                                   objectInformation(object)) :
                                           FAIL().arg(it.key(), result, objectInformation(object)));
                return false;
            }

            if (!expectedFailure.isEmpty())
                qDebug() << EXPECTED_FAIL().arg(it.key(), expectedFailure, objectInformation(object)).toUtf8().data();
        }

        return true;
    }

private:
    QVariantMap expectedFailures(const QVariantMap& testData)
    {
        if (!testData.contains(EXPECT_FAIL()))
            return QVariantMap();

        return testData[EXPECT_FAIL()].toMap();
    }
    static QString noSuchTest(const QVariant&, T)
    {
        return TEST_NOT_FOUND();
    }
    static QString objectInformation(T)
    {
        return QString();
    }
    QHash<QString, TestFunction> m_testFunctions;

    TestSuite() { }
    Q_DISABLE_COPY(TestSuite)

    friend KDEVPLATFORMTESTS_EXPORT TestSuite<Declaration*>& declarationTestSuite();
    friend KDEVPLATFORMTESTS_EXPORT TestSuite<DUContext*>& contextTestSuite();
    friend KDEVPLATFORMTESTS_EXPORT TestSuite<AbstractType::Ptr>& typeTestSuite();
};

template<class T>
inline bool runTests(const QVariantMap& data, T object)
{
    return TestSuite<T>::get().runTests(data, object);
}

///TODO: Once we can use C++11, see whether this can be cleaned up by extern templates
template<>
inline TestSuite<Declaration*>& TestSuite<Declaration*>::get()
{
    return declarationTestSuite();
}

template<>
inline TestSuite<DUContext*>& TestSuite<DUContext*>::get()
{
    return contextTestSuite();
}

template<>
inline TestSuite<AbstractType::Ptr>& TestSuite<AbstractType::Ptr>::get()
{
    return typeTestSuite();
}
}

#endif //KDEVPLATFORM_TESTSUITE_H
