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
#include <iplugincontroller.h>
#include "kgenericfactory.h"
#include <projectmodel.h>

#include "cmakeconfig.h"
#include "cmakemodelitems.h"

#include "cmakeastvisitor.h"
#include "cmakeprojectvisitor.h"
#include "cmakeexport.h"
#include "icmakebuilder.h"

typedef KGenericFactory<CMakeProjectManager> CMakeSupportFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevcmakemanager, CMakeSupportFactory( "kdevcmakemanager" ) )

CMakeProjectManager::CMakeProjectManager( QObject* parent, const QStringList& )
    : KDevelop::IPlugin( CMakeSupportFactory::componentData(), parent ), m_rootItem(0L)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.ICMakeBuilder" );
    Q_ASSERT(i);
    if( i )
    {
        m_builder = i->extension<ICMakeBuilder>();
    }
}

CMakeProjectManager::~CMakeProjectManager()
{
    //delete m_rootItem;
}

// KDevelop::IProject* CMakeProjectManager::project() const
// {
//     return m_project;
// }

QStringList CMakeProjectManager::resolveVariables(const QStringList & vars)
{
    return CMakeProjectVisitor::resolveVariables(vars, &m_vars);
}

KUrl CMakeProjectManager::buildDirectory(KDevelop::ProjectItem *item) const
{
    Q_ASSERT(dynamic_cast<KDevelop::ProjectItem*>(item));
    KSharedConfig::Ptr cfg = item->project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    KUrl path = group.readEntry("Build Dir");
//     KUrl projectPath = item->project()->folder();
    
    kDebug(9032) << "Build folder: " << path;
    return path;
}

QList<KDevelop::ProjectFolderItem*> CMakeProjectManager::parse( KDevelop::ProjectFolderItem* item )
{
    QList<KDevelop::ProjectFolderItem*> folderList;
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );

    if ( !folder )
        return folderList;

    kDebug(9032) << "parse:" << folder->url();
    KUrl cmakeListsPath(folder->url());
    cmakeListsPath.addPath("CMakeLists.txt");
    
    CMakeFileContent f = CMakeListsParser::readCMakeFile(cmakeListsPath.toLocalFile());
    
    if(f.isEmpty())
    {
        kDebug(9032) << "There is no" << cmakeListsPath;
        return folderList;
    }
    
    new KDevelop::ProjectFileItem( item->project(), cmakeListsPath, folder );
    kDebug(9032) << "Adding cmake: " << cmakeListsPath << " to the model";
    
    m_vars.insert("CMAKE_CURRENT_LIST_FILE", QStringList(cmakeListsPath.toLocalFile()));
    m_vars.insert("CMAKE_CURRENT_SOURCE_DIR", QStringList(cmakeListsPath.upUrl().toLocalFile()));
    CMakeProjectVisitor v(folder->url().toLocalFile());
    v.setVariableMap(&m_vars);
    v.setMacroMap(&m_macros);
    v.walk(f, 0);
    m_vars.remove("CMAKE_CURRENT_LIST_FILE");
    m_vars.remove("CMAKE_CURRENT_SOURCE_DIR");

    if(folder->text()=="/")
    {
        folder->setText(v.projectName());
    }
    
    foreach ( QString subf, v.subdirectories() )
    {
        KUrl path(folder->url());
        path.addPath(subf);

        CMakeFolderItem* a = new CMakeFolderItem( item->project(), subf, folder );
        a->setUrl(path);
        folderList.append( a );
    }

    KUrl::List directories;
    foreach(QString s, v.includeDirectories())
    {
        KUrl path;
        if(s.startsWith('/'))
        {
            path=s;
        }
        else
        {
            path=folder->url();
            path.addPath(s);
        }
        directories.append(path);
    }
    folder->setIncludeList(directories);

    foreach ( QString t, v.targets())
    {
        CMakeTargetItem* targetItem = new CMakeTargetItem( item->project(), t, folder );

        foreach( QString sFile, v.targetDependencies(t) )
        {
            KUrl sourceFile = folder->url();
            sourceFile.adjustPath( KUrl::AddTrailingSlash );
            sourceFile.addPath( sFile );
            new KDevelop::ProjectFileItem( item->project(), sourceFile, targetItem );
            kDebug(9032) << "..........Adding:" << sFile;
        }
        m_targets.append(targetItem);
    }
    return folderList;
}

KDevelop::ProjectItem* CMakeProjectManager::import( KDevelop::IProject *project )
{
    KUrl cmakeInfoFile(project->projectFileUrl());
    cmakeInfoFile = cmakeInfoFile.upUrl();
    QString folderUrl(cmakeInfoFile.toLocalFile());
    cmakeInfoFile.addPath("CMakeLists.txt");

    kDebug(9025) << "file is" << cmakeInfoFile.path();
    if ( !cmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. CMake support doesn't handle remote projects" ;
    }
    else
    {
        m_vars.insert("CMAKE_SOURCE_DIR", QStringList(cmakeInfoFile.upUrl().toLocalFile()));
        m_rootItem = new CMakeFolderItem(project, "/", 0 );
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
    return m_targets;
}

KUrl::List CMakeProjectManager::includeDirectories(KDevelop::ProjectBaseItem *item) const
{
    CMakeFolderItem* folder = dynamic_cast<CMakeFolderItem*>( item );
    while(!folder && folder->parent()!=0)
    {
        folder = dynamic_cast<CMakeFolderItem*>( item );
    }

    if(!folder)
        return KUrl::List();
    
    kDebug(9032) << "Include directories!" << folder->includeDirectories();
    return folder->includeDirectories();
}

KDevelop::IProjectBuilder * CMakeProjectManager::builder(KDevelop::ProjectItem *) const
{
    return m_builder;
}


#include "cmakemanager.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
