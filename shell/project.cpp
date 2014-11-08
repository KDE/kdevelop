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
#include <QtGlobal>
#include <QFileInfo>
#include <QtDBus/QtDBus>
#include <QStandardItemModel>
#include <QtCore/QDebug>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KLocalizedString>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kmessagebox.h>
#include <kio/jobclasses.h>
#include <QTemporaryFile>

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
        virtual ~ProjectProgress();
        virtual QString statusName() const;

        /*! Show indeterminate mode progress bar */
        void setBuzzy();

        /*! Hide progress bar */
        void setDone();

        QString projectName;

    private Q_SLOTS:
        void slotClean();

    Q_SIGNALS:
        void clearMessage(KDevelop::IStatus*);
        void showMessage(KDevelop::IStatus*,const QString & message, int timeout = 0);
        void showErrorMessage(const QString & message, int timeout = 0);
        void hideProgress(KDevelop::IStatus*);
        void showProgress(KDevelop::IStatus*,int minimum, int maximum, int value);

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
    QString projectTempFile;
    IPlugin* manager;
    QPointer<IPlugin> vcsPlugin;
    ProjectFolderItem* topItem;
    QString name;
    KSharedConfig::Ptr m_cfg;
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
        developerFile.setLastPathSegment( ".kdev4" );
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

        if( !KIO::NetAccess::download( projectFile.toUrl(), projectTempFile,
                        Core::self()->uiController()->activeMainWindow() ) )
        {
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
            statJob = KIO::stat( developerFile.toUrl(), KIO::HideProgressInfo );
            if( !statJob->exec() || !KIO::NetAccess::download( developerFile.toUrl(), developerTempFile,
                Core::self()->uiController()->activeMainWindow() ) )
            {
                QTemporaryFile tmp;
                tmp.open();
                developerTempFile = tmp.fileName();
            }
        }
        return true;
    }

    KConfigGroup initKConfigObject()
    {
        // helper method for open()
        qCDebug(SHELL) << "Creating KConfig object for project files" << developerTempFile << projectTempFile;
        m_cfg = KSharedConfig::openConfig( developerTempFile );
        m_cfg->addConfigSources( QStringList() << projectTempFile );
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

            qWarning() << "Trying to open a project with a name thats already used by another open project";
            return true;
        }
        return false;
    }

    IProjectFileManager* fetchFileManager(const KConfigGroup& projectGroup)
    {
        if (manager)
        {
            IProjectFileManager* iface = 0;
            iface = manager->extension<KDevelop::IProjectFileManager>();
            Q_ASSERT(iface);
            return iface;
        }

        // helper method for open()
        QString managerSetting = projectGroup.readEntry( "Manager", "KDevGenericManager" );

        //Get our importer
        IPluginController* pluginManager = Core::self()->pluginController();
        manager = pluginManager->pluginForExtension( "org.kdevelop.IProjectFileManager", managerSetting );
        IProjectFileManager* iface = 0;
        if ( manager )
            iface = manager->extension<IProjectFileManager>();
        else
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Could not load project management plugin %1.",
                                  managerSetting ) );
            manager = 0;
            return 0;
        }
        if (iface == 0)
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "project importing plugin (%1) does not support the IProjectFileManager interface.", managerSetting ) );
            delete manager;
            manager = 0;
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
                vcsPlugin = pluginManager->pluginForExtension( "org.kdevelop.IBasicVersionControl", vcsPluginName );
            }
        } else
        {
            foreach( IPlugin* p, pluginManager->allPluginsForExtension( "org.kdevelop.IBasicVersionControl" ) )
            {
                IBasicVersionControl* iface = p->extension<KDevelop::IBasicVersionControl>();
                if( iface && iface->isVersionControlled( topItem->path().toUrl() ) )
                {
                    vcsPlugin = p;
                    projectGroup.writeEntry("VersionControlSupport", pluginManager->pluginInfo( p ).pluginName() );
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
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/Project", this, QDBusConnection::ExportScriptableSlots );

    d->project = this;
    d->manager = 0;
    d->topItem = 0;
    d->loading = false;
    d->scheduleReload = false;
    d->progress = new ProjectProgress;
    Core::self()->uiController()->registerStatus( d->progress );
}

Project::~Project()
{
    delete d->progress;
    delete d;
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
    return d->projectTempFile;
}

KSharedConfig::Ptr Project::projectConfiguration() const
{
    return d->m_cfg;
}

const QUrl Project::folder() const
{
    QUrl url = d->projectPath.toUrl();
    // FIXME: is something like this required here?
//     url.adjustPath(QUrl::AddTrailingSlash);
    return url;
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
    d->topItem = 0;

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
        qWarning() << "Something went wrong. ProjectFolderItem detached. Project closed during reload?";
        return;
    }

    Core::self()->projectController()->projectModel()->removeRow( d->topItem->row() );

    if( !d->developerFile.isLocalFile() && !KIO::NetAccess::upload( d->developerTempFile, d->developerFile.toUrl(),
                Core::self()->uiController()->activeMainWindow() ) )
    {
        KMessageBox::sorry( Core::self()->uiController()->activeMainWindow(),
                    i18n("Could not store developer specific project configuration.\n"
                         "Attention: The project settings you changed will be lost."
                    ) );
    }
}

bool Project::inProject( const IndexedString& path ) const
{
    if (d->fileSet.contains( path )) {
        return true;
    }
    return !d->itemsForPath( path ).isEmpty();
}

QList< ProjectBaseItem* > Project::itemsForUrl(const QUrl &url) const
{
    return d->itemsForPath(IndexedString(url));
}

QList< ProjectBaseItem* > Project::itemsForPath(const IndexedString& path) const
{
    return d->itemsForPath(path);
}

QList<ProjectFileItem*> Project::filesForUrl(const QUrl &url) const
{
    return filesForPath(IndexedString(url));
}

QList< ProjectFileItem* > Project::filesForPath(const IndexedString& file) const
{
    QList<ProjectFileItem*> items;
    foreach(ProjectBaseItem* item,  d->itemsForPath( file ) )
    {
        if( item->type() == ProjectBaseItem::File )
            items << dynamic_cast<ProjectFileItem*>( item );
    }
    return items;
}

QList< ProjectFolderItem* > Project::foldersForUrl(const QUrl &url) const
{
    return foldersForPath(IndexedString(url));
}

QList<ProjectFolderItem*> Project::foldersForPath(const IndexedString& folder) const
{
    QList<ProjectFolderItem*> items;
    foreach(ProjectBaseItem* item,  d->itemsForPath( folder ) )
    {
        if( item->type() == ProjectBaseItem::Folder || item->type() == ProjectBaseItem::BuildFolder )
            items << dynamic_cast<ProjectFolderItem*>( item );
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

QUrl Project::projectFileUrl() const
{
    return d->projectFile.toUrl();
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
