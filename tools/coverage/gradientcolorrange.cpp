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

#include "gradientcolorrange.h"

using Veritas::GradientColorRange;
using Veritas::StopPoints;

GradientColorRange::GradientColorRange()
{
    mStopPoints.append(StopPoint(0, QColor(Qt::black)));
    mStopPoints.append(StopPoint(1, QColor(Qt::white)));
}

GradientColorRange::~GradientColorRange()
{
}

QColor GradientColorRange::colorAt(double position) const
{
    Q_ASSERT(position >= 0 && position <= 1);

    for (int i=0; i<stopPoints().size(); i++) {
        if (position == stopPoints()[i].first) {
            return stopPoints()[i].second;
        } else if (position < stopPoints()[i+1].first) {
            return interpolateColor(stopPoints()[i], stopPoints()[i+1],
                                    position);
        }
    }

    return mStopPoints.last().second;
}

StopPoints GradientColorRange::stopPoints() const
{
    return mStopPoints;
}

void GradientColorRange::setStopPoints(const StopPoints& stopPoints)
{
    Q_ASSERT(stopPoints.size() >= 2);
    Q_ASSERT(stopPoints.first().first == 0);
    Q_ASSERT(stopPoints.last().first == 1);

    mStopPoints = stopPoints;
}

QColor GradientColorRange::interpolateColor(const StopPoint& begin,
                                            const StopPoint& end,
                                            double position) const
{
    double beginPosition = begin.first;
    double endPosition = end.first;
    double width = endPosition - beginPosition;

    double relativePosition = (position - beginPosition) / width;

    QColor beginColor = begin.second;
    QColor endColor = end.second;

    QColor interpolatedColor;
    interpolatedColor.setRed(qRound(beginColor.red() +
                (endColor.red() - beginColor.red()) * relativePosition));
    interpolatedColor.setGreen(qRound(beginColor.green() +
                (endColor.green() - beginColor.green()) * relativePosition));
    interpolatedColor.setBlue(qRound(beginColor.blue() +
                (endColor.blue() - beginColor.blue()) * relativePosition));

    return interpolatedColor;
}
