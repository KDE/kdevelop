/* KDevelop Coverage lcov settings
*
* Copyright 2009  Daniel Calviño Sánchez <danxuliu@gmail.com>
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

#ifndef VERITAS_COVERAGE_COLORRANGEBAR_H
#define VERITAS_COVERAGE_COLORRANGEBAR_H

#include <QtGui/QWidget>

#include "colorrange.h"

namespace Veritas
{
class ColorRange;
}

/**
 * Graphical representation of a ColorRange.
 * The ColorRangeBar should be updated when the ColorRange is modified. It can
 * be done calling update() slot.
 */
class ColorRangeBar: public QWidget {
Q_OBJECT
public:

    /**
     * Creates a new ColorRangeBar for the given ColorRange.
     *
     * @param colorRange The ColorRange to show.
     * @parent The parent widget.
     */
    ColorRangeBar(Veritas::ColorRange* colorRange, QWidget* parent);

    /**
     * Paints the ColorRange in this ColorRangeBar.
     */
    virtual void paintEvent(QPaintEvent*);

private:

    /**
     * The ColorRange to show.
     */
    Veritas::ColorRange* mColorRange;

    /**
     * Paints the ColorRange when it is in discrete mode.
     *
     * @param rectangle The rectanble to paint the ColorRange to.
     * @param painter The painter to use.
     */
    void paintDiscreteColorRange(const QRect& rectangle, QPainter* painter);

    /**
     * Paints the ColorRange when it is in gradient mode.
     *
     * @param rectangle The rectanble to paint the ColorRange to.
     * @param painter The painter to use.
     */
    void paintGradientColorRange(const QRect& rectangle, QPainter* painter);

};

#endif
