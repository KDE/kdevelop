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

#ifndef VERITAS_COVERAGE_GRADIENTCOLORRANGE_H
#define VERITAS_COVERAGE_GRADIENTCOLORRANGE_H

#include <QColor>
#include <QPair>
#include <QVector>

#include "coverageexport.h"

namespace Veritas
{

typedef QPair<double, QColor> StopPoint;
typedef QVector<StopPoint> StopPoints;

/**
 * A gradient of colors, each of them associated with a point in range 0 to 1
 * (both included).
 * A stop point associate a point with a color. There are always at least two
 * stop points, associated with positions 0 and 1. Any other position has a
 * color calculated as a gradient between the set stop points.
 *
 * Method colorAt(double) can be used to know the color at some position (no
 * matter if there is a stop point or not at that position). A color between two
 * stop points is the linear interpolated color between them.
 *
 * Setting a red stop point at 0, a yellow stop point at 0.5 and a green stop
 * point at 1, the following values would be returned by colorAt:
 *  - 0 -> Red
 *  - 0.25 -> Orange
 *  - 0.5 -> Yellow
 *  - 0.75 -> Yellowish green
 *  - 1 -> Green
 */
class VERITAS_COVERAGE_EXPORT GradientColorRange
{
public:

    /**
     * Creates a new GradientColorRange.
     * The color at 0 is set to black, the color at 1 is set to white, and there
     * are no middle stop points.
     */
    GradientColorRange();

    /**
     * Destroys this GradientColorRange.
     */
    virtual ~GradientColorRange();

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
     * points located at position 0 and 1.
     *
     * @param stopPoints The StopPoints to set.
     */
    void setStopPoints(const StopPoints& stopPoints);

private:

    /**
     * The stop points.
     */
    StopPoints mStopPoints;

    /**
     * Interpolates a color between two stop points.
     * Each component (red, green and blue) of the returned color is the result
     * of applying linear interpolation between the components of the begin and
     * end colors at the given position.
     *
     * @param begin The first stop point.
     * @param end The second stop point.
     * @param position The position of the color to interpolate.
     * @return The interpolated color.
     */
    QColor interpolateColor(const StopPoint& begin, const StopPoint& end,
                            double position) const;

};

}

#endif
