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

#ifndef KDEVPLATFORM_ILAUNCHER_H
#define KDEVPLATFORM_ILAUNCHER_H

#include "interfacesexport.h"

#include <QtCore/QList>

#include "iruncontroller.h"

class KJob;
class QString;

namespace KDevelop
{

class ILaunchConfiguration;
class LaunchConfigurationPageFactory;

/**
 * The ILauncher interface allows to actually run launch configurations.
 * Additionally it allows to provide configuration pages specific to the launcher
 * and to identify for which modes this launcher may be used
 */
class KDEVPLATFORMINTERFACES_EXPORT ILauncher
{
public:
    virtual ~ILauncher();

    /**
     * Returns a unique identifier for the launcher,
     * used for example for ILaunchConfigurationType::launcherForId().
     */
    virtual QString id() = 0;

    /**
        * a user visible name for the launcher
        * @returns a translatable string as description for the launcher
        */
    virtual QString name() const = 0;
    
    /**
        * provides a short description about this launcher
        * @returns a description of what the launcher can do
        */
    virtual QString description() const = 0;
    
    /**
        * returns the ids of the supported launch modes
        * @returns a list of id's for ILaunchMode's that are supported
        */
    virtual QStringList supportedModes() const = 0;
    
    /**
        * provide a list of config pages for this launcher
        * @returns the config pages that this launcher needs
        */
    virtual QList<LaunchConfigurationPageFactory*> configPages() const = 0;
    
    /**
        * create a KJob that executes @p cfg in @p launchMode when started.
        * @param launchMode the mode in which the launcher should execute the configuration
        * @param cfg the launch configuration to be executed
        * @returns a KJob that starts the configured launchable
        */
    virtual KJob* start( const QString& launchMode, ILaunchConfiguration* cfg ) = 0;
};

}

#endif

