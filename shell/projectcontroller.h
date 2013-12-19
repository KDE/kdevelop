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

#ifndef KDEVPLATFORM_PROJECTCONTROLLER_H
#define KDEVPLATFORM_PROJECTCONTROLLER_H

#include <interfaces/iprojectcontroller.h>
#include <kio/udsentry.h>

#include "shellexport.h"

namespace Sublime {
    class Area;
}

namespace KIO {
    class Job;
}

namespace KDevelop
{

class IProject;
class Core;
class Context;
class ContextMenuExtension;
class IPlugin;

class KDEVPLATFORMSHELL_EXPORT IProjectDialogProvider : public QObject
{
Q_OBJECT
public:
    IProjectDialogProvider();
    virtual ~IProjectDialogProvider();

public Q_SLOTS:
    /**
     * Displays some UI to ask the user for the project location.
     * 
     * @param fetch will tell the UI that the user might want to fetch the project first
     * @param startUrl tells where to look first
     */
    virtual KUrl askProjectConfigLocation(bool fetch, const KUrl& startUrl = KUrl()) = 0;
    virtual bool userWantsReopen() = 0;
};

class KDEVPLATFORMSHELL_EXPORT ProjectController : public IProjectController
{
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kdevelop.ProjectController" )
    friend class Core;
    friend class CorePrivate;
    friend class ProjectPreferences;

public:
    ProjectController( Core* core );
    virtual ~ProjectController();

    virtual IProject* projectAt( int ) const;
    virtual int projectCount() const;
    virtual QList<IProject*> projects() const;

    virtual ProjectBuildSetModel* buildSetModel();
    virtual ProjectModel* projectModel();
    virtual ProjectChangesModel* changesModel();
    virtual QItemSelectionModel* projectSelectionModel();
    virtual IProject* findProjectByName( const QString& name );
    IProject* findProjectForUrl( const KUrl& ) const;
    void addProject(IProject*);
//     IProject* currentProject() const;

    virtual bool isProjectNameUsed( const QString& name ) const;
    void setDialogProvider(IProjectDialogProvider*);

    KUrl projectsBaseDirectory() const;
    QString prettyFileName(KUrl url, FormattingOptions format = FormatHtml) const;
    QString prettyFilePath(KUrl url, FormattingOptions format = FormatHtml) const;

    ContextMenuExtension contextMenuExtension( KDevelop::Context* ctx );

public Q_SLOTS:
    virtual void openProjectForUrl( const KUrl &sourceUrl );
    virtual void fetchProject();
    virtual void openProject( const KUrl &KDev4ProjectFile = KUrl() );
    virtual void abortOpeningProject( IProject* );
    void projectImportingFinished( IProject* );
    virtual void closeProject( IProject* );
    virtual void configureProject( IProject* );
    void eventuallyOpenProjectFile(KIO::Job*,KIO::UDSEntryList);
    void openProjectForUrlSlot(bool);
//     void changeCurrentProject( ProjectBaseItem* );
    void openProjects(const KUrl::List& projects);
    void commitCurrentProject();

    // Maps the given path from the source to the equivalent path within the build directory
    // of the corresponding project. If the path is already in the build directory and fallbackRoot is true,
    // then it is mapped to the root of the build directory.
    // If reverse is true, maps the opposite direction, from build to source. [ Used in kdevplatform_shell_environment.sh ]
    Q_SCRIPTABLE QString mapSourceBuild( const QString& path, bool reverse = false, bool fallbackRoot = true ) const;

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );

    virtual void initialize();

private:
    //FIXME Do not load all of this just for the project being opened...
    //void legacyLoading();
    void setupActions();
    void cleanup();

    // helper methods for closeProject()
    void unloadUnusedProjectPlugins(IProject* proj);
    void disableProjectCloseAction();
    void closeAllOpenedFiles(IProject* proj);
    void initializePluginCleanup(IProject* proj);

private:
    Q_PRIVATE_SLOT(d, void projectConfig( QObject* ) )
    Q_PRIVATE_SLOT(d, void unloadAllProjectPlugins() )
    Q_PRIVATE_SLOT(d, void notifyProjectConfigurationChanged() )
    Q_PRIVATE_SLOT(d, void updateActionStates( KDevelop::Context* ) )
    Q_PRIVATE_SLOT(d, void closeSelectedProjects() )
    Q_PRIVATE_SLOT(d, void openProjectConfig() )
    Q_PRIVATE_SLOT(d, void areaChanged(Sublime::Area*) )
    class ProjectControllerPrivate* const d;
    friend class ProjectControllerPrivate;
};

class ProjectDialogProvider : public IProjectDialogProvider
{
Q_OBJECT
public:
    ProjectDialogProvider(ProjectControllerPrivate* const p);
    virtual ~ProjectDialogProvider();
    ProjectControllerPrivate* const d;

public Q_SLOTS:
    virtual KUrl askProjectConfigLocation(bool fetch, const KUrl& sta);
    virtual bool userWantsReopen();
};


}
#endif

