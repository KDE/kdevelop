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

#include "cmakemanager.h"

#include <QList>
#include <QVector>
#include <QDomDocument>

#include <QtDesigner/QExtensionFactory>

#include <KUrl>
#include <kio/job.h>

#include <icore.h>
#include <iproject.h>
#include "kgenericfactory.h"
#include <projectmodel.h>

#include "cmakeconfig.h"
#include "cmakemodelitems.h"

#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"

typedef KGenericFactory<CMakeProjectManager> CMakeSupportFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevcmakemanager,
                            CMakeSupportFactory( "kdevcmakemanager" ) )

CMakeProjectManager::CMakeProjectManager( QObject* parent,
                              const QStringList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent ), m_rootItem(0L)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
/*    CMakeSettings* settings = CMakeSettings::self();

    //what do the settings say about our generator?
    QString generator = settings->generator();
    if ( generator.contains( "Unix" ) ) //use make
        m_builder = new KDevMakeBuilder()*/
}

CMakeProjectManager::~CMakeProjectManager()
{
    //delete m_rootItem;
}

// KDevelop::IProject* CMakeProjectManager::project() const
// {
//     return m_project;
// }

KUrl CMakeProjectManager::buildDirectory(KDevelop::ProjectItem *item) const
{
    return item->project()->folder();
}

QList<KDevelop::ProjectFolderItem*> CMakeProjectManager::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    if ( !folder )
        return folderList;

    kDebug(9025) << "parse: " << folder->url() << endl;
    CMakeAst *node = folder->tree();
    CMakeProjectVisitor v;
    node->accept(&v);
//     FolderInfo fi = folder->folderInfo();
//     for ( QStringList::iterator it = fi.includes.begin();
//           it != fi.includes.end(); ++it )
//     {
//         KUrl urlCandidate = KUrl( ( *it ) );
//         if ( m_includeDirList.indexOf( urlCandidate ) == -1 )
//             m_includeDirList.append( urlCandidate );
//     }

    foreach ( QString subf, v.subdirectories() ) {
        CMakeAst *newFolder = new CMakeAst;
        KUrl path(folder->url());
        path.addPath(subf);
        KUrl cmakelistsPath(path);
        cmakelistsPath.addPath("CMakeLists.txt");
        if ( !CMakeListsParser::parseCMakeFile( newFolder, cmakelistsPath.toLocalFile() ) )
        {
            kDebug(9032) << "Subfolder: " << path.toLocalFile() << endl;
            CMakeFolderItem* a = new CMakeFolderItem( item->project(), subf, newFolder, folder );
            a->setUrl(path);
            folderList.append( a );
        } else {
            //FIXME: Put here the error.
            kDebug(9032) << "Parsing error." << endl;
            return folderList;
        }
    }

//     foreach ( TargetInfo ti, fi.targets )
//     {
//         CMakeTargetItem* targetItem = new CMakeTargetItem( item->project(), ti, folder );
//         foreach( QString sFile, ti.sources )
//         {
//             KUrl sourceFile = folder->url();
//             sourceFile.adjustPath( KUrl::AddTrailingSlash );
//             sourceFile.addPath( sFile );
//             new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
//         }
//     }


     return folderList;
}

KDevelop::ProjectItem* CMakeProjectManager::import( KDevelop::IProject *project )
{
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    cmakeInfoFile.addPath("CMakeLists.txt");
    kDebug(9025) << k_funcinfo << "file is " << cmakeInfoFile.path() << endl;
    if ( !cmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. CMake support doesn't handle remote projects" << endl;
    }
    else
    {
        CMakeAst *rootFolder = new CMakeAst;
        QString name;

        kDebug(9032) << "Parsing file: " << cmakeInfoFile.path() << endl;

        if ( !CMakeListsParser::parseCMakeFile( rootFolder, cmakeInfoFile.toLocalFile() ) )
        {
            CMakeProjectVisitor v;
            rootFolder->accept(&v);

            name = v.projectName();
        } else {
            //FIXME: Put here the error.
            kDebug(9032) << "Parsing error." << endl;
        }

        m_rootItem = new CMakeFolderItem(project, "/", rootFolder, 0 );
        m_rootItem->setText(name);
//         m_rootItem->setUrl(project->projectFileUrl().upUrl());
    }
    return m_rootItem;
}

KUrl CMakeProjectManager::findMakefile( KDevelop::ProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl();
}

KUrl::List CMakeProjectManager::findMakefiles( KDevelop::ProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl::List();
}

QList<KDevelop::ProjectTargetItem*> CMakeProjectManager::targets() const
{
    return QList<KDevelop::ProjectTargetItem*>();
}

KUrl::List CMakeProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    return m_includeDirList;
}

#include "cmakemanager.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
