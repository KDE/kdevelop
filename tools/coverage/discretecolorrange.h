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

#ifndef VERITAS_COVERAGE_DISCRETECOLORRANGE_H
#define VERITAS_COVERAGE_DISCRETECOLORRANGE_H

#include <QColor>
#include <QPair>
#include <QVector>

#include "coverageexport.h"

namespace Veritas
{

typedef QPair<double, QColor> StopPoint;
typedef QVector<StopPoint> StopPoints;

/**
 * A collection of colors associated with discrete, non-overlapping ranges from
 * 0 to 1 (both included).
 * A stop point associate the range from the previous stop point (or
 * the beginning) to that point with a color. There is always at least one stop
 * point, the one at position 1.
 *
 * Method colorAt(double) can be used to know the color at some position (no
 * matter if there is a stop point or not at that position).
 *
 * Setting a red stop point at 0.333, a yellow stop point at 0.667 and a green
 * stop point at 1, the following values would be returned by colorAt:
 *  - [0, 0.333] -> Red
 *  - (0.333, 0.667] -> Yellow
 *  - (0.667, 1] -> Green
 */
class VERITAS_COVERAGE_EXPORT DiscreteColorRange
{
public:

    /**
     * Creates a new DiscreteColorRange.
     * The color at 1 is set to white, and there are no other stop points.
     */
    DiscreteColorRange();

    /**
     * Destroys this DiscreteColorRange.
     */
    virtual ~DiscreteColorRange();

    /**
     * Returns the color at the given position.
     *
     * @param position The position, in the range 0 to 1 (both included).
     * @return The color.
     */
    QColor colorAt(double position) const;

    /**
     * Returns the stop points.
     *
     * @return The stop points.
     */
    StopPoints stopPoints() const;

    /**
     * Sets the stop points.
     * The StopPoints must be in the range [0, 1], in ascending order, and they
     * must contain only valid colors. The list must contain at least the stop
     * point located at position 1.
     *
     * @param stopPoints The StopPoints to set.
     */
    void setStopPoints(const StopPoints& stopPoints);

private:

    /**
     * The stop points.
     */
    StopPoints mStopPoints;

};

}

#endif
