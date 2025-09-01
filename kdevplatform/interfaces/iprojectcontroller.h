/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECTCONTROLLER_H
#define KDEVPLATFORM_IPROJECTCONTROLLER_H

#include <QObject>
#include <QList>
#include <QUrl>

#include "interfacesexport.h"

#include <memory>

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
    explicit IProjectController( QObject *parent = nullptr );
    ~IProjectController() override;

    Q_INVOKABLE virtual KDevelop::IProject* projectAt( int ) const = 0;
    Q_INVOKABLE virtual int projectCount() const = 0;
    Q_INVOKABLE virtual QList<IProject*> projects() const = 0;

    /**
     * Provides access to the model representing the open projects
     * @returns the model containing the projects and their items
     */
    virtual ProjectModel* projectModel() = 0;

    /**
     * @return the model that keeps track of the VCS statuses of project files
     *         or @c nullptr if the model is currently nonexistent
     */
    [[nodiscard]] virtual std::shared_ptr<ProjectChangesModel> changesModel() const = 0;
    /**
     * @return the model that keeps track of the VCS statuses of project files
     *
     * If the model does not currently exist, a new one is created and returned.
     */
    virtual std::shared_ptr<ProjectChangesModel> makeChangesModel() = 0;

    virtual ProjectBuildSetModel* buildSetModel() = 0;

    /**
     * Find an open project using the name of the project
     * @param name the name of the project to be found
     * @returns the project or null if no project with that name is open
     */
    virtual KDevelop::IProject* findProjectByName( const QString& name ) = 0;

    /**
     * Finding an open project for a given file or folder in the project
     * @param url the url of a file/folder belonging to an open project
     * @returns the first open project containing the url or null if no such
     * project can be found
     */
    virtual IProject* findProjectForUrl( const QUrl& url ) const = 0;

    /**
     * Checks whether the given project name is used already or not. The project
     * controller supports only 1 project with a given name to be open at any time
     * @param name the name of the project to be opened or created
     * @returns whether the name is already used for an open project
     */
    virtual bool isProjectNameUsed( const QString& name ) const = 0;

    virtual QUrl projectsBaseDirectory() const = 0;

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
    virtual QString prettyFilePath(const QUrl& url, FormattingOptions format = FormatHtml) const = 0;
    
    /**
     * Returns a pretty short representation of the given url, considering the currently loaded projects:
     * When the file is part of a currently loaded project, that projects name is shown as prefix instead of the
     * the full file path.
     * @param format formatting used for the string
     */
    virtual QString prettyFileName(const QUrl& url, FormattingOptions format = FormatHtml) const = 0;

    /**
     * @returns whether project files should be parsed or not
     */
    static bool parseAllProjectSources();

public Q_SLOTS:
    /**
     * Tries finding a project-file for the given source-url and opens it.
     * If no .kdev4 project file is found, the user is asked to import a project.
     */
        virtual void openProjectForUrl( const QUrl &sourceUrl ) = 0;
    /**
     * open the project from the given kdev4 project file. This only reads
     * the file and starts creating the project model from it. The opening process
     * is finished once @ref projectOpened signal is emitted.
     * @param url a kdev4 project file top open
     */
    virtual void openProject( const QUrl & url = QUrl() ) = 0;
    /**
     * close the given project. Closing the project is done in steps and
     * the @ref projectClosing and @ref projectClosed signals are emitted. Only when
     * the latter signal is emitted it is guaranteed that the project has been closed.
     * The @ref IProject object will be deleted after the closing has finished.
     */
    virtual void closeProject( IProject* ) = 0;

    /**
     * Close all projects
     *
     * This usually calls closeProject() on all controlled projects
     * @sa closeProject()
     */
    virtual void closeAllProjects() = 0;

    virtual void configureProject( IProject* ) = 0;

    /**
     * Schedules all files of the @p project for reparsing by @see BackgroundParser
     * The @p forceAll argument is for triggering a full reparse of the entire project
     * after the initial import.
     */
    virtual void reparseProject( IProject* project, bool forceUpdate = false, bool forceAll = false ) = 0;

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
     * @p project contents may not be initialized properly.
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

