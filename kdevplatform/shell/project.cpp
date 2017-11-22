    /* This file is part of the KDE project
   Copyright 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2002 Simon Hausmann <hausmann@kde.org>
   Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright     2006 Matt Rogers <mattr@kde.org>
   Copyright     2007 Andreas Pakulat <apaku@gmx.de>

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

#include "project.h"

#include <QSet>
#include <QTemporaryFile>
#include <QTimer>
#include <QPointer>

#include <KConfigGroup>
#include <KIO/FileCopyJob>
#include <KIO/MkdirJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/isession.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <serialization/indexedstring.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

#include "core.h"
#include "mainwindow.h"
#include "projectcontroller.h"
#include "uicontroller.h"
#include "debug.h"

namespace KDevelop
{

class ProjectProgress : public QObject, public IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

    public:
        ProjectProgress();
        ~ProjectProgress() override;
        QString statusName() const override;

        /*! Show indeterminate mode progress bar */
        void setBuzzy();

        /*! Hide progress bar */
        void setDone();

        QString projectName;

    private Q_SLOTS:
        void slotClean();

    Q_SIGNALS:
        void clearMessage(KDevelop::IStatus*) override;
        void showMessage(KDevelop::IStatus*,const QString & message, int timeout = 0) override;
        void showErrorMessage(const QString & message, int timeout = 0) override;
        void hideProgress(KDevelop::IStatus*) override;
        void showProgress(KDevelop::IStatus*,int minimum, int maximum, int value) override;

    private:
        QTimer* m_timer;
};



ProjectProgress::ProjectProgress()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot( true );
    m_timer->setInterval( 1000 );
    connect(m_timer, &QTimer::timeout,this, &ProjectProgress::slotClean);
}

ProjectProgress::~ProjectProgress()
{
}

QString ProjectProgress::statusName() const
{
    return i18n("Loading Project %1", projectName);
}

void ProjectProgress::setBuzzy()
{
    qCDebug(SHELL) << "showing busy progress" << statusName();
    // show an indeterminate progressbar
    emit showProgress(this, 0,0,0);
    emit showMessage(this, i18nc("%1: Project name", "Loading %1", projectName));
}


void ProjectProgress::setDone()
{
    qCDebug(SHELL) << "showing done progress" << statusName();
    // first show 100% bar for a second, then hide.
    emit showProgress(this, 0,1,1);
    m_timer->start();
}

void ProjectProgress::slotClean()
{
    emit hideProgress(this);
    emit clearMessage(this);
}

class ProjectPrivate
{
public:
    Path projectPath;
    Path projectFile;
    Path developerFile;
    QString developerTempFile;
    QTemporaryFile projectTempFile;
    IPlugin* manager;
    QPointer<IPlugin> vcsPlugin;
    ProjectFolderItem* topItem;
    QString name;
    KSharedConfigPtr m_cfg;
    IProject *project;
    QSet<KDevelop::IndexedString> fileSet;
    bool loading;
    bool fullReload;
    bool scheduleReload;
    ProjectProgress* progress;

    void reloadDone(KJob* job)
    {
        progress->setDone();
        loading = false;

        ProjectController* projCtrl = Core::self()->projectControllerInternal();
        if (job->error() == 0 && !Core::self()->shuttingDown()) {

            if(fullReload)
                projCtrl->projectModel()->appendRow(topItem);

            if (scheduleReload) {
                scheduleReload = false;
                project->reloadModel();
            }
        } else {
            projCtrl->abortOpeningProject(project);
        }
    }

    QList<ProjectBaseItem*> itemsForPath( const IndexedString& path ) const
    {
        if ( path.isEmpty() ) {
            return QList<ProjectBaseItem*>();
        }

        if (!topItem->model()) {
            // this gets hit when the project has not yet been added to the model
            // i.e. during import phase
            // TODO: should we handle this somehow?
            // possible idea: make the item<->path hash per-project
            return QList<ProjectBaseItem*>();
        }

        Q_ASSERT(topItem->model());
        QList<ProjectBaseItem*> items = topItem->model()->itemsForPath(path);

        QList<ProjectBaseItem*>::iterator it = items.begin();
        while(it != items.end()) {
            if ((*it)->project() != project) {
                it = items.erase(it);
            } else {
                ++it;
            }
        }

        return items;
    }


    void importDone( KJob* job)
    {
        progress->setDone();
        ProjectController* projCtrl = Core::self()->projectControllerInternal();

        if(job->error() == 0 && !Core::self()->shuttingDown()) {
            loading=false;
            projCtrl->projectModel()->appendRow(topItem);
            projCtrl->projectImportingFinished( project );
        } else {
            projCtrl->abortOpeningProject(project);
        }
    }

    void initProject(const Path& projectFile_)
    {
        // helper method for open()
        projectFile = projectFile_;
    }

    bool initProjectFiles()
    {
        KIO::StatJob* statJob = KIO::stat( projectFile.toUrl(), KIO::HideProgressInfo );
        if ( !statJob->exec() ) //be sync for right now
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Unable to load the project file %1.<br>"
                                  "The project has been removed from the session.",
                                  projectFile.pathOrUrl() ) );
            return false;
        }

        // developerfile == dirname(projectFileUrl) ."/.kdev4/". basename(projectfileUrl)
        developerFile = projectFile;
        developerFile.setLastPathSegment( QStringLiteral(".kdev4") );
        developerFile.addPath( projectFile.lastPathSegment() );

        statJob = KIO::stat( developerFile.toUrl(), KIO::HideProgressInfo );
        if( !statJob->exec() )
        {
            // the developerfile does not exist yet, check if its folder exists
            // the developerfile itself will get created below
            QUrl dir = developerFile.parent().toUrl();
            statJob = KIO::stat( dir, KIO::HideProgressInfo );
            if( !statJob->exec() )
            {
                KIO::SimpleJob* mkdirJob = KIO::mkdir( dir );
                if( !mkdirJob->exec() )
                {
                    KMessageBox::sorry(
                        Core::self()->uiController()->activeMainWindow(),
                        i18n("Unable to create hidden dir (%1) for developer file",
                        dir.toDisplayString(QUrl::PreferLocalFile) )
                        );
                    return false;
                }
            }
        }

        projectTempFile.open();
        auto copyJob = KIO::file_copy(projectFile.toUrl(), QUrl::fromLocalFile(projectTempFile.fileName()), -1, KIO::HideProgressInfo | KIO::Overwrite);
        KJobWidgets::setWindow(copyJob, Core::self()->uiController()->activeMainWindow());
        if (!copyJob->exec())
        {
            qCDebug(SHELL) << "Job failed:" << copyJob->errorString();

            KMessageBox::sorry( Core::self()->uiController()->activeMainWindow(),
                            i18n("Unable to get project file: %1",
                            projectFile.pathOrUrl() ) );
            return false;
        }

        if(developerFile.isLocalFile())
        {
            developerTempFile = developerFile.toLocalFile();
        }
        else {
            QTemporaryFile tmp;
            tmp.open();
            developerTempFile = tmp.fileName();

            auto job = KIO::file_copy(developerFile.toUrl(), QUrl::fromLocalFile(developerTempFile), -1, KIO::HideProgressInfo | KIO::Overwrite);
            KJobWidgets::setWindow(job, Core::self()->uiController()->activeMainWindow());
            job->exec();
        }
        return true;
    }

    KConfigGroup initKConfigObject()
    {
        // helper method for open()
        qCDebug(SHELL) << "Creating KConfig object for project files" << developerTempFile << projectTempFile.fileName();
        m_cfg = KSharedConfig::openConfig( developerTempFile );
        m_cfg->addConfigSources( QStringList() << projectTempFile.fileName() );
        KConfigGroup projectGroup( m_cfg, "Project" );
        return projectGroup;
    }

    bool projectNameUsed(const KConfigGroup& projectGroup)
    {
        // helper method for open()
        name = projectGroup.readEntry( "Name", projectFile.lastPathSegment() );
        progress->projectName = name;
        if( Core::self()->projectController()->isProjectNameUsed( name ) )
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                                i18n( "Could not load %1, a project with the same name '%2' is already open.",
                                projectFile.pathOrUrl(), name ) );

            qCWarning(SHELL) << "Trying to open a project with a name that is already used by another open project";
            return true;
        }
        return false;
    }

    IProjectFileManager* fetchFileManager(const KConfigGroup& projectGroup)
    {
        if (manager)
        {
            IProjectFileManager* iface = manager->extension<KDevelop::IProjectFileManager>();
            Q_ASSERT(iface);
            return iface;
        }

        // helper method for open()
        QString managerSetting = projectGroup.readEntry( "Manager", "KDevGenericManager" );

        //Get our importer
        IPluginController* pluginManager = Core::self()->pluginController();
        manager = pluginManager->pluginForExtension( QStringLiteral("org.kdevelop.IProjectFileManager"), managerSetting );
        IProjectFileManager* iface = nullptr;
        if ( manager )
            iface = manager->extension<IProjectFileManager>();
        else
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Could not load project management plugin <b>%1</b>.<br> Check that the required programs are installed,"
                                  " or see console output for more information.",
                                  managerSetting ) );
            manager = nullptr;
            return nullptr;
        }
        if (iface == nullptr)
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "project importing plugin (%1) does not support the IProjectFileManager interface.", managerSetting ) );
            delete manager;
            manager = nullptr;
        }
        return iface;
    }

    void loadVersionControlPlugin(KConfigGroup& projectGroup)
    {
        // helper method for open()
        IPluginController* pluginManager = Core::self()->pluginController();
        if( projectGroup.hasKey( "VersionControlSupport" ) )
        {
            QString vcsPluginName = projectGroup.readEntry("VersionControlSupport", "");
            if( !vcsPluginName.isEmpty() )
            {
                vcsPlugin = pluginManager->pluginForExtension( QStringLiteral( "org.kdevelop.IBasicVersionControl" ), vcsPluginName );
            }
        } else
        {
            QList<IPlugin*> plugins = pluginManager->allPluginsForExtension( QStringLiteral( "org.kdevelop.IBasicVersionControl" ) );
            foreach( IPlugin* p, plugins )
            {
                IBasicVersionControl* iface = p->extension<KDevelop::IBasicVersionControl>();
                if (!iface) {
                    continue;
                }

                const auto url = topItem->path().toUrl();
                qCDebug(SHELL) << "Checking whether" << url << "is version controlled by" << iface->name();
                if(iface->isVersionControlled(url))
                {
                    qDebug(SHELL) << "Detected that" << url << "is a" << iface->name() << "project";

                    vcsPlugin = p;
                    projectGroup.writeEntry("VersionControlSupport", pluginManager->pluginInfo(p).pluginId());
                    projectGroup.sync();
                }
            }
        }

    }

    bool importTopItem(IProjectFileManager* fileManager)
    {
        if (!fileManager)
        {
            return false;
        }
        topItem = fileManager->import( project );
        if( !topItem )
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                                i18n("Could not open project") );
            return false;
        }

        return true;
    }

};

Project::Project( QObject *parent )
        : IProject( parent )
        , d( new ProjectPrivate )
{
    d->project = this;
    d->manager = nullptr;
    d->topItem = nullptr;
    d->loading = false;
    d->scheduleReload = false;
    d->progress = new ProjectProgress;
    Core::self()->uiController()->registerStatus( d->progress );
}

Project::~Project()
{
    delete d->progress;
}

QString Project::name() const
{
    return d->name;
}

QString Project::developerTempFile() const
{
    return d->developerTempFile;
}

QString Project::projectTempFile() const
{
    return d->projectTempFile.fileName();
}

KSharedConfigPtr Project::projectConfiguration() const
{
    return d->m_cfg;
}

Path Project::path() const
{
    return d->projectPath;
}

void Project::reloadModel()
{
    if (d->loading) {
        d->scheduleReload = true;
        return;
    }
    d->loading = true;
    d->fileSet.clear();

    // delete topItem and remove it from model
    ProjectModel* model = Core::self()->projectController()->projectModel();
    model->removeRow( d->topItem->row() );
    d->topItem = nullptr;

    IProjectFileManager* iface = d->manager->extension<IProjectFileManager>();
    if (!d->importTopItem(iface))
    {
            d->loading = false;
            d->scheduleReload = false;
            return;
    }

    KJob* importJob = iface->createImportJob(d->topItem );
    setReloadJob(importJob);
    d->fullReload = true;
    Core::self()->runController()->registerJob( importJob );
}

void Project::setReloadJob(KJob* job)
{
    d->loading = true;
    d->fullReload = false;
    d->progress->setBuzzy();
    connect(job, &KJob::finished, this, [&] (KJob* job) { d->reloadDone(job); });
}

bool Project::open( const Path& projectFile )
{
    d->initProject(projectFile);
    if (!d->initProjectFiles())
        return false;

    KConfigGroup projectGroup = d->initKConfigObject();
    if (d->projectNameUsed(projectGroup))
        return false;

    d->projectPath = d->projectFile.parent();

    IProjectFileManager* iface = d->fetchFileManager(projectGroup);
    if (!iface)
        return false;

    Q_ASSERT(d->manager);

    emit aboutToOpen(this);
    if (!d->importTopItem(iface) ) {
        return false;
    }

    d->loading=true;
    d->loadVersionControlPlugin(projectGroup);
    d->progress->setBuzzy();
    KJob* importJob = iface->createImportJob(d->topItem );
    connect( importJob, &KJob::result, this, [&] (KJob* job) { d->importDone(job); } );
    Core::self()->runController()->registerJob( importJob );
    return true;
}

void Project::close()
{
    Q_ASSERT(d->topItem);
    if (d->topItem->row() == -1) {
        qCWarning(SHELL) << "Something went wrong. ProjectFolderItem detached. Project closed during reload?";
        return;
    }

    Core::self()->projectController()->projectModel()->removeRow( d->topItem->row() );

    if (!d->developerFile.isLocalFile())
    {
        auto copyJob = KIO::file_copy(QUrl::fromLocalFile(d->developerTempFile), d->developerFile.toUrl(), -1, KIO::HideProgressInfo);
        KJobWidgets::setWindow(copyJob, Core::self()->uiController()->activeMainWindow());
        if (!copyJob->exec()) {
            qCDebug(SHELL) << "Job failed:" << copyJob->errorString();

            KMessageBox::sorry(Core::self()->uiController()->activeMainWindow(),
                               i18n("Could not store developer specific project configuration.\n"
                                    "Attention: The project settings you changed will be lost."));
        }
    }
}

bool Project::inProject( const IndexedString& path ) const
{
    if (d->fileSet.contains( path )) {
        return true;
    }
    return !d->itemsForPath( path ).isEmpty();
}

QList< ProjectBaseItem* > Project::itemsForPath(const IndexedString& path) const
{
    return d->itemsForPath(path);
}

QList< ProjectFileItem* > Project::filesForPath(const IndexedString& file) const
{
    QList<ProjectFileItem*> items;
    foreach(ProjectBaseItem* item,  d->itemsForPath( file ) )
    {
        if( item->type() == ProjectBaseItem::File )
            items << static_cast<ProjectFileItem*>(item);
    }
    return items;
}

QList<ProjectFolderItem*> Project::foldersForPath(const IndexedString& folder) const
{
    QList<ProjectFolderItem*> items;
    foreach(ProjectBaseItem* item,  d->itemsForPath( folder ) )
    {
        if( item->type() == ProjectBaseItem::Folder || item->type() == ProjectBaseItem::BuildFolder )
            items << static_cast<ProjectFolderItem*>(item);
    }
    return items;
}

IProjectFileManager* Project::projectFileManager() const
{
    return d->manager->extension<IProjectFileManager>();
}

IBuildSystemManager* Project::buildSystemManager() const
{
    return d->manager->extension<IBuildSystemManager>();
}

IPlugin* Project::managerPlugin() const
{
  return d->manager;
}

void Project::setManagerPlugin( IPlugin* manager )
{
    d->manager = manager;
}

Path Project::projectFile() const
{
    return d->projectFile;
}

Path Project::developerFile() const
{
    return d->developerFile;
}

ProjectFolderItem* Project::projectItem() const
{
    return d->topItem;
}

IPlugin* Project::versionControlPlugin() const
{
    return d->vcsPlugin.data();
}

void Project::addToFileSet( ProjectFileItem* file )
{
    if (d->fileSet.contains(file->indexedPath())) {
        return;
    }

    d->fileSet.insert( file->indexedPath() );
    emit fileAddedToSet( file );
}

void Project::removeFromFileSet( ProjectFileItem* file )
{
    QSet<IndexedString>::iterator it = d->fileSet.find(file->indexedPath());
    if (it == d->fileSet.end()) {
        return;
    }

    d->fileSet.erase( it );
    emit fileRemovedFromSet( file );
}

QSet<IndexedString> Project::fileSet() const
{
    return d->fileSet;
}

bool Project::isReady() const
{
    return !d->loading;
}

} // namespace KDevelop

#include "project.moc"
#include "moc_project.cpp"
