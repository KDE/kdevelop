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

#ifndef VERITAS_COVERAGE_LCOVPREFSWIDGET_H
#define VERITAS_COVERAGE_LCOVPREFSWIDGET_H

#include <QtGui/QWidget>

#include "ui_lcovprefswidget.h"

#include "colorrange.h"

class LCovSettingsBase;
class StopPointWidget;

/**
 * Preferences widget for coverage plugin.
 * This widget manages the ColorRange configuration used in coverage report
 * colors, as it can be managed by the KDE Configuration Compiler itself.
 * However, those widgets managed by the KDE Configuration Compiler aren't
 * managed in any way by LCovPrefsWidget.
 *
 * This LCovPrefsWidget is meant to be used with LCovSettingsBase and LCovPrefs
 * objects.
 *
 * LCovPrefs manages the dialog, and calls the appropriate methods in
 * LCovSettingsBase when it is loaded, saved, or default values are set. When
 * the ColorRange is modified by the user, LCovPrefsWidget emits changed(bool)
 * signal. The parameter tells whether the ColorRange is different to the saved
 * one (true), or equal to it (false), so "Apply" button can be enabled or
 * disabled as needed by LCovPrefs.
 *
 * LCovSettingsBase manages the configuration: load, save and set default
 * values. Using that class it can be checked whether the actual ColorRange is
 * equal to the saved one. In order to show changes to the ColorRange made
 * outside LCovPrefsWidget (for example, when setting the default ColorRange in
 * LCovSettingsBase), reloadColorRange() slot is called.
 */
class LCovPrefsWidget: public QWidget
{
Q_OBJECT
public:

    /**
     * Creates a new LCovPrefsWidget.
     *
     * @param settings The base settings that contain the ColorRange to show.
     * @param parent The parent widget.
     */
    explicit LCovPrefsWidget(LCovSettingsBase* settings, QWidget* parent = 0);

    /**
     * Destroys this LCovPrefsWidget.
     */
    virtual ~LCovPrefsWidget();

public slots:

    /**
     * Changes the mode of the ColorRange.
     * Emits changed(bool).
     *
     * @param mode "Discrete" or "Gradient", depending on the mode to set.
     */
    void changeMode(const QString& mode);

    /**
     * Changes the color of the stop point at the given index.
     * Emits changed(bool).
     *
     * @param index The index of the stop point.
     * @param color The color to set.
     */
    void changeColor(int index, const QColor& color);

    /**
     * Changes the position of the stop point at the given index.
     * The range for the previous and next StopPointWidget are updated.
     * Emits changed(bool).
     *
     * @param index The index of the stop point.
     * @param position The position to set.
     */
    void changePosition(int index, double position);

    /**
     * Adds a new stop point after the one at the given index.
     * The value of the stop points that follow the added one are updated, so
     * they don't overlap.
     * The stop points are reloaded.
     * Emits changed(bool).
     *
     * @param index The index to add the stop point after.
     */
    void addStopPointAfter(int index);

    /**
     * Removes the stop point at the given index.
     * The stop points are reloaded.
     * Emits changed(bool).
     *
     * @param index The index of the stop point to remove.
     */
    void removeStopPoint(int index);

    /**
     * Sets the type combobox to the current type of the ColorRange, and reloads
     * the list of stop points.
     * Emits changed(bool).
     */
    void reloadColorRange();

signals:

    /**
     * Emitted when the ColorRange changes.
     *
     * @param changed True if it changed and it is different to the saved one,
     *        false otherwise.
     */
    void changed(bool changed);

private:

    /**
     * The Qt Designer widget file.
     */
    Ui::LCovPrefsWidget mUi;

    /**
     * The base settings that contain the color range to show.
     */
    LCovSettingsBase* mSettings;

    /**
     * Reloads the list of StopPointWidgets.
     * The previous StopPointWidgets are removed and deleted, and a new
     * StopPointWidget is added for each StopPoint in the current mode of the
     * ColorRange.
     * If there is only one stop point, a dummy StopPointWidget is added first.
     */
    void reloadStopPoints();

    /**
     * Creates a new StopPointWidget.
     *
     * @param stopPoints The list of StopPoints.
     * @param index The index of the StopPoint to create the widget for.
     * @param mode The mode of the StopPoint.
     * @return The new StopPointWidget.
     */
    StopPointWidget* newStopPointWidgetFor(
                            const Veritas::StopPoints& stopPoints, int index,
                            Veritas::ColorRange::Mode mode);

    /**
     * Creates a new dummy StopPointWidget.
     *
     * @return The new dummy StopPointWidget.
     */
    StopPointWidget* newDummyStopPointWidget();

    /**
     * Returns the StopPointWidget at the given index.
     *
     * @param index The index of the StopPointWidget.
     * @return The StopPointWidget.
     */
    StopPointWidget* getStopPointWidget(int index);

    /**
     * Updates the range of the StopPointWidget at the given index.
     * The range is set to the previous and next stop point positions, or 0 or 1
     * if there are no previous or next stop points.
     *
     * @param index The index of the StopPointWidget.
     */
    void updateRange(int index);

    /**
     * Emits changed(bool).
     * The boolean value is set to true if the actual color range in the
     * settings is different to the saved one, false otherwise.
     */
    void checkIfChanged();

};

#endif
