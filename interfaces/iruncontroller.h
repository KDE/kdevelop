/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVPLATFORM_IRUNCONTROLLER_H
#define KDEVPLATFORM_IRUNCONTROLLER_H

#include <QtCore/QObject>

#include <kjobtrackerinterface.h>

#include "interfacesexport.h"

class KJob;

namespace KDevelop
{
class IProject;
class ILaunchMode;
class ILaunchConfiguration;
class LaunchConfigurationType;

/**
 * The main controller for running processes.
 */
class KDEVPLATFORMINTERFACES_EXPORT IRunController : public KJobTrackerInterface
{
    Q_OBJECT
    Q_ENUMS(State)
public:
    ///Constructor.
    IRunController(QObject *parent);

    /**
     * Interrogate the current managed jobs
     */
    Q_SCRIPTABLE virtual QList<KJob*> currentJobs() const = 0;

    /**
     * An enumeration of the possible states for the run controller.
     */
    enum State {
        Idle     /**< No processes are currently running */,
        Running  /**< processes are currently running */
    };

    /**
     * Get a list of all launch modes that the app knows
     * @returns a list of registered launch modes
     */
    virtual QList<ILaunchMode*> launchModes() const = 0;

    /**
     * Get a list of all available launch configurations
     */
    virtual QList<ILaunchConfiguration*> launchConfigurations() const = 0;

    /**
     * Get a specific launch mode based using its \a id
     * @param id the identifier of the launchmode to get
     * @returns launch mode for the given id or 0 if no such mode is known
     */
    virtual ILaunchMode* launchModeForId( const QString& id ) const = 0;
    
    /**
     * add @p mode to the list of registered launch modes
     * @param mode the mode to be registered
     */
    virtual void addLaunchMode( ILaunchMode* mode ) = 0;
    
    /**
     * remove @p mode from the list of registered launch modes
     * @param mode the mode to be unregistered
     */
    virtual void removeLaunchMode( ILaunchMode* mode ) = 0;

    /**
     * Get a list of all configuration types that are registered
     * @returns a list of run configuration types
     */
    virtual QList<LaunchConfigurationType*> launchConfigurationTypes() const = 0;

    
    /**
     * Adds @p type to the list of known run config types
     * @param type the new run configuration type
     */
    virtual void addConfigurationType( LaunchConfigurationType* type ) = 0;

    /**
     * Removes @p type from the list of known run config types
     * @param type run configuration type that should be removed
     */
    virtual void removeConfigurationType( LaunchConfigurationType* type ) = 0;
    
    /**
     * Executes the default launch in the given mode
     * @param runMode the launch mode to start with
     */
    virtual void executeDefaultLaunch( const QString& runMode ) = 0;

    virtual KJob* execute(const QString& launchMode, ILaunchConfiguration* launch) = 0;

    /**
     * tries to find a launch config type for the given @p id
     * @param id the id of the launch configuration type to search
     * @returns the launch configuration type if found, or 0 otherwise
     */
    virtual LaunchConfigurationType* launchConfigurationTypeForId( const QString& id ) = 0;
    
    /**
     * Creates a new launch configuration in the given project or in the session if no project
     * was provided using the given launch configuration type and launcher. The configuration 
     * is also added to the list of configurations in the runcontroller.
     * 
     * @param type the launch configuration type to be used for the new config
     * @param launcher the mode and id of the launcher to be used in the config
     * @param project the project in which the launch configuration should be stored
     * @param name the name of the new launch configuration, if this is empty a new default name will be generated
     * @returns a new launch configuration
     */
    virtual ILaunchConfiguration* createLaunchConfiguration( LaunchConfigurationType* type,
                                                             const QPair<QString,QString>& launcher,
                                                             KDevelop::IProject* project = 0,
                                                             const QString& name = QString() ) = 0;

public Q_SLOTS:
    /**
     * Request for all running processes to be killed.
     */
    virtual void stopAllProcesses() = 0;

Q_SIGNALS:
    /**
     * Notify that the state of the run controller has changed to \a {state}.
     */
    void runStateChanged(State state);

    /**
     * Notify that a new job has been registered.
     */
    void jobRegistered(KJob* job);

    /**
     * Notify that a job has been unregistered.
     */
    void jobUnregistered(KJob* job);
};

}

#endif // KDEVPLATFORM_IRUNCONTROLLER_H
