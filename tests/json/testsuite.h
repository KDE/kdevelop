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

namespace KDevelop
{

class DUContext;

class Declaration;

const QString EXPECT_FAIL =    "EXPECT_FAIL";
const QString FAILED_TO_FAIL = "\"%1\" FAILED TO FAIL AS EXPECTED: \"%2\" %3";
const QString EXPECTED_FAIL =  "\"%1\" FAILED (expected): %2 %3";
const QString FAIL =           "\"%1\" FAILED: %2 %3";
const QString TEST_NOT_FOUND = "Test not found";

template<class T> class TestSuite;

KDEVPLATFORMJSONTESTS_EXPORT TestSuite<KDevelop::Declaration*>& declarationTestSuite();
KDEVPLATFORMJSONTESTS_EXPORT TestSuite<KDevelop::DUContext*>& contextTestSuite();
KDEVPLATFORMJSONTESTS_EXPORT TestSuite<KDevelop::AbstractType::Ptr>& typeTestSuite();

template<class T>
class KDEVPLATFORMJSONTESTS_EXPORT TestSuite
{
public:
  typedef QString (*TestFunction)(const QVariant&, T);
  static TestSuite& get();
  bool addTest(const QString& testName, TestFunction testFunc)
  {
    qDebug() << testName << "ADD TEST" << __PRETTY_FUNCTION__ << this;
    m_testFunctions.insert(testName, testFunc);
    return true;
  }
  bool runTests(const QVariantMap &testData, T object)
  {
    QVariantMap expectedFails = expectedFailures(testData);
    QVariantMap::const_iterator it;
    DelayedOutput::Delay delay(&DelayedOutput::self());
    for (it = testData.begin(); it != testData.end(); ++it)
    {
      if (it.key() == EXPECT_FAIL)
        continue;

      qDebug() << m_testFunctions.keys() << __PRETTY_FUNCTION__ << this;
      QString result = m_testFunctions.value(it.key(), &TestSuite<T>::noSuchTest)(it.value(), object);
      QString expectedFailure = expectedFails.value(it.key(), QString()).toString();

      //Either ("expected failure" & "no result failure") or ("no expected failure" & "result failure")
      if ((bool)expectedFailure.size() ^ (bool)result.size())
      {
        DelayedOutput::self().push(result.size() ? FAIL.arg(it.key(), result, objectInformation(object)) :
                                   FAILED_TO_FAIL.arg(it.key(), expectedFailure, objectInformation(object)));
        return false;
      }

      if (expectedFailure.size())
        qDebug() << EXPECTED_FAIL.arg(it.key(), expectedFailure, objectInformation(object)).toUtf8().data();
    }
    return true;
  }
private:
  QVariantMap expectedFailures(const QVariantMap &testData)
  {
    if (!testData.contains(EXPECT_FAIL))
      return QVariantMap();

    return testData[EXPECT_FAIL].toMap();
  }
  static QString noSuchTest(const QVariant&, T)
  {
    return TEST_NOT_FOUND;
  }
  static QString objectInformation(T)
  {
    return QString();
  }
  QHash<QString, TestFunction> m_testFunctions;

  TestSuite() { qDebug() << "NEW TEST SUITE #################################" << __PRETTY_FUNCTION__ << this; }
  Q_DISABLE_COPY(TestSuite);

  friend TestSuite<Declaration*>& declarationTestSuite();
  friend TestSuite<DUContext*>& contextTestSuite();
  friend TestSuite<AbstractType::Ptr>& typeTestSuite();
};

template<class T>
inline bool runTests(const QVariantMap &data, T object)
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
