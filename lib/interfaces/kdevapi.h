/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KDEVAPI_H_
#define _KDEVAPI_H_

#include <qobject.h>

class QStringList;
class QDomDocument;
class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevMakeFrontend;
class KDevAppFrontend;
class CodeModel;
class KDevPartController;
class KDevMainWindow;
class KDevDebugger;
class KDevDiffFrontend;
class KDevCreateFile;
class KDevCodeRepository;
class KDevPlugin;

class KDevApiPrivate;

/**
* This abstract class provides an interface to KDevelop's core components.
*/
class KDevApi : public QObject
{
    Q_OBJECT
public:
    /**
    * Constructor
    */
    KDevApi();

    /**
    * Destructor
    */
    virtual ~KDevApi();

    /**
    * Returns a reference to the main window.
    * @see KDevMainWindow
    */
    virtual KDevMainWindow *mainWindow() const = 0;

    /**
    * Check if the main window is valid
    */
    virtual bool mainWindowValid() const = 0;

    /**
    * Returns a reference to the part controller component, which permits
    * access to the current active parts (or components).
    * @see KDevPartController
    */
    virtual KDevPartController *partController() const = 0;

    /**
    * Returns a reference to core object which provides basic functionalities
    * for inter-parts communications / cooperation.
    */
    virtual KDevCore *core() const = 0;

    /**
    * Returns a reference to encharged class store object.
    */
    virtual CodeModel *codeModel() const = 0;

    /**
    * Returns a reference to the debugger component.
    */
    virtual KDevDebugger *debugger() const = 0;

    /**
    * Returns a reference to Document Object Model for the current project, or null
    * if not project loaded.
    */
    QDomDocument *projectDom() const;
    /**
    * Set the Document Object Model for the current project.
    * @param dom
    */
    void setProjectDom(QDomDocument *dom);

    /**
    * Returns a reference to current project, or null if no project is loaded.
    * @ref KDevProject
    */
    KDevProject *project() const;
    /**
    * Set the current project.
    * @param project
    */
    void setProject(KDevProject *project);

    /**
    * Returns a reference to current make front-end, which runs build commands
    * and display output messages in its widget; null if none is found.
    * @ref KDevMakeFrontend
    */
    KDevMakeFrontend *makeFrontend() const;
    /**
    * Set the make front-end to use.
    * @param makeFrontend
    */
    void setMakeFrontend(KDevMakeFrontend *makeFrontend);

    /**
    * Returns a reference to current application front-end, which runs displays
    * running application's output messages in its widget; null if none is found.
    * @ref KDevMakeFrontend
    */
    KDevAppFrontend *appFrontend() const;
    /**
    * Set the application front-end to use.
    * @param appFrontend
    */
    void setAppFrontend(KDevAppFrontend *appFrontend);

    /**
    * Returns the module encharged for supporting the language(s) used in the project.
    */
    KDevLanguageSupport *languageSupport() const;
    /**
    * Set the object charged of providing handling for the source files.
    * @param languageSupport
    */
    void setLanguageSupport(KDevLanguageSupport *languageSupport);

    /**
    * Returns a reference to the version control used.
    * @return
    */
    KDevVersionControl *versionControl() const;
    /**
    * Set the default version control.
    * @param vcs
    */
    void setVersionControl( KDevVersionControl *vcs );

    /**
    * Dinamically add a new Version Control plug-in to the IDE (several may
    * be running in the same project).
    * @param vcs new version control object
    */
    void registerVersionControl( KDevVersionControl *vcs );
    /**
    * Dinamically deletes the version control plug-in from the list of the
    * available ones. The plug-in will be unloaded.
    * @param vcs version control object to delete
    */
    void unregisterVersionControl( KDevVersionControl *vcs );
    /**
    * Returns a @see QStringList containing unique (among the list) identifiers
    * of the registered version control systems (currently their names).
    */
    QStringList registeredVersionControls() const;
    /**
    * Returns a reference to the version control identified by its id; will
    * return null if the specified VCS is not present.
    * @param  uid unique identifier of the VCS to unload, as returned by
    *         @see KDevVersionControl::uid().
    */
    KDevVersionControl *versionControlByName( const QString &uid ) const;

    /**
    * Returns the reference to the current diff frontend, which basically
    * provides a way for pretty-displaying the output of the "diff" or
    * "cvs diff" commands.
    * @see KDevDiffFrontend
    */
    KDevDiffFrontend *diffFrontend() const;
    /**
    * Set the diff front-end to use (currently only one is provided).
    * @param diffFrontend
    */
    void setDiffFrontend( KDevDiffFrontend *diffFrontend );

    /**
    * The kind of objects returned by this method provides widgets and methods
    * for creating new files based on language features and user definable
    * templates.
    * @see KDevCreateFile
    */
    KDevCreateFile *createFile() const;
    /**
    * Set the object encharged for creating new files from templates.
    * @param createFile
    */
    void setCreateFile( KDevCreateFile *createFile );

    /**
    * Queries for the plugin which supports given service type.
    * All already loaded plugins will be queried and the first one to support the service type
    * will be returned. Any plugin can be an extension, only "ServiceTypes=..." entry is
    * required in .desktop file for that plugin.
    * @param serviceType a service type of an extension (like "KDevelop/SourceFormatter")
    * @return a KDevelop extension plugin for given service type or 0 if no plugin supports it
    */
    virtual KDevPlugin *queryForExtension(const QString &serviceType) = 0;

    /**
    * KDevSourceRepository objects provides connections to the interested
    * modules so they can know about what happens to code Catalogs
    */
    KDevCodeRepository *codeRepository() const;
    
private:
    KDevApiPrivate *d;
};

#endif
