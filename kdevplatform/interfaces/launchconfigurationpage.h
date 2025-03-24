/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LAUNCHCONFIGURATIONPAGE_H
#define KDEVPLATFORM_LAUNCHCONFIGURATIONPAGE_H

#include "interfacesexport.h"

#include <QWidget>

class KConfigGroup;

namespace KDevelop
{

class IProject;
    
/**
 * Provides a configuration page for a launch configuration, the interface
 * allows the actual dialog to easily load/save the configuration and show some title/icon
 */
class KDEVPLATFORMINTERFACES_EXPORT LaunchConfigurationPage : public QWidget
{
Q_OBJECT
public:
    explicit LaunchConfigurationPage( QWidget* parent );
    /**
     * Allows the page to load values from an existing launch configuration
     * @param cfg the configuration to load from
     */
    virtual void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = nullptr ) = 0;
    
    /**
     * Allows the page to save values to an existing launch configuration
     * @param cfg the configuration to save to
     */
    virtual void saveToConfiguration( KConfigGroup cfg, KDevelop::IProject* project = nullptr ) const = 0;
    
    /**
     * A title for displaying in the GUI
     * @returns the title of the page
     */
    virtual QString title() const = 0;
    
    /**
     * an icon for the GUI
     * @returns an icon suitable for display in the GUI
     */
    virtual QIcon icon() const = 0;
Q_SIGNALS:
    void changed();
};

/**
 * A simple factory class to create certain launch config pages
 * this is used to create config pages only when they're needed
 */
class KDEVPLATFORMINTERFACES_EXPORT LaunchConfigurationPageFactory
{
public:
    LaunchConfigurationPageFactory() = default;
    virtual ~LaunchConfigurationPageFactory() {}
    Q_DISABLE_COPY_MOVE(LaunchConfigurationPageFactory)

    /**
     * create a new launch config page widget using the given @p parent
     * @param parent the parent widget to be used
     * @returns a new launch config page
     */
    virtual LaunchConfigurationPage* createWidget( QWidget* parent ) = 0;
};

}

#endif

