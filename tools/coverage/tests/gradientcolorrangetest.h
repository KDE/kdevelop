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

#ifndef VERITAS_COVERAGE_GRADIENTCOLORRANGETEST_H
#define VERITAS_COVERAGE_GRADIENTCOLORRANGETEST_H

#include <QtCore/QObject>
#include <QColor>

namespace Veritas {

class GradientColorRangeTest: public QObject
{
Q_OBJECT
private slots:

    void testConstructor();
    void testSetStopPoints();
    void testColorAtWithNoMiddleStopPoints();
    void testColorAtWithSeveralStopPoints();

private:

    /**
     * Checks that actual has the three RGB components equal to the RGB
     * components +-1 of expected.
     */
    void assertFuzzyColor(const QColor& actual, const QColor& expected);

};

}

#endif
