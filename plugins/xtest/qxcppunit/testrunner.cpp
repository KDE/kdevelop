/*!
 * \file  testrunner.cpp
 *
 * \brief Implements class TestRunner.
 */

#include "testrunner.h"
#include "cppunitmodel.h"

#include <qxrunner/runner.h>
#include <cppunit/TestSuite.h>
#include <QIcon>

// Helper function needed to expand Q_INIT_RESOURCE outside the namespace.
static void initQxCppUnitResource()
{
	Q_INIT_RESOURCE(qxcppunit);
}

using namespace QxRunner;

namespace QxCppUnit {

TestRunner::TestRunner()
{
	m_runner = 0;
	m_model = new CppUnitModel;
}

TestRunner::~TestRunner()
{
	// Delete the runner first.
	delete m_runner;
	delete m_model;
}

void TestRunner::addTest(CPPUNIT_NS::Test* test) const
{
	m_model->addTest(test);
}

void TestRunner::addTests(const CppUnitVector<CPPUNIT_NS::Test*>& tests) const
{
	CppUnitVector<CPPUNIT_NS::Test*>::const_iterator it = tests.begin();

	for (; it != tests.end(); ++it)
	{
		addTest(*it);
	}
}

void TestRunner::run()
{
	m_runner = new QxRunner::Runner(m_model);

	// Application icon.
	initQxCppUnitResource();
	m_runner->setWindowIcon(QIcon(":/icons/qxcppunit_16x16.png"));

	m_runner->run();
}

} // namespace
