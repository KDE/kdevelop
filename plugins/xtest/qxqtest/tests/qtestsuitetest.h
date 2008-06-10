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

#ifndef QXQTEST_QTESTSUITETEST_H
#define QXQTEST_QTESTSUITETEST_H

#include <QtTest/QtTest>
#include <qtestsuite.h>

namespace QxQTest
{
namespace ut {

class QTestSuiteTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void construct();
    void addChildCase();
    void addChildCases();

private:
    QFileInfo m_path;
    QString m_name;
    QxQTest::QTestSuite* m_suite;
};

}
}

#endif // QXQTEST_QTESTSUITETEST_H
