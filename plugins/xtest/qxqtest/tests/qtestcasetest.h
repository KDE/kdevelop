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

#ifndef QXQTEST_QTESTCASETEST_H
#define QXQTEST_QTESTCASETEST_H

#include <QtTest/QtTest>
#include <qtestcase.h>

class QTestCaseTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void constructDefault();
    void construct();
    void addCommand();
    void addCommands();

private:
    QFileInfo m_exe;
    QString m_name;
    QxQTest::QTestCase* m_case;
};

#endif // QXQTEST_QTESTCASETEST_H
