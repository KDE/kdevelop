/* KDevelop coverage plugin
 *    Copyright 2009 Daniel Calviño Sánchez <danxuliu@gmail.com>
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

#ifndef VERITAS_COVERAGE_COLORRANGETEST_H
#define VERITAS_COVERAGE_COLORRANGETEST_H

#include <QtCore/QObject>
#include <QColor>

namespace Veritas {

class ColorRangeTest: public QObject
{
Q_OBJECT
private slots:

    void cleanup();

    void testConstructor();
    void testSetMode();
    void testSetStopPoints();
    void testColorAtDiscreteMode();
    void testColorAtGradientMode();

    void testLoad();
    void testSave();
    void testLoadAfterSave();

    void testOperatorEqual();
    void testOperatorEqualDifferentMode();
    void testOperatorEqualDifferentDiscreteColorRange();
    void testOperatorEqualDifferentGradientColorRange();

private:

    void assertFuzzyColor(const QColor& actual, const QColor& expected);

};

}

#endif
