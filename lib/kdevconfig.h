/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#ifndef KDEVCONFIG_H
#define KDEVCONFIG_H

#include <QObject>
#include <kconfig.h>

#include "kdevexport.h"

/**
The interface to KDevelop's config objects.
Developers using the KDevelop API should use these config objects instead of
the standard KGlobal::config object.  Again, DO NOT USE KGlobal::config() as
it can cause unexpected syncing issues.
*/
class KDEVINTERFACES_EXPORT KDevConfig: public QObject
{
    Q_OBJECT
public:
    KDevConfig( QObject *parent = 0 );
    virtual ~KDevConfig();

    /**
     * @return A pointer to the standard config object.  This object will point
     * to different config files depending on whether a project is opened.
     * If a project IS NOT opened then the config object will consist of the
     * settings found in $KDEINSTALL/config/kdeveloprc and $USER/.kde/config/kdeveloprc.
     * THE MOST SPECIFIC FILE WILL BE $USER/.kde/config/kdeveloprc.
     *
     * If a project IS opened then it will additionally consist of the settings
     * found in the global project file and the local project file.  THE MOST
     * SPECIFIC FILE WILL BE THE GLOBAL PROJECT FILE.
     */
    static KConfig *standard();

    /**
     * @return A pointer to the local project config object.  This object will point
     * to different config files depending on whether a project is opened.
     * If a project IS NOT opened then the config object will consist of the
     * settings found in $KDEINSTALL/config/kdeveloprc and $USER/.kde/config/kdeveloprc.
     * THE MOST SPECIFIC FILE WILL BE $USER/.kde/config/kdeveloprc.
     *
     * If a project IS opened then it will additionally consist of the settings
     * found in the global project file and the local project file.  THE MOST
     * SPECIFIC FILE WILL BE THE LOCAL PROJECT FILE.
     */
    static KConfig *localProject();

    /**
     * @return A pointer to the global project config object.  This object will point
     * to different config files depending on whether a project is opened.
     * If a project IS NOT opened then the config object will consist of the
     * settings found in $KDEINSTALL/config/kdeveloprc and $USER/.kde/config/kdeveloprc.
     * THE MOST SPECIFIC FILE WILL BE $USER/.kde/config/kdeveloprc.
     *
     * If a project IS opened then it will additionally consist of the settings
     * found in the global project file and the local project file.  THE MOST
     * SPECIFIC FILE WILL BE THE LOCAL GLOBAL PROJECT FILE.
     *
     * This function should RARELY be used as it is operationally the same as standard()
     */
    static KConfig *globalProject();

    /**
     * @return A shared pointer to the standard config object.
     */
    static KSharedConfig::Ptr sharedStandard();
    /**
     * @return A shared pointer to the local project config object.
     */
    static KSharedConfig::Ptr sharedLocalProject();
    /**
     * @return A shared pointer to the global project config object.
     */
    static KSharedConfig::Ptr sharedGlobalProject();
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
