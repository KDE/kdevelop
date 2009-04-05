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

#include "discretecolorrange.h"

using Veritas::DiscreteColorRange;
using Veritas::StopPoints;

DiscreteColorRange::DiscreteColorRange()
{
    mStopPoints.append(StopPoint(1, QColor(Qt::white)));
}

DiscreteColorRange::~DiscreteColorRange()
{
}

QColor DiscreteColorRange::colorAt(double position) const
{
    Q_ASSERT(position >= 0 && position <= 1);

    if (position >= 0 && position <= mStopPoints[0].first) {
        return mStopPoints[0].second;
    }

    for (int i=1; i<mStopPoints.size()-1; ++i) {
        if (position > mStopPoints[i-1].first &&
                position <= mStopPoints[i].first) {
            return mStopPoints[i].second;
        }
    }

    return mStopPoints.last().second;
}

StopPoints DiscreteColorRange::stopPoints() const
{
    return mStopPoints;
}

void DiscreteColorRange::setStopPoints(const StopPoints& stopPoints)
{
    Q_ASSERT(stopPoints.size() >= 1);
    Q_ASSERT(stopPoints.last().first == 1);

    mStopPoints = stopPoints;
}
