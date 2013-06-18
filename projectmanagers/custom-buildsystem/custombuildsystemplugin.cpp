/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
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
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include "configconstants.h"

#include "custombuildjob.h"

using KDevelop::ProjectTargetItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBuildFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFileItem;
using KDevelop::IPlugin;
using KDevelop::ICore;
using KDevelop::IOutputView;
using KDevelop::IGenericProjectManager;
using KDevelop::IProjectFileManager;
using KDevelop::IProjectBuilder;
using KDevelop::IProject;

K_PLUGIN_FACTORY(CustomBuildSystemFactory, registerPlugin<CustomBuildSystem>(); )
K_EXPORT_PLUGIN(CustomBuildSystemFactory(KAboutData("kdevcustombuildsystem","kdevcustombuildsystem", ki18n("Custom Build System"), CUSTOM_BUILDSYSTEM_VERSION, ki18n("Support for building and managing custom build systems"), KAboutData::License_GPL, ki18n("Copyright 2010 Andreas Pakulat <apaku@gmx.de>"), KLocalizedString(), "", "apaku@gmx.de" )))


CustomBuildSystem::CustomBuildSystem( QObject *parent, const QVariantList & )
    : AbstractFileManagerPlugin( CustomBuildSystemFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    setImportFileNameFilter( QStringList() << ".git" << "CVS" << ".svn" << "_svn" << ".kdev4" << "SCCS" << "_darcs" << ".hg" << ".bzr" );
}

CustomBuildSystem::~CustomBuildSystem()
{
}

bool CustomBuildSystem::addFilesToTarget( const QList<ProjectFileItem*>&, ProjectTargetItem* )
{
    return false;
}

KJob* CustomBuildSystem::build( ProjectBaseItem* dom )
{
    return new CustomBuildJob( this, dom, CustomBuildSystemTool::Build );
}

KUrl CustomBuildSystem::buildDirectory( ProjectBaseItem*  item ) const
{
    KUrl u;
    if( item->folder() ) {
        u = item->url();
    } else {
        ProjectBaseItem* parent = item;
        while( !parent->folder() ) {
            parent = dynamic_cast<ProjectBaseItem*>( parent->parent() );
        }
        u = parent->url();
    }
    KUrl projecturl = item->project()->projectItem()->url();
    QString relative = KUrl::relativeUrl( projecturl, u );
    KUrl builddir = configuration( item->project() ).readEntry( ConfigConstants::buildDirKey, projecturl );
    if(!builddir.isValid() )  // set builddir to default if project contains a buildDirKey that does not have a value 
    {
        builddir = projecturl;
    }
    builddir.addPath( relative );
    builddir.cleanPath();
    return builddir;
}

IProjectBuilder* CustomBuildSystem::builder() const
{
    return const_cast<IProjectBuilder*>(dynamic_cast<const IProjectBuilder*>(this));
}

KJob* CustomBuildSystem::clean( ProjectBaseItem* dom )
{
    return new CustomBuildJob( this, dom, CustomBuildSystemTool::Clean );
}

KJob* CustomBuildSystem::configure( IProject* project )
{
    return new CustomBuildJob( this, project->projectItem(), CustomBuildSystemTool::Configure );
}

ProjectTargetItem* CustomBuildSystem::createTarget( const QString&, ProjectFolderItem* )
{
    return 0;
}

QHash< QString, QString > CustomBuildSystem::defines( ProjectBaseItem* item ) const
{
    QHash<QString,QVariant> hash;
    KConfigGroup cfg = configuration( item->project() );
    KConfigGroup groupForItem = findMatchingPathGroup( cfg, item );
    if( groupForItem.isValid() ) {
        QByteArray data = groupForItem.readEntry( ConfigConstants::definesKey, QByteArray() );
        QDataStream ds( data );
        ds.setVersion( QDataStream::Qt_4_5 );
        ds >> hash;
    }
    QHash<QString,QString> defines;
    foreach( const QString& k, hash.keys() )
    {
        defines.insert( k, hash[k].toString() );
    }
    return defines;
}

IProjectFileManager::Features CustomBuildSystem::features() const
{
    return IProjectFileManager::Files | IProjectFileManager::Folders;
}

ProjectFolderItem* CustomBuildSystem::createFolderItem( KDevelop::IProject* project, 
                    const KUrl& url, KDevelop::ProjectBaseItem* parent )
{
    return new ProjectBuildFolderItem( project, url, parent );
}

KUrl::List CustomBuildSystem::includeDirectories( ProjectBaseItem* item ) const
{
    QStringList includes;
    KConfigGroup cfg = configuration( item->project() );
    KConfigGroup groupForItem = findMatchingPathGroup( cfg, item );
    if( groupForItem.isValid() ) {
        QByteArray data = groupForItem.readEntry( ConfigConstants::includesKey, QByteArray() );
        QDataStream ds( data );
        ds.setVersion( QDataStream::Qt_4_5 );
        ds >> includes;
    }
    return KUrl::List( includes );
}

KJob* CustomBuildSystem::install( ProjectBaseItem* item )
{
    return new CustomBuildJob( this, item, CustomBuildSystemTool::Install );
}

KJob* CustomBuildSystem::prune( IProject* project )
{
    return new CustomBuildJob( this, project->projectItem(), CustomBuildSystemTool::Prune );
}

bool CustomBuildSystem::removeFilesFromTargets( const QList<ProjectFileItem*>& )
{
    return false;
}

bool CustomBuildSystem::removeTarget( ProjectTargetItem* )
{
    return false;
}

QList<ProjectTargetItem*> CustomBuildSystem::targets( ProjectFolderItem* ) const
{
    return QList<ProjectTargetItem*>();
}

KConfigGroup CustomBuildSystem::configuration( IProject* project ) const
{
    KConfigGroup grp = project->projectConfiguration()->group( ConfigConstants::customBuildSystemGroup );
    KConfigGroup currentConfig = grp.group( grp.readEntry( ConfigConstants::currentConfigKey ) );
    // This is ugly and actually not necessary as far as support from KConfig* goes, but when using a debug-compiled
    // kdelibs KConfig* has various (IMO) incorrect assert()'s when trying to read entries from a KConfigGroup
    // that does not exist in the config files. There does not seem to be any problem doing so with a non-debug
    // kdelibs, i.e. KConfig* can handle this case just fine. So someone (who can patch kdelibs) should probably remove
    // those useless assert()'s.
    currentConfig.sync();
    return currentConfig;
}

KConfigGroup CustomBuildSystem::findMatchingPathGroup(const KConfigGroup& cfg, ProjectBaseItem* item) const
{
    KConfigGroup candidateGroup;
    KUrl candidateTargetDirectory;

    KUrl itemUrl = item->url();
    KUrl rootDirectory = item->project()->folder();

    foreach( const QString& groupName, cfg.groupList() ) {
        if( groupName.startsWith( ConfigConstants::projectPathPrefix ) ) {
            KConfigGroup pathGroup = cfg.group(groupName);

            QString targetDirectoryRelative = pathGroup.readEntry( ConfigConstants::projectPathKey, "" );
            KUrl targetDirectory = rootDirectory;
            // note: a dot represents the project root
            if (targetDirectoryRelative != ".") {
                targetDirectory.addPath( targetDirectoryRelative );
            }

            if( targetDirectory.isParentOf(itemUrl) ) {
                if( candidateTargetDirectory.isEmpty() || candidateTargetDirectory.isParentOf(targetDirectory) ) {
                  candidateGroup = pathGroup;
                  candidateTargetDirectory = targetDirectory;
                }
            }
        }
    }
    return candidateGroup;
}

#include "custombuildsystemplugin.moc"
