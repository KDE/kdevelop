/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
class ClassStore;
class KDevPartController;
class KDevMainWindow;
class KDevDebugger;
class KDevDiffFrontend;
class KDevCreateFile;
class KDevSourceFormatter;
class KDevCodeRepository;

class KDevApiPrivate;

/**
* This abstract class provides an interface to KDevelop's core components.
*/
class KDevApi : public QObject
{
    Q_OBJECT
public:
    typedef QMap<QString,KDevVersionControl*> VersionControlMap;

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
    virtual KDevMainWindow *mainWindow() = 0;

    /**
    * Check if the main window is valid
    */
    virtual bool mainWindowValid() = 0;

    /**
    * Returns a reference to the part controller component, which permits
    * access to the current active parts (or components).
    * @see KDevPartController
    */
    virtual KDevPartController *partController() = 0;

    /**
    * Returns a reference to core object which provides basic functionalities
    * for inter-parts communications / cooperation.
    */
    virtual KDevCore *core() = 0;

    /**
    * Returns a reference to encharged class store object.
    */
    virtual ClassStore *classStore() = 0;

    /**
    * Returns a reference to the encharged class store used for code completion
    * feature.
    */
    virtual ClassStore *ccClassStore() = 0;

    /**
    * Returns a reference to the debugger component.
    */
    virtual KDevDebugger *debugger() = 0;

    /**
    * Returns a reference to Document Object Model for the current project, or null
    * if not project loaded.
    */
    QDomDocument *projectDom();
    /**
    * Set the Document Object Model for the current project.
    * @param dom
    */
    void setProjectDom(QDomDocument *dom);

    /**
    * Returns a reference to current project, or null if no project is loaded.
    * @ref KDeveProject
    */
    KDevProject *project();
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
    KDevMakeFrontend *makeFrontend();
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
    KDevAppFrontend *appFrontend();
    /**
    * Set the application front-end to use.
    * @param appFrontend
    */
    void setAppFrontend(KDevAppFrontend *appFrontend);

    /**
    * Returns the module encharged for supporting the language(s) used in the project.
    */
    KDevLanguageSupport *languageSupport();
    /**
    * Set the object charged of providing handling for the source files.
    * @param languageSupport
    */
    void setLanguageSupport(KDevLanguageSupport *languageSupport);

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
    KDevVersionControl *versionControlByName( const QString &uid );

    /**
    * Returns the reference to the current diff frontend, which basically
    * provides a way for pretty-displaying the output of the "diff" or
    * "cvs diff" commands.
    * @see KDevDiffFrontend
    */
    KDevDiffFrontend *diffFrontend();
    /**
    * Set the diff front-end to use (currently only one is provided).
    * @param diffFrontend
    */
    void setDiffFrontend(KDevDiffFrontend *diffFrontend);

    /**
    * The kind of objects returned by this method provides widgets and methods
    * for creating new files based on language features and user definable
    * templates.
    * @see KDevCreateFile
    */
    KDevCreateFile *createFile();
    /**
    * Set the object encharged for creating new files from templates.
    * @param dom
    */
    void setCreateFile(KDevCreateFile *createFile);

    /**
    * KDevSourceFormatter objects provides the ability to apply different
    * standard formatting to the source or let the user to customize his own.
    * @see KDevSourceFormatter
    */
    KDevSourceFormatter *sourceFormatter();
    /**
    * Set the Document Object Model for the current project.
    * @param dom
    */
    void setSourceFormatter(KDevSourceFormatter *sourceFormatter);

    /**
    * KDevSourceRepository objects provides connections to the interested
    * modules so they can know about what happens to code Catalogs
    */
    KDevCodeRepository *codeRepository();

private:
    KDevApiPrivate *d;
};

#endif
