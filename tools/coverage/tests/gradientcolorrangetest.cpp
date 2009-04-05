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

#include "gradientcolorrangetest.h"
#include "../gradientcolorrange.h"

#include "testutils.h"

using Veritas::GradientColorRangeTest;

void GradientColorRangeTest::testConstructor()
{
    GradientColorRange gcr;

    QCOMPARE(gcr.stopPoints().size(), 2);
    QCOMPARE(gcr.colorAt(0), QColor(Qt::black));
    QCOMPARE(gcr.colorAt(1), QColor(Qt::white));
}

void GradientColorRangeTest::testSetStopPoints()
{
    GradientColorRange gcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.23, QColor(Qt::blue)));
    stopPoints.append(StopPoint(0.42, QColor(Qt::darkGreen)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    gcr.setStopPoints(stopPoints);

    QCOMPARE(gcr.stopPoints().size(), 4);
    QCOMPARE(gcr.stopPoints()[0].first, 0.0);
    QCOMPARE(gcr.stopPoints()[0].second, QColor(Qt::red));
    QCOMPARE(gcr.stopPoints()[1].first, 0.23);
    QCOMPARE(gcr.stopPoints()[1].second, QColor(Qt::blue));
    QCOMPARE(gcr.stopPoints()[2].first, 0.42);
    QCOMPARE(gcr.stopPoints()[2].second, QColor(Qt::darkGreen));
    QCOMPARE(gcr.stopPoints()[3].first, 1.0);
    QCOMPARE(gcr.stopPoints()[3].second, QColor(Qt::cyan));
}

void GradientColorRangeTest::testColorAtWithNoMiddleStopPoints()
{
    GradientColorRange gcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(1, QColor(Qt::yellow)));
    gcr.setStopPoints(stopPoints);

    QCOMPARE(gcr.colorAt(0), QColor(Qt::red));
    assertFuzzyColor(gcr.colorAt(0.5), QColor(255, 127, 0));
    QCOMPARE(gcr.colorAt(1), QColor(Qt::yellow));
}

void GradientColorRangeTest::testColorAtWithSeveralStopPoints()
{
    GradientColorRange gcr;

    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::black)));
    stopPoints.append(StopPoint(0.333, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.667, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    gcr.setStopPoints(stopPoints);

    QCOMPARE(gcr.colorAt(0), QColor(Qt::black));
    assertFuzzyColor(gcr.colorAt(0.167), QColor(127, 0, 0));
    assertFuzzyColor(gcr.colorAt(0.333), QColor(Qt::red));
    assertFuzzyColor(gcr.colorAt(0.5), QColor(255, 127, 0));
    assertFuzzyColor(gcr.colorAt(0.667), QColor(Qt::yellow));
    assertFuzzyColor(gcr.colorAt(0.834), QColor(127, 255, 0));
    QCOMPARE(gcr.colorAt(1), QColor(Qt::green));
}

////////////////////////////// Asserts ////////////////////////////////////////

void GradientColorRangeTest::assertFuzzyColor(const QColor& actual, const QColor& expected)
{
    QVERIFY(actual.red() >= expected.red() -1);
    QVERIFY(actual.red() <= expected.red() + 1);
    QVERIFY(actual.green() >= expected.green() -1);
    QVERIFY(actual.green() <= expected.green() + 1);
    QVERIFY(actual.blue() >= expected.blue() -1);
    QVERIFY(actual.blue() <= expected.blue() + 1);
}

QTEST_KDEMAIN(GradientColorRangeTest, NoGUI)

#include "gradientcolorrangetest.moc"
