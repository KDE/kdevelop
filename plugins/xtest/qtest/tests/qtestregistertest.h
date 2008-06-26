/* KDevelop xUnit plugin
 *
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

#ifndef QXQTEST_QTESTREGISTERTEST_H
#define QXQTEST_QTESTREGISTERTEST_H

#include <QtTest/QtTest>

namespace QTest
{
class QTestRegister;
class QTestSuite;
class QTestCase;
}

namespace QTest
{
namespace ut {

class QTestRegisterTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void parseSuiteXml();
    void parseMultiSuitesXml();
    void parseCaseXml();
    void parseMultiCaseXml();
    void parseCmdXml();
    void parseMultiCmdXMl();

private: // state
    QTest::QTestRegister* reg;

private: // helpers
    void compareSuites(QTest::QTestSuite* exp,
                       QTest::QTestSuite* actual);
    void compareCase(QTest::QTestCase* expected,
                     QTest::QTestCase* actual);
    void registerTests(QByteArray& xml);
};

}
}

#endif // QXQTEST_QTESTREGISTERTEST_H
