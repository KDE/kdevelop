/*!
 * \file  cppunitmodel.h
 *
 * \brief Declares class CppUnitModel.
 */

#ifndef CPPUNITMODEL_H
#define CPPUNITMODEL_H

#include <qxrunner/runnermodel.h>
#include <cppunit/Test.h>

using namespace QxRunner;

namespace QxCppUnit
{

/*!
 * \brief The CppUnitModel class maintains CppUnit data and executes
 *        CppUnit unit tests.
 *
 * This class stores CppUnit test data using CppUnitItem objects. The
 * CppUnit tests referenced by the CppUnitItem objects can be executed.
 * Test results are prepared for representation in views.
 *
 * The expected results from this model are:
 *
 *
 * - QxRunner::RunSuccess - CppUnit test case passed
 * - QxRunner::RunWarning - CppUnit assertion failed
 * - QxRunner::RunError   - CppUnit reported an error
 */

class CppUnitModel : public RunnerModel
{
    Q_OBJECT

public: // Operations

    /*!
     * Constructs a CppUnit model with the given \a parent. The root
     * item is created and the set of expected results is defined.
     */
    CppUnitModel(QObject* parent = 0);

    /*!
     * Destroys this CppUnit model.
     */
    ~CppUnitModel();

    /*!
     * Returns the model name "QxCppUnit".
     */
    QString name() const;

    /*!
     * Returns the CppUnit version used.
     */
    QString about() const;

    /*!
     * Appends the \a test to the list of CppUnit tests in the model.
     * \a test can be one individual test or a test suite.
     */
    void addTest(CPPUNIT_NS::Test* test) const;

private: // Operations

    /*!
     * Helper method to append a CppUnitItem object for the \a test
     * to the given \a parent. If \a test is a suite then recursively
     * a CppUnitItem object for each test in the suite is appended.
     */
    void addTestItem(CPPUNIT_NS::Test* test, RunnerItem* parent) const;
};

} // namespace

#endif // CPPUNITMODEL_H
