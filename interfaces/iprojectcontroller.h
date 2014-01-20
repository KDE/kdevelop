/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_IPROJECTCONTROLLER_H
#define KDEVPLATFORM_IPROJECTCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <kurl.h>
#include "interfacesexport.h"

class QItemSelectionModel;

namespace KDevelop
{

class IProject;
class ProjectBuildSetModel;
class ProjectModel;
class ProjectBaseItem;
class ProjectChangesModel;

/**
 * @class IProjectController
 */
class KDEVPLATFORMINTERFACES_EXPORT IProjectController : public QObject
{
    Q_OBJECT
public:
    IProjectController( QObject *parent = 0 );
    virtual ~IProjectController();

    Q_SCRIPTABLE virtual IProject* projectAt( int ) const = 0;
    Q_SCRIPTABLE virtual int projectCount() const = 0;
    Q_SCRIPTABLE virtual QList<IProject*> projects() const = 0;

    /**
     * Provides access to the model representing the open projects
     * @returns the model containing the projects and their items
     */
    Q_SCRIPTABLE virtual ProjectModel* projectModel() = 0;
    
    
    /**
     * @returns an instance to the model that keeps track of the state 
     * of the files per project.
     */
    Q_SCRIPTABLE virtual ProjectChangesModel* changesModel() = 0;

    Q_SCRIPTABLE virtual ProjectBuildSetModel* buildSetModel() = 0;

    /**
     * Find an open project using the name of the project
     * @param name the name of the project to be found
     * @returns the project or null if no project with that name is open
     */
    Q_SCRIPTABLE virtual KDevelop::IProject* findProjectByName( const QString& name ) = 0;

    /**
     * Finding an open project for a given file or folder in the project
     * @param url the url of a file/folder belonging to an open project
     * @returns the first open project containing the url or null if no such
     * project can be found
     */
    Q_SCRIPTABLE virtual IProject* findProjectForUrl( const KUrl& url ) const = 0;

    /**
     * Checks whether the given project name is used already or not. The project
     * controller supports only 1 project with a given name to be open at any time
     * @param name the name of the project to be opened or created
     * @returns whether the name is already used for an open project
     */
    Q_SCRIPTABLE virtual bool isProjectNameUsed( const QString& name ) const = 0;

    virtual KUrl projectsBaseDirectory() const = 0;

    enum FormattingOptions {
        FormatHtml,
        FormatPlain
    };

    /**
     * Returns a pretty short representation of the base path of the url, considering the currently loaded projects:
     * When the file is part of a currently loaded project, that projects name is shown as prefix instead of the
     * the full file path.
     * The returned path always has a training slash.
     * @param format formatting used for the string
     */
    Q_SCRIPTABLE virtual QString prettyFilePath(const KUrl& url, FormattingOptions format = FormatHtml) const = 0;
    
    /**
     * Returns a pretty short representation of the given url, considering the currently loaded projects:
     * When the file is part of a currently loaded project, that projects name is shown as prefix instead of the
     * the full file path.
     * @param format formatting used for the string
     */
    Q_SCRIPTABLE virtual QString prettyFileName(const KUrl& url, FormattingOptions format = FormatHtml) const = 0;

    /**
     * @returns whether project files should be parsed or not
     */
    static bool parseAllProjectSources();

public Q_SLOTS:
    /**
     * Tries finding a project-file for the given source-url and opens it.
     * If no .kdev4 project file is found, the user is asked to import a project.
     */
        virtual void openProjectForUrl( const KUrl &sourceUrl ) = 0;
    /**
     * open the project from the given kdev4 project file. This only reads
     * the file and starts creating the project model from it. The opening process
     * is finished once @ref projectOpened signal is emitted.
     * @param url a kdev4 project file top open
     * @returns true if the given project could be opened, false otherwise
     */
    virtual void openProject( const KUrl & url = KUrl() ) = 0;
    /**
     * close the given project. Closing the project is done in steps and
     * the @ref projectClosing and @ref projectClosed signals are emitted. Only when
     * the latter signal is emitted it is guaranteed that the project has been closed.
     * The @ref IProject object will be deleted after the closing has finished.
     * @returns true if the project could be closed, false otherwise
     */
    virtual void closeProject( IProject* ) = 0;
    virtual void configureProject( IProject* ) = 0;
//     virtual void changeCurrentProject( KDevelop::ProjectBaseItem* ) = 0;

Q_SIGNALS:
    /**
     * Emitted right before a project is started to be loaded.
     *
     * At this point all sanity checks have been done, so the project
     * is really going to be loaded. Will be followed by @ref projectOpened signal
     * when loading completes or by @ref projectOpeningAborted if there are errors during loading
     * or it is aborted.
     *
     * @param project the project that is about to be opened.
     */
    void projectAboutToBeOpened( KDevelop::IProject* project );
    /**
     * emitted after a project is completely opened and the project model
     * has been populated.
     * @param project the project that has been opened.
     */
    void projectOpened( KDevelop::IProject* project );
    /**
     * emitted when starting to close a project that has been completely loaded before
     * (the @ref projectOpened signal has been emitted).
     * @param project the project that is going to be closed.
     */
    void projectClosing( KDevelop::IProject* project );
    /**
     * emitted when a project has been closed completely.
     * The project object is still valid, the deletion will be done
     * delayed during the next run of the event loop.
     * @param project the project that has been closed.
     */
    void projectClosed( KDevelop::IProject* project );
    /**
     * emitted when a project could not be loaded correctly or loading was aborted.
     * @ref project contents may not be initialized properly.
     * @param project the project which loading has been aborted.
     */
    void projectOpeningAborted( KDevelop::IProject* project );

    /**
     * emitted whenever the project configuration dialog accepted 
     * changes
     * @param project the project whose configuration has changed
     */
    void projectConfigurationChanged( KDevelop::IProject* project );
};

}
#endif

