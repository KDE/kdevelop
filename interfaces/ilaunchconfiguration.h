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

#ifndef KDEVPLATFORM_ILAUNCHCONFIGURATION_H
#define KDEVPLATFORM_ILAUNCHCONFIGURATION_H

#include "interfacesexport.h"

class KConfigGroup;
class QString;

namespace KDevelop
{

class LaunchConfigurationType;
class IProject;

/**
 * A specific configuration to start a launchable, this could be a native
 * compiled application, or some script file or byte-compiled file or something else
 * Provides access to the various configurations, as well as their type and name
 */
class KDEVPLATFORMINTERFACES_EXPORT ILaunchConfiguration
{
public:
    virtual ~ILaunchConfiguration();

    /**
     * Access to the configuration group for this launch configuration.
     * @returns the config to read values from
     */
    virtual const KConfigGroup config() const = 0;
    
    /**
     * Access to the configuration group for this launch configuration.
     * @returns the config to write to
     */
    virtual KConfigGroup config() = 0;
    
    /**
     * Each launch configuration has a type, which identifies which launcher and launch modes
     * are supported.
     * @returns the configuration type
     */
    virtual LaunchConfigurationType* type() const = 0;
    
    /**
     * The user-chosen name for the launch configuration.
     * @returns the config name.
     */
    virtual QString name() const = 0;
    
    /**
     * Provides access to the project where the launch config is stored.
     * @returns the project in which the configuration is stored or 0 if the configuration is stored globally
     */
    virtual IProject* project() const = 0;
};

}

#endif

