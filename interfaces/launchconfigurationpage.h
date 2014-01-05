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

#ifndef KDEVPLATFORM_LAUNCHCONFIGURATIONPAGE_H
#define KDEVPLATFORM_LAUNCHCONFIGURATIONPAGE_H

#include "interfacesexport.h"

#include <QWidget>

class KConfigGroup;
class KIcon;

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
    LaunchConfigurationPage( QWidget* parent );
    /**
     * Allows the page to load values from an existing launch configuration
     * @param cfg the configuration to load from
     */
    virtual void loadFromConfiguration( const KConfigGroup& cfg, KDevelop::IProject* project = 0 ) = 0;
    
    /**
     * Allows the page to save values to an existing launch configuration
     * @param cfg the configuration to save to
     */
    virtual void saveToConfiguration( KConfigGroup, KDevelop::IProject* project = 0 ) const = 0;
    
    /**
     * A title for displaying in the GUI
     * @returns the title of the page
     */
    virtual QString title() const = 0;
    
    /**
     * an icon for the GUI
     * @returns an icon suitable for display in the GUI
     */
    virtual KIcon icon() const = 0;
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
    virtual ~LaunchConfigurationPageFactory() {}
    /**
     * create a new launch config page widget using the given @p parent
     * @param parent the parent widget to be used
     * @returns a new launch config page
     */
    virtual LaunchConfigurationPage* createWidget( QWidget* parent ) = 0;
};

}

#endif

