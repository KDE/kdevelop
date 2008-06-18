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

#ifndef QXQTEST_KASSERTS_H
#define QXQTEST_KASSERTS_H

#include <QtTest/QtTest>

namespace QTest
{

template<> inline char* toString(const QFileInfo& fi)
{
    return qstrdup(fi.filePath().toLatin1().constData());
}
template<> inline char* toString(const QVariant& va)
{
    return qstrdup(va.toString().toLatin1().constData());
}

} // namespace QTest

#define KVERIFY_MSG(condition,message) QVERIFY2(condition, QTest::toString(message))
#define KVERIFY(condition) QVERIFY(condition)
#define KOMPARE_MSG(expected,actual,message) QVERIFY2(expected == actual, QTest::toString(message))
#define KOMPARE(expected,actual) QVERIFY2(expected == actual, KOMPARE_ERR_MSG(expected, actual))
// this is not quite what we want if expected/actual modifies stuff, and thus is caled twice ...
#define KTODO QWARN("Test command not implemented yet")

#define KOMPARE_ERR_MSG(expected, actual) QString(QString("expected: '") +\
        QTest::toString(expected) + "' actual: '" + QTest::toString(actual) + "'").toAscii()

#endif // QXQTEST_KASSERTS_H
