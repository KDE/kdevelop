/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTCONTROLLER_H
#define KDEVPLATFORM_PROJECTCONTROLLER_H

#include <interfaces/iprojectcontroller.h>

#include <QUrl>

#include <KIO/UDSEntry>

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
class ProjectControllerPrivate;

class KDEVPLATFORMSHELL_EXPORT IProjectDialogProvider : public QObject
{
Q_OBJECT
public:
    IProjectDialogProvider();
    ~IProjectDialogProvider() override;

public Q_SLOTS:
    /**
     * Displays some UI to ask the user for the project location.
     *
     * @param fetch will tell the UI that the user might want to fetch the project first
     * @param startUrl tells where to look first
     */
    virtual QUrl askProjectConfigLocation(bool fetch, const QUrl& startUrl = QUrl(),
                                          const QUrl& repoUrl = QUrl(), IPlugin* plugin = nullptr) = 0;
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
    explicit ProjectController( Core* core );
    ~ProjectController() override;

    IProject* projectAt( int ) const override;
    int projectCount() const override;
    QList<IProject*> projects() const override;

    ProjectBuildSetModel* buildSetModel() override;
    ProjectModel* projectModel() override;
    [[nodiscard]] std::shared_ptr<ProjectChangesModel> changesModel() const override;
    std::shared_ptr<ProjectChangesModel> makeChangesModel() override;
    IProject* findProjectByName( const QString& name ) override;
    IProject* findProjectForUrl( const QUrl& ) const override;

    bool isProjectNameUsed( const QString& name ) const override;
    void setDialogProvider(IProjectDialogProvider*);

    QUrl projectsBaseDirectory() const override;
    QString prettyFileName(const QUrl& url, FormattingOptions format = FormatHtml) const override;
    QString prettyFilePath(const QUrl& url, FormattingOptions format = FormatHtml) const override;

    ContextMenuExtension contextMenuExtension(KDevelop::Context* ctx, QWidget* parent);

    enum FetchFlag {
        NoFetchFlags = 0,
        FetchShowErrorIfNotSupported = 1,
    };
    Q_DECLARE_FLAGS(FetchFlags, FetchFlag)

    /**
     * @param repoUrl url identifying the repo
     * @returns @c true if a plugin was found to handle the repo (also if user cancelled), @c false otherwise
     */
    bool fetchProjectFromUrl(const QUrl& repoUrl, FetchFlags fetchFlags = FetchShowErrorIfNotSupported);

public Q_SLOTS:
    Q_SCRIPTABLE void openProjectForUrl( const QString &sourceUrl ) { openProjectForUrl(QUrl(sourceUrl)); }
    void openProjectForUrl( const QUrl &sourceUrl ) override;
    void fetchProject();
    void openProject( const QUrl &KDev4ProjectFile = QUrl() ) override;
    void abortOpeningProject( IProject* );
    void projectImportingFinished( IProject* );
    void closeProject( IProject* ) override;
    void closeAllProjects() override;
    void configureProject( IProject* ) override;

    void reparseProject( IProject* project, bool forceUpdate = false, bool forceAll = false  ) override;

    void eventuallyOpenProjectFile(KIO::Job* job, const KIO::UDSEntryList& entries);
    void openProjectForUrlSlot(bool);
//     void changeCurrentProject( ProjectBaseItem* );
    void openProjects(const QList<QUrl>& projects);
    void commitCurrentProject();

    // Maps the given path from the source to the equivalent path within the build directory
    // of the corresponding project. If the path is already in the build directory and fallbackRoot is true,
    // then it is mapped to the root of the build directory.
    // This is used through DBus from within kdevplatform_shell_environment.sh
    // If reverse is true, maps the opposite direction, from build to source. [ Used in kdevplatform_shell_environment.sh ]
    Q_SCRIPTABLE QString mapSourceBuild( const QString& path, bool reverse = false, bool fallbackRoot = true ) const;

protected:
    /**
     * Add the existing project @p project to the controller
     *
     * @note Method is used for testing objectives, consider using openProject() instead!
     * @note takes ownership over the project
     *
     * @sa openProject()
     */
    void addProject(IProject* proj);
    /**
     * Remove the project @p project from the controller
     *
     * @note Ownership is passed on to the caller
     */
    void takeProject(IProject* proj);

    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );

    virtual void initialize();

Q_SIGNALS:
    void initialized();

private:
    //FIXME Do not load all of this just for the project being opened...
    //void legacyLoading();
    void setupActions();
    void cleanup();

    void saveRecentProjectsActionEntries();

    // helper methods for closeProject()
    void unloadUnusedProjectPlugins(IProject* proj);
    void disableProjectCloseAction();
    void closeAllOpenedFiles(IProject* proj);
    void initializePluginCleanup(IProject* proj);

private:
    const QScopedPointer<class ProjectControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectController)
    friend class ProjectControllerPrivate;
};

class ProjectDialogProvider : public IProjectDialogProvider
{
Q_OBJECT
public:
    explicit ProjectDialogProvider(ProjectControllerPrivate* p);
    ~ProjectDialogProvider() override;
    ProjectControllerPrivate* const d;

public Q_SLOTS:
    QUrl askProjectConfigLocation(bool fetch, const QUrl& startUrl,
                                  const QUrl& repoUrl, IPlugin* plugin) override;
    bool userWantsReopen() override;
};


}
#endif
