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

#include "discretecolorrangetest.h"
#include "../discretecolorrange.h"

#include "testutils.h"

using Veritas::DiscreteColorRangeTest;

void DiscreteColorRangeTest::testConstructor()
{
    DiscreteColorRange dcr;

    QCOMPARE(dcr.stopPoints().size(), 1);
    QCOMPARE(dcr.colorAt(1), QColor(Qt::white));
}

void DiscreteColorRangeTest::testSetStopPoints()
{
    DiscreteColorRange dcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(0.23, QColor(Qt::blue)));
    stopPoints.append(StopPoint(0.42, QColor(Qt::darkGreen)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    dcr.setStopPoints(stopPoints);

    QCOMPARE(dcr.stopPoints().size(), 3);
    QCOMPARE(dcr.stopPoints()[0].first, 0.23);
    QCOMPARE(dcr.stopPoints()[0].second, QColor(Qt::blue));
    QCOMPARE(dcr.stopPoints()[1].first, 0.42);
    QCOMPARE(dcr.stopPoints()[1].second, QColor(Qt::darkGreen));
    QCOMPARE(dcr.stopPoints()[2].first, 1.0);
    QCOMPARE(dcr.stopPoints()[2].second, QColor(Qt::cyan));
}

void DiscreteColorRangeTest::testColorAtWithNoMiddleStopPoints()
{
    DiscreteColorRange dcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(1, QColor(QColor(Qt::green))));
    dcr.setStopPoints(stopPoints);

    QCOMPARE(dcr.colorAt(0), QColor(Qt::green));
    QCOMPARE(dcr.colorAt(0.5), QColor(Qt::green));
    QCOMPARE(dcr.colorAt(1), QColor(Qt::green));
}

void DiscreteColorRangeTest::testColorAtWithSeveralStopPoints()
{
    DiscreteColorRange dcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(0.333, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.667, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    dcr.setStopPoints(stopPoints);

    QCOMPARE(dcr.colorAt(0), QColor(Qt::red));
    QCOMPARE(dcr.colorAt(0.23), QColor(Qt::red));
    QCOMPARE(dcr.colorAt(0.333), QColor(Qt::red));
    QCOMPARE(dcr.colorAt(0.334), QColor(Qt::yellow));
    QCOMPARE(dcr.colorAt(0.42), QColor(Qt::yellow));
    QCOMPARE(dcr.colorAt(0.667), QColor(Qt::yellow));
    QCOMPARE(dcr.colorAt(0.668), QColor(Qt::green));
    QCOMPARE(dcr.colorAt(0.8), QColor(Qt::green));
    QCOMPARE(dcr.colorAt(1), QColor(Qt::green));
}

void DiscreteColorRangeTest::testColorAtWithStopPointAtBegin()
{
    DiscreteColorRange dcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    dcr.setStopPoints(stopPoints);

    QCOMPARE(dcr.colorAt(0), QColor(Qt::black));
    QCOMPARE(dcr.colorAt(0.001), QColor(Qt::green));
    QCOMPARE(dcr.colorAt(1), QColor(Qt::green));
}

QTEST_KDEMAIN(DiscreteColorRangeTest, NoGUI)

#include "discretecolorrangetest.moc"
