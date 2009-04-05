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

#ifndef VERITAS_COVERAGE_STOPPOINTWIDGET_H
#define VERITAS_COVERAGE_STOPPOINTWIDGET_H

#include <QtGui/QWidget>

#include "colorrange.h"

namespace Ui
{
class StopPointWidget;
}

/**
 * Widget to display and edit a StopPoint.
 * StopPoints are used to define ColorRanges: each StopPoint contains a Color
 * and its position (in [0, 1] range), and a ColorRange contains several
 * StopPoints for each mode (discrete and gradient).
 *
 * This widget is used with LCovPrefsWidget to modify a ColorRange.
 * LCovPrefsWidget contains as many StopPointWidgets as StopPoint are in the
 * current mode of the ColorRange.
 *
 * Each StopPointWidget displays the values for a StopPoint, identified by
 * its index. When the values are modified, LCovPrefsWidget modifies the
 * ColorRange to reflect the changes.
 *
 * As the position of a StopPoint must be greater than the position of the 
 * previous one, and lower than the position of the next one, the valid range of
 * values for the StopPoint can be set using setRange(double, double).
 *
 * Although the methods expect positions in range [0, 1], the position of the
 * StopPoint is shown to the user as a percentage.
 *
 * There is a special case in StopPointWidgets: dummy StopPointWidget. A dummy
 * StopPointWidget only has its add button visible. It is used in discrete mode
 * when there is only one stop point. A dummy StopPointWidget is added before
 * its StopPointWidget to allow the user insert a new StopPoint (as the widget
 * for the last and unique StopPoint would have its add button disabled).
 */
class StopPointWidget: public QWidget
{
Q_OBJECT
public:

    /**
     * Creates a new StopPointWidget.
     *
     * @param parent The parent widget.
     */
    explicit StopPointWidget(QWidget* parent = 0);

    /**
     * Destroys this StopPointWidget.
     */
    virtual ~StopPointWidget();

    /**
     * Sets the StopPoint.
     * A StopPoint at position 1 has its position, add and remove buttons 
     * disabled. Also, in Gradient mode, a StopPoint at 0 has its position and 
     * add button disabled.
     *
     * @param stopPoint The StopPoint.
     */
    void setStopPoint(const Veritas::StopPoint& stopPoint, 
                      Veritas::ColorRange::Mode mode);

    /**
     * Sets the index of the StopPoint.
     *
     * @param index The index of the StopPoint.
     */
    void setStopPointIndex(int index);

    /**
     * Sets the range of valid position values.
     * The range must be between 0 and 1, both inclusive. The range shown to the
     * user is set to [minimum * 100 + 0,01, maximum * 100 - 0,01].
     *
     * When the StopPoint position is 1, or the StopPoint position is 0 and it
     * is in GradientMode, the ranges are ignored.
     *
     * @param minimum The position of the previous stop point.
     * @param maximum The position of the next stop point.
     */
    void setRange(double minimum, double maximum);

    /**
     * Sets this StopPointWidget in dummy mode.
     *
     * @param dummy True to hide everything but the add button, false to show
     * everything.
     */
    void setDummy(bool dummy);

signals:

    /**
     * Emitted when the color for the StopPoint at the given index changes.
     *
     * @param index The index of the StopPoint.
     * @param color The new color.
     */
    void colorChanged(int index, const QColor& color);

    /**
     * Emitted when the position for the StopPoint at the given index changes.
     *
     * @param index The index of the StopPoint.
     * @param position The new position.
     */
    void positionChanged(int index, double position);

    /**
     * Emitted when a new StopPoint has to be added after the given index.
     *
     * @param index The index of the StopPoint.
     */
    void stopPointToAddAfter(int index);

    /**
     * Emitted when the StopPoint at the given index has to be removed.
     *
     * @param index The index of the StopPoint.
     */
    void stopPointToRemove(int index);

private:

    /**
     * The Qt Designer widget file.
     */
    Ui::StopPointWidget* mUi;

    /**
     * The index of the StopPoint.
     */
    int mIndex;

    /**
     * The mode of the StopPoint.
     */
    Veritas::ColorRange::Mode mMode;

private slots:

    /**
     * Emits colorChanged(int, const QColor&).
     */
    void changeColor(const QColor& color);

    /**
     * Emits positionChanged(int, double).
     */
    void changePosition(double percentage);

    /**
     * Emits stopPointToAddAfter(int).
     */
    void addStopPointAfter();

    /**
     * Emits stopPointToRemove(int).
     */
    void removeStopPoint();

};

#endif
