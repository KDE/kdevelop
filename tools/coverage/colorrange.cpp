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

#include "colorrange.h"

#include <KConfigGroup>

using Veritas::ColorRange;
using Veritas::StopPoints;

ColorRange::ColorRange()
{
    mMode = Discrete;
}

ColorRange::~ColorRange()
{
}

ColorRange::Mode ColorRange::mode() const
{
    return mMode;
}

void ColorRange::setMode(Mode mode)
{
    mMode = mode;
}

QColor ColorRange::colorAt(double position) const
{
    if (mMode == Discrete) {
        return mDiscreteColorRange.colorAt(position);
    }

    return mGradientColorRange.colorAt(position);
}

StopPoints ColorRange::stopPoints() const
{
    if (mMode == Discrete) {
        return mDiscreteColorRange.stopPoints();
    }

    return mGradientColorRange.stopPoints();
}

void ColorRange::setStopPoints(const StopPoints& stopPoints)
{
    if (mMode == Discrete) {
        mDiscreteColorRange.setStopPoints(stopPoints);
    } else {
        mGradientColorRange.setStopPoints(stopPoints);
    }
}

void ColorRange::load(const KConfigGroup& group)
{
    if (group.readEntry("Mode") == "Gradient") {
        mMode = Gradient;
    } else {
        mMode = Discrete;
    }

    mDiscreteColorRange.setStopPoints(loadStopPoints(group.group("Discrete")));
    mGradientColorRange.setStopPoints(loadStopPoints(group.group("Gradient")));
}

void ColorRange::save(KConfigGroup& group)
{
    if (mMode == Gradient) {
        group.writeEntry("Mode", "Gradient");
    } else {
        group.writeEntry("Mode", "Discrete");
    }

    KConfigGroup discreteGroup(&group, "Discrete");
    saveStopPoints(discreteGroup, mDiscreteColorRange.stopPoints());

    KConfigGroup gradientGroup(&group, "Gradient");
    saveStopPoints(gradientGroup, mGradientColorRange.stopPoints());
}

bool ColorRange::operator==(const ColorRange& colorRange) const
{
    if (mMode != colorRange.mMode) {
        return false;
    }

    if (mDiscreteColorRange.stopPoints() !=
            colorRange.mDiscreteColorRange.stopPoints()) {
        return false;
    }

    if (mGradientColorRange.stopPoints() !=
            colorRange.mGradientColorRange.stopPoints()) {
        return false;
    }

    return true;
}

bool ColorRange::operator!=(const ColorRange& colorRange) const
{
    return !(*this == colorRange);
}

StopPoints ColorRange::loadStopPoints(const KConfigGroup& group) const
{
    StopPoints stopPoints;

    int numberOfStopPoints = group.readEntry("NumberOfStopPoints", 0);
    for (int i=0; i<numberOfStopPoints; ++i) {
        QString baseName = QString("StopPoint") + QString::number(i);

        StopPoint stopPoint;
        stopPoint.first = group.readEntry(baseName + "Position", 0.0);
        stopPoint.second = group.readEntry(baseName + "Color", QColor());

        stopPoints.append(stopPoint);
    }

    return stopPoints;
}

void ColorRange::saveStopPoints(KConfigGroup& group,
                                const StopPoints& stopPoints) const
{
    group.writeEntry("NumberOfStopPoints", stopPoints.size());
    for (int i=0; i<stopPoints.size(); ++i) {
        QString baseName = QString("StopPoint") + QString::number(i);

        group.writeEntry(baseName + "Position", stopPoints[i].first);
        group.writeEntry(baseName + "Color", stopPoints[i].second);
    }
}
