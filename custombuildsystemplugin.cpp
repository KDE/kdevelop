/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "custombuildsystemplugin.h"

#include <KPluginFactory>
#include <KLocale>
#include <KAboutData>
#include <KComponentData>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <outputview/ioutputview.h>
#include <util/environmentgrouplist.h>
#include <interfaces/iplugincontroller.h>
#include "configconstants.h"

#include <genericprojectmanager/igenericprojectmanager.h>

using KDevelop::ProjectTargetItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFileItem;
using KDevelop::IPlugin;
using KDevelop::EnvironmentGroupList;
using KDevelop::ICore;
using KDevelop::IOutputView;
using KDevelop::IGenericProjectManager;
using KDevelop::IProjectFileManager;
using KDevelop::IProjectBuilder;
using KDevelop::IProject;

K_PLUGIN_FACTORY(CustomBuildSystemFactory, registerPlugin<CustomBuildSystem>(); )
K_EXPORT_PLUGIN(CustomBuildSystemFactory(KAboutData("kdevcustombuildsystem","kdevcustombuildsystem", ki18n("Custom BuildSystem"), "0.1", ki18n("Support for building and managing Custom Buildsystems"), KAboutData::License_GPL, ki18n("Copyright 2010 Andreas Pakulat <apaku@gmx.de>"), ki18n(""), "", "apaku@gmx.de" )))


CustomBuildSystem::CustomBuildSystem(QObject *parent, const QVariantList &)
    : IPlugin(CustomBuildSystemFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
}

CustomBuildSystem::~CustomBuildSystem()
{
}

ProjectFileItem* CustomBuildSystem::addFile( const KUrl& folder, ProjectFolderItem* parent )
{
    return genericManager()->addFile( folder, parent );
}

bool CustomBuildSystem::addFileToTarget( ProjectFileItem* file, ProjectTargetItem* parent )
{
    return 0;
}

ProjectFolderItem* CustomBuildSystem::addFolder( const KUrl& folder, ProjectFolderItem* parent )
{
    return genericManager()->addFolder( folder, parent );
}

KJob* CustomBuildSystem::build( ProjectBaseItem* dom )
{
    return 0;
}

KUrl CustomBuildSystem::buildDirectory( ProjectBaseItem*  item ) const
{
    KUrl u = item->url();
    KUrl projecturl = item->project()->projectItem()->url();
    QString relative = KUrl::relativeUrl( projecturl, u );
    KUrl builddir = configuration( item->project() ).readEntry( ConfigConstants::buildDirKey, projecturl );
    builddir.addPath( relative );
    builddir.cleanPath();
    return builddir;
}

IProjectBuilder* CustomBuildSystem::builder( ProjectFolderItem*  ) const
{
    return const_cast<IProjectBuilder*>(dynamic_cast<const IProjectBuilder*>(this));
}

KJob* CustomBuildSystem::clean( ProjectBaseItem* dom )
{
    return 0;
}

KJob* CustomBuildSystem::configure( IProject*  )
{
    return 0;
}

ProjectTargetItem* CustomBuildSystem::createTarget( const QString& target, ProjectFolderItem* parent )
{
    return 0;
}

QHash< QString, QString > CustomBuildSystem::defines( ProjectBaseItem* item ) const
{
    QHash<QString,QString> hash;
    QByteArray data = configuration( item->project() ).readEntry( ConfigConstants::definesKey, QByteArray() );
    QDataStream ds( data );
    ds >> hash;
    return hash;
}

QHash< QString, QString > CustomBuildSystem::environment( ProjectBaseItem* item ) const
{
    EnvironmentGroupList l( KGlobal::config() );
    QHash<QString,QString> envvars;
    foreach( QString s, l.createEnvironment( configuration( item->project() ).readEntry(
                            ConfigConstants::environmentKey, "default" ), QStringList() ) ) {
        int idx = s.indexOf( "=" );
        envvars.insert( s.left( idx ), s.mid( idx+1 ) );
    }
    return envvars;
}

IProjectFileManager::Features CustomBuildSystem::features() const
{
    return IProjectFileManager::Files | IProjectFileManager::Folders;
}

ProjectFolderItem* CustomBuildSystem::import( IProject* project )
{
    return genericManager()->import( project );
}

KUrl::List CustomBuildSystem::includeDirectories( ProjectBaseItem*  ) const
{
    return KUrl::List();
}

KJob* CustomBuildSystem::install( ProjectBaseItem* item )
{
    return 0;
}

QList< ProjectFolderItem* > CustomBuildSystem::parse( ProjectFolderItem* dom )
{
    return genericManager()->parse( dom );
}

KJob* CustomBuildSystem::prune( IProject* )
{
    return 0;
}

bool CustomBuildSystem::reload( ProjectFolderItem* item )
{
    return genericManager()->reload( item );
}

bool CustomBuildSystem::removeFile( ProjectFileItem* file )
{
    return genericManager()->removeFile( file );
}

bool CustomBuildSystem::removeFileFromTarget( ProjectFileItem* file, ProjectTargetItem* parent )
{
    return false;
}

bool CustomBuildSystem::removeFolder( ProjectFolderItem* folder )
{
    return genericManager()->removeFolder( folder );
}

bool CustomBuildSystem::removeTarget( ProjectTargetItem* target )
{
    return false;
}

bool CustomBuildSystem::renameFile( ProjectFileItem* oldFile, const KUrl& newFile )
{
    return genericManager()->renameFile( oldFile, newFile );
}

bool CustomBuildSystem::renameFolder( ProjectFolderItem* oldFolder, const KUrl& newFolder )
{
    return genericManager()->renameFolder( oldFolder, newFolder );
}

QList<ProjectTargetItem*> CustomBuildSystem::targets( ProjectFolderItem* ) const
{
    return QList<ProjectTargetItem*>();
}

IGenericProjectManager* CustomBuildSystem::genericManager() const
{
    IGenericProjectManager* manager = ICore::self()->pluginController()->extensionForPlugin<KDevelop::IGenericProjectManager>( "org.kdevelop.IGenericProjectManager" );
    Q_ASSERT(manager);
    return manager;
}

IOutputView* CustomBuildSystem::outputView() const
{
    IOutputView* view = ICore::self()->pluginController()->extensionForPlugin<KDevelop::IOutputView>( "org.kdevelop.IOutputView" );
    Q_ASSERT(view);
    return view;
}

KJob* CustomBuildSystem::createImportJob( ProjectFolderItem* item )
{
    return genericManager()->createImportJob(item);
}

KConfigGroup CustomBuildSystem::configuration( IProject* project ) const
{
    KConfigGroup grp = project->projectConfiguration()->group( ConfigConstants::customBuildSystemGroup );
    return grp.group( grp.readEntry( ConfigConstants::currentConfigKey ) );
}

#include "custombuildsystemplugin.moc"
