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

#ifndef QXQTEST_QTESTOUTPUTPARSERTEST
#define QXQTEST_QTESTOUTPUTPARSERTEST

#include <QtTest/QtTest>

namespace QTest
{
class QTestCase;
}

namespace QTest
{
namespace ut {

class QTestOutputParserTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void parse_data();
    void parse();
    void initFailure();
    void cleanupFailure();

private:
    void setupColumns();
    void addSunnyDayData();
    void addBasicFailureData();

private:
    QTestCase* m_caze;
};

}
}

#endif // QXQTEST_QTESTOUTPUTPARSERTEST
