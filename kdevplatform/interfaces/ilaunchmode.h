/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ILAUNCHMODE_H
#define KDEVPLATFORM_ILAUNCHMODE_H

#include "interfacesexport.h"

class QString;
class QIcon;

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
    virtual QIcon icon() const = 0;
    
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

