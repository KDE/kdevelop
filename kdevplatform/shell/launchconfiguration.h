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

