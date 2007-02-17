/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakeimporter.h"

#include <QList>
#include <QVector>
#include <QDomDocument>

#include <QtDesigner/QExtensionFactory>

#include <kurl.h>
#include <kio/job.h>

#include "icore.h"
#include "iproject.h"
#include "kgenericfactory.h"
#include "projectmodel.h"

#include "cmakeconfig.h"
#include "cmakemodelitems.h"

typedef KGenericFactory<CMakeImporter> CMakeSupportFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevcmakeimporter,
                            CMakeSupportFactory( "kdevcmakeimporter" ) )

KDEV_ADD_EXTENSION_FACTORY_NS( KDevelop, IBuildSystemManager, CMakeImporter )
KDEV_ADD_EXTENSION_FACTORY_NS( KDevelop, IProjectFileManager, CMakeImporter )

CMakeImporter::CMakeImporter( QObject* parent,
                              const QStringList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent ), m_rootItem(0L)
{
/*    CMakeSettings* settings = CMakeSettings::self();

    //what do the settings say about our generator?
    QString generator = settings->generator();
    if ( generator.contains( "Unix" ) ) //use make
        m_builder = new KDevMakeBuilder()*/
}

CMakeImporter::~CMakeImporter()
{
    //delete m_rootItem;
}

// KDevelop::IProject* CMakeImporter::project() const
// {
//     return m_project;
// }

KUrl CMakeImporter::buildDirectory(KDevelop::ProjectItem *item) const
{
    return item->project()->folder();
}

QList<KDevelop::ProjectFolderItem*> CMakeImporter::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );
    if ( !folder )
        return folderList;

    FolderInfo fi = folder->folderInfo();
    for ( QStringList::iterator it = fi.includes.begin();
          it != fi.includes.end(); ++it )
    {
        KUrl urlCandidate = KUrl( ( *it ) );
        if ( m_includeDirList.indexOf( urlCandidate ) == -1 )
            m_includeDirList.append( urlCandidate );
    }

    foreach ( FolderInfo sfi, fi.subFolders )
        folderList.append( new CMakeFolderItem( item->project(), sfi, folder ) );

    foreach ( TargetInfo ti, fi.targets )
    {
        CMakeTargetItem* targetItem = new CMakeTargetItem( item->project(), ti, folder );
        foreach( QString sFile, ti.sources )
        {
            KUrl sourceFile = folder->url();
            sourceFile.adjustPath( KUrl::AddTrailingSlash );
            sourceFile.addPath( sFile );
            new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
        }
    }


    return folderList;
}

KDevelop::ProjectItem* CMakeImporter::import( KDevelop::IProject *project )
{
    QString buildDir = CMakeSettings::self()->buildFolder();
    kDebug( 9025 ) << k_funcinfo << "build dir is " << qPrintable( buildDir ) << endl;
    KUrl cmakeInfoFile(buildDir);
    cmakeInfoFile.adjustPath( KUrl::AddTrailingSlash );
    cmakeInfoFile.addPath("cmakeinfo.xml");
    kDebug(9025) << k_funcinfo << "file is " << cmakeInfoFile.path() << endl;
    if ( !cmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. CMake support doesn't handle remote projects" << endl;
    }
    else
    {
        m_projectInfo = m_xmlParser.parse( cmakeInfoFile );
        FolderInfo rootFolder = m_projectInfo.rootFolder;
        m_rootItem = new CMakeFolderItem( project, rootFolder, 0 );
        m_rootItem->setText( project->name() );
    }
    return m_rootItem;
}

KUrl CMakeImporter::findMakefile( KDevelop::ProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl();
}

KUrl::List CMakeImporter::findMakefiles( KDevelop::ProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl::List();
}

QList<KDevelop::ProjectTargetItem*> CMakeImporter::targets() const
{
    return QList<KDevelop::ProjectTargetItem*>();
}

KUrl::List CMakeImporter::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return m_includeDirList;
}

QStringList CMakeImporter::extensions() const
{
    return QStringList() << "IBuildSystemManager" << "IProjectFileManager";
}

void CMakeImporter::registerExtensions()
{
    extensionManager()->registerExtensions( new CMakeImporterIProjectFileManagerFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IProjectFileManager ) );
    extensionManager()->registerExtensions( new CMakeImporterIBuildSystemManagerFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IBuildSystemManager ) );
}
void CMakeImporter::unregisterExtensions()
{
    extensionManager()->unregisterExtensions( new CMakeImporterIProjectFileManagerFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IProjectFileManager ) );
    extensionManager()->unregisterExtensions( new CMakeImporterIBuildSystemManagerFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IBuildSystemManager ) );
}

#include "cmakeimporter.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
