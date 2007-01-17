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

namespace Koncrete
{

class Environment;
class Project;
class ProjectController;
class LanguageSupport;
class LanguageController;
class PartController;
class DocumentController;
class PluginController;
class MainWindow;
class BackgroundParser;


/**
 * KDevCore is the container class for all the various objects in use by
 * KDevelop. If access is needed to a particular controller, then this class
 * should be used.
 *
 * Core can provide the user with instances of the following things:
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
 * When an object is provided to Core so it can be used later, Core
 * will take ownership of the object and upon application shutdown will take
 * responsibility for deleting the objects stored by Core.
 */
class KDEVPLATFORM_EXPORT Core
{
public:
    /** Constructor */
    Core() {};

    /** Destructor.*/
    virtual ~Core() {};

    /**
     * @brief Get the current environment for child processes
     * @return the KDevEnv object that represents the current environment
     * that will be used by processes started by KDevelop as part of an
     * operation requested by the user.
     */
    static Environment *environment();

    /**
     * @brief Set the current environment for child processes
     * @param environment the KDevEnv object that represents the current
     * environment that will be used by processes started by KDevelop as
     * part of an operation requested by the user.
     */
    static void setEnvironment( Environment *environment );

    /**
     *
     * @return
     */
    static ProjectController *projectController();

    /**
     *
     * @param project
     */
    static void setProjectController( ProjectController *projectController );

    /**
     * @brief Get the active project
     * @return a Project object representing the currently active project
     */
    static Project* activeProject();

    /**
     *
     * @return
     */
    static MainWindow *mainWindow();

    /**
     *
     * @param mainWindow
     */
    static void setMainWindow( MainWindow *mainWindow );

    /**
     *
     * @return
     */
    static DocumentController *documentController();

    /**
     *
     * @param documentController
     */
    static void setDocumentController( DocumentController *documentController );

    /**
     *
     * @return
     */
    static PartController *partController();

    /**
     *
     * @param partController
     */
    static void setPartController( PartController *partController );

    /**
     * @brief Get the language controller
     * @return the previously stored LanguageController based object
     * @sa setLanguageController
     */
    static LanguageController *languageController();

    /**
     * @brief Set the language controller
     * @param langController The LanguageController based object to set as
     * the language controller
     */
    static void setLanguageController( LanguageController *langController );

    /**
     * @brief Get the active language support
     * @return a LanguageSupport object representing the currently active
     * language support
     */
    static LanguageSupport *activeLanguage();

    /**
     * @brief Get the background parser
     * @return a BackgroundParser object which controls the parser jobs
     */
    static BackgroundParser *backgroundParser();

    /**
     * @brief Set the background parser
     * @param backgroundParser
     */
    static void setBackgroundParser( BackgroundParser *backgroundParser );

    /**
     * @brief Load the settings for the Core objects
     */
    static void loadSettings();

    /**
     * @brief Save the settings of the Core objects
     */
    static void saveSettings();

    /**
     * @brief Initialize the Core objects
     */
    static void initialize();

   /**
     * @brief Cleanup the Core objects
    */
    static void cleanup();
};


/**
 * CoreInterface is a virtual base class for all objects which are provided by Core.
 * These objects need to inherit the various pure virtual functions which facilitate things like
 * project loading and inter-object initialization.
 */
class KDEVPLATFORM_EXPORT CoreInterface
{
public:
   /**
    * @brief Load the settings for this Core object
    */
    void load();

   /**
    * @brief Save the settings of this Core object
    */
    void save();

protected:
    /** Constructor */
    CoreInterface(){}

    /** Destructor */
    virtual ~CoreInterface(){}

   /**
    * @brief Load the settings for this Core object
    */
    virtual void loadSettings( bool projectIsLoaded ) = 0;

   /**
    * @brief Save the settings of this Core object
    */
    virtual void saveSettings( bool projectIsLoaded ) = 0;

   /**
    * @brief Initialize this Core object
    */
    virtual void initialize() = 0;

   /**
    * @brief Cleanup this Core object
    */
    virtual void cleanup() = 0;
};

}
#endif
