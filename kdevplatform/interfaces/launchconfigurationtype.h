/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LAUNCHCONFIGURATIONTYPE_H
#define KDEVPLATFORM_LAUNCHCONFIGURATIONTYPE_H

#include "interfacesexport.h"

#include <QObject>

class QMenu;
class QIcon;
class QUrl;
#include <QStringList>
class KConfigGroup;

namespace KDevelop
{
class IProject;
class ILaunchConfiguration;
class ProjectBaseItem;
class ILauncher;
class LaunchConfigurationPageFactory;
class LaunchConfigurationTypePrivate;

/**
 * Launch configuration types are used to be able to create
 * new launch configurations. Each launch configuration has a
 * specific type, which specifies which launchers can be used
 * for the configuration as well as which config pages are needed
 * to setup the launch configuration
 */
class KDEVPLATFORMINTERFACES_EXPORT LaunchConfigurationType : public QObject
{
Q_OBJECT
public:
    LaunchConfigurationType();
    ~LaunchConfigurationType() override;

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
    ILauncher* launcherForId(const QString& id) const;

    /**
     * Provide a list of widgets to configure a launch configuration for this type
     * @returns a list of factories to create config pages from.
     */
    virtual QList<LaunchConfigurationPageFactory*> configPages() const = 0;

    /**
     * Provide an icon for this launch configuration type
     * @returns an icon to be used for representing launch configurations of this type
     */
    virtual QIcon icon() const = 0;

    /**
     * Check whether this launch configuration type can launch the given project item
     * @param item the project tree item to test
     * @returns true if this configuration type can launch the given item, false otherwise
     */
    virtual bool canLaunch( KDevelop::ProjectBaseItem* item ) const = 0;

    /**
     * Configure the given launch configuration to execute the selected item
     * @param config the configuration to setup
     * @param item the item to launch
     */
    virtual void configureLaunchFromItem( KConfigGroup config,
                                          KDevelop::ProjectBaseItem* item ) const = 0;

    /**
     * Configure the given launch configuration to execute the selected item
     * @param config the configuration to setup
     * @param args argument list
     */
    virtual void configureLaunchFromCmdLineArguments( KConfigGroup config,
                                                      const QStringList &args ) const = 0;

    /**
    * Check whether this launch configuration type can launch the given file
    * @param file the file to test launchability
    * @returns true if this configuration type can launch the given file, false otherwise
    */
    virtual bool canLaunch( const QUrl& file ) const = 0;

    /**
     * Creates and returns a menu that will be added to the UI where the interface will be
     * able to add any suggestion it needs, like default targets.
     * Can be a nullptr if there are no suggestions.
     * Caller takes ownership of the returned menu.
     */
    virtual QMenu* launcherSuggestions() { return nullptr; }

Q_SIGNALS:
    void signalAddLaunchConfiguration(KDevelop::ILaunchConfiguration* launch);

private:
    const QScopedPointer<class LaunchConfigurationTypePrivate> d_ptr;
    Q_DECLARE_PRIVATE(LaunchConfigurationType)
};

}

#endif

