/* This file is part of KDevelop
Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
Copyright (C) 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
Copyright (C) 2003,2006 Hamish Rodda <rodda@kde.org>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
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

#ifndef KDEVCORE_H
#define KDEVCORE_H

#include "kdevexport.h"

class KDevEnvironment;
class KDevProject;
class KDevProjectController;
class KDevLanguageSupport;
class KDevLanguageController;
class KDevPartController;
class KDevDocumentController;
class KDevPluginController;
class KDevMainWindow;
class KDevBackgroundParser;

namespace MainWindowUtils
{
QString beautifyToolTip( const QString& text );
}

/**
 * KDevCore is the container class for all the various objects in use by
 * KDevelop. If access is needed to a particular controller, then this class
 * should be used.
 *
 * KDevCore can provide the user with instances of the following things:
 * <li>the project controller
 * <li>the document controller
 * <li>the mainwindow
 * <li>the plugin controller
 * <li>the part controller
 * <li>the language controller
 * <li>the currently active project
 * <li>the currently active language support
 * <li>the environment as used by processes started by KDevelop
 *
 * When an object is provided to KDevCore so it can be used later, KDevCore
 * will take ownership of the object and upon application shutdown will take
 * responsibility for deleting the objects stored by KDevCore.
 */
class KDEVINTERFACES_EXPORT KDevCore
{
public:
    /** Constructor */
    KDevCore() {};

    /** Destructor.*/
    virtual ~KDevCore() {};

    /**
     * @brief Get the current environment for child processes
     * @return the KDevEnv object that represents the current environment
     * that will be used by processes started by KDevelop as part of an
     * operation requested by the user.
     */
    static KDevEnvironment *environment();

    /**
     * @brief Set the current environment for child processes
     * @param environment the KDevEnv object that represents the current
     * environment that will be used by processes started by KDevelop as
     * part of an operation requested by the user.
     */
    static void setEnvironment( KDevEnvironment *environment );

    /**
     *
     * @return
     */
    static KDevProjectController *projectController();

    /**
     *
     * @param project
     */
    static void setProjectController( KDevProjectController *projectController );

    /**
     *
     * @return
     */
    static KDevProject* activeProject();

    /**
     *
     * @return
     */
    static KDevMainWindow *mainWindow();

    /**
     *
     * @param mainWindow
     */
    static void setMainWindow( KDevMainWindow *mainWindow );

    /**
     *
     * @return
     */
    static KDevPluginController *pluginController();

    /**
     *
     * @param pluginController
     */
    static void setPluginController( KDevPluginController *pluginController );

    /**
     *
     * @return
     */
    static KDevDocumentController *documentController();

    /**
     *
     * @param documentController
     */
    static void setDocumentController( KDevDocumentController *documentController );

    /**
     *
     * @return
     */
    static KDevPartController *partController();

    /**
     *
     * @param partController
     */
    static void setPartController( KDevPartController *partController );

    /**
     * @brief Get the language controller
     * @return the previously stored KDevLanguageController based object
     * @sa setLanguageController
     */
    static KDevLanguageController *languageController();

    /**
     * @brief Set the language controller
     * @param langController The KDevLanguageController based object to set as
     * the language controller
     */
    static void setLanguageController( KDevLanguageController *langController );

    /**
     * @brief Get the active language support
     * @return a KDevLanguageSupport object representing the currently active
     * language support
     */
    static KDevLanguageSupport *activeLanguage();

        /**
     *
     * @return
         */
    static KDevBackgroundParser *backgroundParser();

    /**
     *
     * @param backgroundParser
     */
    static void setBackgroundParser( KDevBackgroundParser *backgroundParser );

    static KDE_DEPRECATED void initialize();
};

#endif
