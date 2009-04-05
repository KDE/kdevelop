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

#ifndef VERITAS_COVERAGE_COLORRANGE_H
#define VERITAS_COVERAGE_COLORRANGE_H

#include <QColor>
#include <QPair>
#include <QVector>

#include "coverageexport.h"

#include "discretecolorrange.h"
#include "gradientcolorrange.h"

class KConfigGroup;

namespace Veritas
{

typedef QPair<double, QColor> StopPoint;
typedef QVector<StopPoint> StopPoints;

/**
 * Colors associated with subranges or points in range 0 to 1 (both included).
 * ColorRange has two independent modes: discrete and gradient. In discrete
 * mode, each color is associated with a subrange of the full range. In gradient
 * mode, each color is associated with a point, and the other points are
 * calculated as a gradient between the set points. For further information
 * about each mode, see the documentation of DiscreteColorRange and
 * GradientColorRange.
 *
 * The changes made in each mode doesn't affect those made in the other mode.
 * Also note that depending on the current mode, the semantics of the stop
 * points change a bit, as discrete mode uses 1 stop point less than gradient
 * mode, and in discrete mode they represent subranges and in gradient mode just
 * points.
 *
 * A ColorRange can be loaded from and saved to a KConfigurationGroup.
 *
 * @see DiscreteColorRange
 * @see GradientColorRange
 */
class VERITAS_COVERAGE_EXPORT ColorRange
{
public:

    enum Mode {
        Gradient = 0,
        Discrete
    };

    /**
     * Creates a new ColorRange.
     * Default mode is Discrete.
     */
    ColorRange();

    /**
     * Destroys this ColorRange.
     */
    virtual ~ColorRange();

    /**
     * Returns the current mode of this ColorRange.
     *
     * @return The current mode of this ColorRange.
     */
    Mode mode() const;

    /**
     * Sets the current mode of this ColorRange.
     *
     * @param mode The current mode of this ColorRange.
     */
    void setMode(Mode mode);

    /**
     * Returns the color for the current mode at the given position.
     *
     * @param position The position, in the range 0 to 1 (both included).
     * @return The color.
     */
    QColor colorAt(double position) const;

    /**
     * Returns the stop points for the current mode.
     *
     * @return The stop points.
     */
    StopPoints stopPoints() const;

    /**
     * Sets the stop points for the current mode.
     *
     * @param stopPoints The StopPoints to set.
     * @see DiscreteColorRange::setStopPoints(const StopPoints&)
     * @see GradientColorRange::setStopPoints(const StopPoints&)
     */
    void setStopPoints(const StopPoints& stopPoints);

    /**
     * Loads this ColorRange from the given configuration group.
     *
     * @param group The configuration group to load this ColorRange from.
     * @see save(KConfigGroup&)
     */
    void load(const KConfigGroup& group);

    /**
     * Saves this ColorRange to the given configuration group.
     * The configuration group is expected to be empty. Also note that no sync
     * is made, only the needed groups and entries are added.
     *
     * The current mode of this ColorRange is saved, as a QString, in "Mode"
     * entry (with values "Discrete" and "Gradient"). The StopPoints of discrete
     * mode are saved in a group called "Discrete", and the StopPoints of
     * gradient mode are saved in a group called "Gradient". Note that both
     * modes are saved, no matter which is the current mode.
     *
     * @param group The empty configuration group to save this ColorRange to.
     * @see saveStopPoints(const KConfigGroup&, const StopPoints&)
     */
    void save(KConfigGroup& group);

    /**
     * Check whether this ColorRange is equal to the given one or not.
     *
     * @param colorRange The ColorRange to compare to.
     * @return True if both ColorRange are equal, false otherwise.
     */
    bool operator==(const ColorRange& colorRange) const;

    /**
     * Check whether this ColorRange is different to the given one or not.
     *
     * @param colorRange The ColorRange to compare to.
     * @return True if both ColorRange are different, false otherwise.
     */
    bool operator!=(const ColorRange& colorRange) const;

private:

    Mode mMode;
    DiscreteColorRange mDiscreteColorRange;
    GradientColorRange mGradientColorRange;

    /**
     * Loads the StopPoints stored in the given configuration group.
     *
     * @param group The configuration group to load the StopPoints from.
     * @return The loaded StopPoints.
     * @see saveStopPoints(const KConfigGroup&, const StopPoints&)
     */
    StopPoints loadStopPoints(const KConfigGroup& group) const;

    /**
     * Saves the StopPoints in the given configuration group.
     * The number of StopPoints are saved, as an integer, in
     * "NumberOfStopPoints" entry. For each StopPoint, to entries are saved:
     * "StopPointXPosition", a real, with the position of the StopPoint, and
     * "StopPointXColor", a QColor, with the color of the StopPoint.
     * In both names, the X refer to the index of the StopPoint, indexed at 0.
     *
     * @param group The configuration group to save the StopPoints to.
     * @param stopPoints The StopPoints to save.
     */
    void saveStopPoints(KConfigGroup& group,
                        const StopPoints& stopPoints) const;

};

}

#endif
