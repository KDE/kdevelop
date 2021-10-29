/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LAUNCHCONFIGURATION_H
#define KDEVPLATFORM_LAUNCHCONFIGURATION_H

#include <interfaces/ilaunchconfiguration.h>

#include "shellexport.h"

#include <KConfigGroup>

class QString;

namespace KDevelop
{

class LaunchConfigurationType;
class IProject;
class LaunchConfigurationPrivate;

/**
 * @copydoc KDevelop::ILaunchConfiguration
 */
class KDEVPLATFORMSHELL_EXPORT LaunchConfiguration : public QObject, public ILaunchConfiguration
{
Q_OBJECT
public:
    explicit LaunchConfiguration( const KConfigGroup&, IProject* = nullptr, QObject* = nullptr );
    ~LaunchConfiguration() override;

    static QString LaunchConfigurationNameEntry();
    static QString LaunchConfigurationTypeEntry();

    /**
     * Change the name of this launch configuration
     * @param name the new name for the launch configuration
     */
    void setName( const QString& name );
    
    /**
     * Changes the type of this launch configuration. Note that
     * this removes all existing config values from this configuration
     * @param typeId the id of the new type
     */
    void setType( const QString& typeId );

    /**
     * @copydoc KDevelop::ILaunchConfiguration::config()
     */
    const KConfigGroup config() const override;
    
    /**
     * @copydoc KDevelop::ILaunchConfiguration::type()
     */
    LaunchConfigurationType* type() const override;
    
    /**
     * @copydoc KDevelop::ILaunchConfiguration::name()
     */
    QString name() const override;
    
    /**
     * @copydoc KDevelop::ILaunchConfiguration::project()
     */
    IProject* project() const override;
    void save();
    
    QString configGroupName() const;
    
    QString launcherForMode( const QString& mode ) const;
    void setLauncherForMode( const QString& mode, const QString& id );
    
    KConfigGroup config() override;
    
Q_SIGNALS:
    void nameChanged( LaunchConfiguration* );
    void typeChanged( LaunchConfigurationType* );

private:
    const QScopedPointer<class LaunchConfigurationPrivate> d_ptr;
    Q_DECLARE_PRIVATE(LaunchConfiguration)
};

}

#endif

