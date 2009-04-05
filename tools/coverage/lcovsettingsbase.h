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

#ifndef VERITAS_COVERAGE_LCOVSETTINGSBASE_H
#define VERITAS_COVERAGE_LCOVSETTINGSBASE_H

#include <KConfigSkeleton>

#include "colorrange.h"

/**
 * Base class for LCovSettings.
 * This class manages the configuration of coverage report colors, as it can't
 * be managed using the KDE Configuration Compiler.
 *
 * It is done implementing the usrReadConfig(), usrWriteConfig(),
 * usrSetDefaults() and usrUseDefaults() protected methods. KConfigSkeleton
 * takes care of calling them when needed.
 *
 * In order to allow the modification of the coverage report colors, the actual
 * ColorRange is exposed in colorRange() method. It can be checked whether the
 * actual ColorRange is equal to the default one or to the last saved one using
 * isDefaultColorRange() and isSavedColorRange().
 *
 * When the ColorRange changes (due to being read or set to the default value),
 * colorRangeChanged() is emitted.
 */
class LCovSettingsBase: public KConfigSkeleton
{
Q_OBJECT
public:

    /**
     * Destroys this LCovSettingsBase.
     */
    virtual ~LCovSettingsBase();

    /**
     * Returns the actual ColorRange.
     *
     * @return The actual ColorRange.
     */
    Veritas::ColorRange* colorRange();

    /**
     * Whether the actual ColorRange is equal to the default one or not.
     *
     * @return True if the actual ColorRange is equal to the default one, false
     *         otherwise.
     */
    bool isDefaultColorRange() const;

    /**
     * Whether the actual ColorRange is equal to the saved one or not.
     *
     * @return True if the actual ColorRange is equal to the saved one, false
     *         otherwise.
     */
    bool isSavedColorRange() const;

signals:

    /**
     * Emitted when the actual ColorRange changes (due to the configuration
     * being read or the default values set).
     * Note that it won't be emitted if the configuration is read but the actual
     * ColorRange is equal to the newly read one, or if the default values are
     * set and the actual ColorRange is equal to the default one.
     */
    void colorRangeChanged();

protected:

    /**
     * Creates a new LCovSettingsBase.
     * Actual and saved ColorRange are set to the default one.
     *
     * @param configurationFileName The name of the configuration file.
     */
    LCovSettingsBase(const QString &configurationFileName);

    /**
     * Loads the saved ColorRange from the configuration.
     * If there is no entry for it, the saved ColorRange is the default one.
     * The actual ColorRange is set to the saved one.
     */
    virtual void usrReadConfig();

    /**
     * Sets the actual ColorRange to the default one.
     */
    virtual void usrSetDefaults();

    /**
     * Swaps default and actual ColorRange values.
     *
     * @param useDefaults Ignored.
     * @return False (ignored in KConfigSkeleton::useDefaults(bool)).
     */
    virtual bool usrUseDefaults(bool useDefaults);

    /**
     * Saves the ColorRange to the configuration.
     * If the actual ColorRange is equal to the default ColorRange, it is
     * removed from the configuration.
     */
    virtual void usrWriteConfig();

private:

    /**
     * The actual ColorRange.
     */
    Veritas::ColorRange mColorRange;

    /**
     * The saved ColorRange.
     */
    Veritas::ColorRange mSavedColorRange;

    /**
     * The default ColorRange.
     */
    Veritas::ColorRange mDefaultColorRange;

};

#endif
