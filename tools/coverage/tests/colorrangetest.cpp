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

#include "colorrangetest.h"
#include "../colorrange.h"

#include "testutils.h"

#include <QFile>

#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

using Veritas::ColorRangeTest;

static const char* filename = "coverageConfigurationTest";

void ColorRangeTest::cleanup() {
    QFile file(KStandardDirs().findResource("config", filename));
    if (file.exists()) {
        file.remove();
    }
}

void ColorRangeTest::testConstructor()
{
    ColorRange cr;

    QCOMPARE(cr.mode(), ColorRange::Discrete);
    QCOMPARE(cr.stopPoints().size(), 1);
    QCOMPARE(cr.colorAt(1), QColor(Qt::white));

    cr.setMode(ColorRange::Gradient);
    QCOMPARE(cr.stopPoints().size(), 2);
    QCOMPARE(cr.colorAt(0), QColor(Qt::black));
    QCOMPARE(cr.colorAt(1), QColor(Qt::white));
}

void ColorRangeTest::testSetMode()
{
    ColorRange cr;
    cr.setMode(ColorRange::Gradient);

    QCOMPARE(cr.mode(), ColorRange::Gradient);

    cr.setMode(ColorRange::Discrete);

    QCOMPARE(cr.mode(), ColorRange::Discrete);
}

void ColorRangeTest::testSetStopPoints()
{
    ColorRange cr;

    cr.setMode(ColorRange::Discrete);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Gradient);
    stopPoints.clear();
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Discrete);
    QCOMPARE(cr.stopPoints().size(), 1);
    QCOMPARE(cr.stopPoints()[0].first, 1.0);
    QCOMPARE(cr.stopPoints()[0].second, QColor(Qt::cyan));

    cr.setMode(ColorRange::Gradient);
    QCOMPARE(cr.stopPoints().size(), 2);
    QCOMPARE(cr.stopPoints()[0].first, 0.0);
    QCOMPARE(cr.stopPoints()[0].second, QColor(Qt::red));
    QCOMPARE(cr.stopPoints()[1].first, 1.0);
    QCOMPARE(cr.stopPoints()[1].second, QColor(Qt::green));
}

void ColorRangeTest::testColorAtDiscreteMode()
{
    ColorRange cr;

    cr.setMode(ColorRange::Discrete);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(1, QColor(Qt::yellow)));
    cr.setStopPoints(stopPoints);

    QCOMPARE(cr.colorAt(0.42), QColor(Qt::yellow));
}

void ColorRangeTest::testColorAtGradientMode()
{
    ColorRange cr;

    cr.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);

    assertFuzzyColor(cr.colorAt(0.5), QColor(127, 127, 0));
}

void ColorRangeTest::testLoad()
{
    KConfig config(filename);
    KConfigGroup group(&config, "Color range test");

    group.writeEntry("Mode", "Gradient");
    KConfigGroup discreteGroup(&group, "Discrete");
    discreteGroup.writeEntry("NumberOfStopPoints", 2);
    discreteGroup.writeEntry("StopPoint0Position", 0.42);
    discreteGroup.writeEntry("StopPoint0Color", QColor(Qt::black));
    discreteGroup.writeEntry("StopPoint1Position", 1.0);
    discreteGroup.writeEntry("StopPoint1Color", QColor(Qt::cyan));

    KConfigGroup gradientGroup(&group, "Gradient");
    gradientGroup.writeEntry("NumberOfStopPoints", 3);
    gradientGroup.writeEntry("StopPoint0Position", 0.0);
    gradientGroup.writeEntry("StopPoint0Color", QColor(Qt::red));
    gradientGroup.writeEntry("StopPoint1Position", 0.5);
    gradientGroup.writeEntry("StopPoint1Color", QColor(Qt::yellow));
    gradientGroup.writeEntry("StopPoint2Position", 1.0);
    gradientGroup.writeEntry("StopPoint2Color", QColor(Qt::green));

    ColorRange cr;
    cr.load(group);

    QCOMPARE(cr.mode(), ColorRange::Gradient);

    cr.setMode(ColorRange::Discrete);
    QCOMPARE(cr.stopPoints().size(), 2);
    QCOMPARE(cr.stopPoints()[0].first, 0.42);
    QCOMPARE(cr.stopPoints()[0].second, QColor(Qt::black));
    QCOMPARE(cr.stopPoints()[1].first, 1.0);
    QCOMPARE(cr.stopPoints()[1].second, QColor(Qt::cyan));

    cr.setMode(ColorRange::Gradient);
    QCOMPARE(cr.stopPoints().size(), 3);
    QCOMPARE(cr.stopPoints()[0].first, 0.0);
    QCOMPARE(cr.stopPoints()[0].second, QColor(Qt::red));
    QCOMPARE(cr.stopPoints()[1].first, 0.5);
    QCOMPARE(cr.stopPoints()[1].second, QColor(Qt::yellow));
    QCOMPARE(cr.stopPoints()[2].first, 1.0);
    QCOMPARE(cr.stopPoints()[2].second, QColor(Qt::green));
}

void ColorRangeTest::testSave()
{
    ColorRange cr;

    cr.setMode(ColorRange::Discrete);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0.42, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Gradient);
    stopPoints.clear();
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);

    KConfig config(filename);
    KConfigGroup group(&config, "Color range test");
    cr.save(group);

    QCOMPARE(group.readEntry("Mode"), QString("Gradient"));

    KConfigGroup discreteGroup = group.group("Discrete");
    QCOMPARE(discreteGroup.readEntry("NumberOfStopPoints", 0), 2);
    QCOMPARE(discreteGroup.readEntry("StopPoint0Position", 0.0), 0.42);
    QCOMPARE(discreteGroup.readEntry("StopPoint0Color", QColor()),
             QColor(Qt::black));
    QCOMPARE(discreteGroup.readEntry("StopPoint1Position", 0.0), 1.0);
    QCOMPARE(discreteGroup.readEntry("StopPoint1Color", QColor()),
             QColor(Qt::cyan));

    KConfigGroup gradientGroup = group.group("Gradient");
    QCOMPARE(gradientGroup.readEntry("NumberOfStopPoints", 0), 3);
    QCOMPARE(gradientGroup.readEntry("StopPoint0Position", 1.0), 0.0);
    QCOMPARE(gradientGroup.readEntry("StopPoint0Color", QColor()),
             QColor(Qt::red));
    QCOMPARE(gradientGroup.readEntry("StopPoint1Position", 0.0), 0.5);
    QCOMPARE(gradientGroup.readEntry("StopPoint1Color", QColor()),
             QColor(Qt::yellow));
    QCOMPARE(gradientGroup.readEntry("StopPoint2Position", 0.0), 1.0);
    QCOMPARE(gradientGroup.readEntry("StopPoint2Color", QColor()),
             QColor(Qt::green));
}

void ColorRangeTest::testLoadAfterSave()
{
    ColorRange cr;

    cr.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Discrete);
    stopPoints.clear();
    stopPoints.append(StopPoint(0.42, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);

    KConfig config(filename);
    KConfigGroup group(&config, "Color range test");
    cr.save(group);

    ColorRange newCr;
    newCr.load(group);

    QCOMPARE(newCr.mode(), ColorRange::Discrete);

    newCr.setMode(ColorRange::Discrete);
    QCOMPARE(newCr.stopPoints().size(), 2);
    QCOMPARE(newCr.stopPoints()[0].first, 0.42);
    QCOMPARE(newCr.stopPoints()[0].second, QColor(Qt::black));
    QCOMPARE(newCr.stopPoints()[1].first, 1.0);
    QCOMPARE(newCr.stopPoints()[1].second, QColor(Qt::cyan));

    newCr.setMode(ColorRange::Gradient);
    QCOMPARE(newCr.stopPoints().size(), 3);
    QCOMPARE(newCr.stopPoints()[0].first, 0.0);
    QCOMPARE(newCr.stopPoints()[0].second, QColor(Qt::red));
    QCOMPARE(newCr.stopPoints()[1].first, 0.5);
    QCOMPARE(newCr.stopPoints()[1].second, QColor(Qt::yellow));
    QCOMPARE(newCr.stopPoints()[2].first, 1.0);
    QCOMPARE(newCr.stopPoints()[2].second, QColor(Qt::green));
}

void ColorRangeTest::testOperatorEqual()
{
    ColorRange cr;
    ColorRange cr2;

    cr.setMode(ColorRange::Gradient);
    cr2.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);
    cr2.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Discrete);
    cr2.setMode(ColorRange::Discrete);
    stopPoints.clear();
    stopPoints.append(StopPoint(0.42, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);
    cr2.setStopPoints(stopPoints);

    QVERIFY(cr == cr2);
    QVERIFY(!(cr != cr2));
}

void ColorRangeTest::testOperatorEqualDifferentMode()
{
    ColorRange cr;
    ColorRange cr2;

    cr.setMode(ColorRange::Gradient);
    cr2.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);
    cr2.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Discrete);
    cr2.setMode(ColorRange::Discrete);
    stopPoints.clear();
    stopPoints.append(StopPoint(0.42, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);
    cr2.setStopPoints(stopPoints);

    cr2.setMode(ColorRange::Gradient);

    QVERIFY(!(cr == cr2));
    QVERIFY(cr != cr2);
}

void ColorRangeTest::testOperatorEqualDifferentDiscreteColorRange()
{
    ColorRange cr;
    ColorRange cr2;

    cr.setMode(ColorRange::Gradient);
    cr2.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);
    cr2.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Discrete);
    cr2.setMode(ColorRange::Discrete);
    stopPoints.clear();
    stopPoints.append(StopPoint(0.42, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);

    stopPoints.clear();
    stopPoints.append(StopPoint(0.23, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr2.setStopPoints(stopPoints);

    QVERIFY(!(cr == cr2));
    QVERIFY(cr != cr2);
}

void ColorRangeTest::testOperatorEqualDifferentGradientColorRange()
{
    ColorRange cr;
    ColorRange cr2;

    cr.setMode(ColorRange::Gradient);
    cr2.setMode(ColorRange::Gradient);
    StopPoints stopPoints;
    stopPoints.append(StopPoint(0, QColor(Qt::red)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr.setStopPoints(stopPoints);
    stopPoints.clear();
    stopPoints.append(StopPoint(0, QColor(Qt::blue)));
    stopPoints.append(StopPoint(0.5, QColor(Qt::yellow)));
    stopPoints.append(StopPoint(1, QColor(Qt::green)));
    cr2.setStopPoints(stopPoints);

    cr.setMode(ColorRange::Discrete);
    cr2.setMode(ColorRange::Discrete);
    stopPoints.clear();
    stopPoints.append(StopPoint(0.42, QColor(Qt::black)));
    stopPoints.append(StopPoint(1, QColor(Qt::cyan)));
    cr.setStopPoints(stopPoints);
    cr2.setStopPoints(stopPoints);

    QVERIFY(!(cr == cr2));
    QVERIFY(cr != cr2);
}

////////////////////////////// Asserts ////////////////////////////////////////

void ColorRangeTest::assertFuzzyColor(const QColor& actual, const QColor& expected)
{
    QVERIFY(actual.red() >= expected.red() -1);
    QVERIFY(actual.red() <= expected.red() + 1);
    QVERIFY(actual.green() >= expected.green() -1);
    QVERIFY(actual.green() <= expected.green() + 1);
    QVERIFY(actual.blue() >= expected.blue() -1);
    QVERIFY(actual.blue() <= expected.blue() + 1);
}

QTEST_KDEMAIN(ColorRangeTest, NoGUI)

#include "colorrangetest.moc"
