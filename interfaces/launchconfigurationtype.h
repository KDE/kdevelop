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

#ifndef LAUNCHCONFIGURATIONTYPE_H
#define LAUNCHCONFIGURATIONTYPE_H

#include "interfacesexport.h"

#include <QtCore/QList>

class KIcon;

namespace KDevelop
{

class ILauncher;
class LaunchConfigurationPageFactory;

/**
 * Launch configuration types are used to be able to create
 * new launch configurations. Each launch configuration has a 
 * specific type, which specifies which launchers can be used
 * for the configuration as well as which config pages are needed
 * to setup the launch configuration
 */
class KDEVPLATFORMINTERFACES_EXPORT LaunchConfigurationType
{
public:
    LaunchConfigurationType();
    virtual ~LaunchConfigurationType() {}

    /**
     * Provide a unique identifier for the type
     * among other things this will be used to create a config group in launch
     * configurations for the pages of this config type
     * @returns a unique identifier for this launch configuration type
     */
    virtual QString id() const = 0;

    /**
     * Provide a user visible name for the type
     * @returns a translatable string for the type
     */
    virtual QString name() const = 0;
    
    /**
     * Add @p starter to this configuration type
     * @param starter the launcher that can start configurations of this type
     */
    void addLauncher( ILauncher* starter );
    
    /**
     * remove @p starter from this configuration type
     * @param starter the launcher that should not start configurations of this type
     */
    void removeLauncher( ILauncher* starter );
    
    /**
     * Access all launchers that are usable with this type
     * @returns a list of launchers that can be used with configurations of this type
     */
    QList<ILauncher*> launchers() const;
    
    /**
     * Convenience method to access a launcher given its @p id
     * @param id the id of the launcher to be found
     * @returns the launcher with the given id or 0 if there's no such launcher in this configuration type
     */
    ILauncher* launcherForId( const QString& id );
    
    /**
     * Provide a list of widgets to configure a launch configuration for this type
     * @returns a list of factories to create config pages from.
     */
    virtual QList<LaunchConfigurationPageFactory*> configPages() const = 0;
    
    /**
     * Provide an icon for this launch configuration type
     * @returns an icon to be used for representing launch configurations of this type
     */
    virtual KIcon icon() const = 0;
private:
    class LaunchConfigurationTypePrivate* const d;
};

}

#endif

