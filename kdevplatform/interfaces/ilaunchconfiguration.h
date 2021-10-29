/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

