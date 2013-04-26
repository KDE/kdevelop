/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ILAUNCHMODE_H
#define KDEVPLATFORM_ILAUNCHMODE_H

#include "interfacesexport.h"

class QString;
class KIcon;

namespace KDevelop
{

/**
 * This class is used to identify in which "mode" a given
 * launch configuration should be started. Typical modes are "Debug", 
 * "Execute" or "Profile".
 * @see ILauncher
 */
class KDEVPLATFORMINTERFACES_EXPORT ILaunchMode
{
public:
    virtual ~ILaunchMode();

    /**
     * Provide an icon for this launch mode for the GUI.
     * @returns an icon for menus/toolbars
     */
    virtual KIcon icon() const = 0;
    
    /**
     * Provide a unique ID for this launch mode.
     * This is used for example from ILauncher::supportedModes()
     * @returns a unique ID for this launchmode
     */
    virtual QString id() const = 0;
    
    /**
     * A translated name for this launch mode.
     * For example:
     * \code
     * QString ExecuteMode::name() { return i18n("Execute"); }
     * \endcode
     * @returns a human readable name
     */
    virtual QString name() const = 0;
};

}

#endif

