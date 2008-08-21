/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#ifndef QTEST_CASERESOLVERTEST_H_INCLUDED
#define QTEST_CASERESOLVERTEST_H_INCLUDED

#include <QtCore/QObject>

namespace Veritas { class Test; }

namespace QTest {
class QTestSuite;
class QTestCase;
class QTestCommand;

namespace Test {
class CustomSuiteBuilder;
class ExecutableStub;

/*! @unitundertest QTest::SuiteBuilder */
class SuiteBuilderTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void construct();
    void singleSuiteCaseCommand();
    void multiSuitesCasesCommands();
    void identicalSuiteNames();
    void removeDirPrefix();
    void keepSecondaryPrefixes();

private:
    QTestSuite* fetchSuite(Veritas::Test* root, int nrofSuite);
    void verifySuite(QTestSuite* suite, const QString name, int childCount);
    void verifyCommand(int nrofChild, QTestCase* parent, const QString name);
    void verifyCaze(QTestCase* caze, const QString name, int childCount);

private:
    CustomSuiteBuilder* m_builder;
    ExecutableStub* m_exe;
};

}}

#endif // QTEST_CASERESOLVERTEST_H_INCLUDED
