/*!
 * \file  cppunitmodel.cpp
 *
 * \brief Implements class CppUnitModel.
 */

#include "cppunitmodel.h"
#include "cppunititem.h"

namespace QxCppUnit
{

CppUnitModel::CppUnitModel(QObject* parent)
        : RunnerModel(parent)
{
    // Data for column headers is stored in the root item.
    QList<QVariant> rootData;
    rootData << tr("Test Name") << tr("Result") << tr("Message")
    << tr("File Name") << tr("Line Number");

    setRootItem(new CppUnitItem(rootData));

    // Define the set of expected results.
    setExpectedResults(QxRunner::RunWarning | QxRunner::RunError);
}

CppUnitModel::~CppUnitModel()
{

}

QString CppUnitModel::name() const
{
    return tr("QxCppUnit");
}

QString CppUnitModel::about() const
{
    QString version(CPPUNIT_VERSION);
    QString aboutModel = tr("for CppUnit") + " " + version;
    return aboutModel;
}

void CppUnitModel::addTest(CPPUNIT_NS::Test* test) const
{
    // Recursively add the tests to the model.
    addTestItem(test, rootItem());
}

void CppUnitModel::addTestItem(CPPUNIT_NS::Test* test, RunnerItem* parent) const
{
    QString testName = QString::fromLatin1(test->getName().c_str());

    QList<QVariant> columnData;
    columnData << testName;

    CppUnitItem* item = new CppUnitItem(columnData, parent, test);
    parent->appendChild(item);

    // Could be a suite with several tests.
    for (int i = 0; i < test->getChildTestCount(); i++) {
        addTestItem(test->getChildTestAt(i), item);
    }
}

} // namespace
