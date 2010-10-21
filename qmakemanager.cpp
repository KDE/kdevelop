/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#include "qmakemanager.h"
#include <QList>
#include <QVector>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QHash>

#include <kurl.h>
#include <kio/job.h>
#include <kprocess.h>
#include <kdebug.h>

#include <interfaces/icore.h>
#include <language/duchain/indexedstring.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include "iqmakebuilder.h"
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <project/projectmodel.h>

#include "qmakemodelitems.h"
#include "qmakeprojectfile.h"
#include "qmakecache.h"
#include "qmakemkspecs.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(QMakeSupportFactory, registerPlugin<QMakeProjectManager>(); )
K_EXPORT_PLUGIN(QMakeSupportFactory(KAboutData("kdevqmakemanager","kdevqmake", ki18n("QMake Manager"), "0.1", ki18n("Support for managing QMake projects"), KAboutData::License_GPL)))

QMakeProjectManager::QMakeProjectManager( QObject* parent,
                              const QVariantList& )
        : IPlugin( QMakeSupportFactory::componentData(), parent ), m_builder(0)
{
    KDEV_USE_EXTENSION_INTERFACE( IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( IBuildSystemManager )
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IQMakeBuilder" );
    Q_ASSERT(i);
    m_builder = i->extension<IQMakeBuilder>();
    Q_ASSERT(m_builder);
}

QMakeProjectManager::~QMakeProjectManager()
{

}

KUrl QMakeProjectManager::buildDirectory(ProjectBaseItem* project) const
{
    if( project->folder() )
        return project->folder()->url();
    else if( project->parent() )
    {
        ProjectBaseItem* base = static_cast<ProjectBaseItem*>(project->parent());
        if( base->type() == ProjectBaseItem::Target )
        {
            return static_cast<ProjectFolderItem*>(base->parent())->url();
        }else
        {
            return static_cast<ProjectFolderItem*>(base)->url();
        }
    }
    return KUrl();
}

QList<ProjectFolderItem*> QMakeProjectManager::parse( ProjectFolderItem* item )
{
    QList<ProjectFolderItem*> folderList;

    kDebug(9024) << "Parsing item:";

    QMakeFolderItem* folderitem = dynamic_cast<QMakeFolderItem*>( item );
    QStringList entries = QDir( item->url().toLocalFile() ).entryList( QDir::AllEntries | QDir::Hidden | QDir::System );

    entries.removeAll(".");
    entries.removeAll("..");

    if( folderitem )
    {
        kDebug(9024) << "Item is a qmakefolder:";

        foreach( QMakeProjectFile* subproject, folderitem->projectFile()->subProjects() )
        {
            kDebug(9024) << "adding subproject:" << subproject->absoluteDir();
            if( entries.contains( KUrl(subproject->absoluteFile()).fileName() ) )
            {
                entries.removeAll( KUrl(subproject->absoluteFile()).fileName() );
            }
            folderList.append( new QMakeFolderItem( item->project(),
                               subproject,
                               KUrl( subproject->absoluteDir() ),
                               item ) );
        }
        foreach( const QString& s, folderitem->projectFile()->targets() )
        {
            kDebug(9024) << "adding target:" << s;
            QMakeTargetItem* target = new QMakeTargetItem( item->project(), s,  item );
            foreach( const KUrl& u, folderitem->projectFile()->filesForTarget(s) )
            {
                if( entries.contains( u.fileName() ) )
                {
                    entries.removeAll( u.fileName() );
                }
                kDebug(9024) << "adding file:" << u;
                new ProjectFileItem( item->project(), u, target );
                new ProjectFileItem( item->project(), u, folderitem );
                item->project()->addToFileSet( IndexedString( u ) );
            }
        }
    }

    foreach( const QString& entry, entries )
    {
        if( item->hasFileOrFolder( entry ) )
            continue;

        KUrl folderurl = item->url();
        folderurl.addPath( entry );
        if( QFileInfo( folderurl.toLocalFile() ).isDir() )
        {
            new ProjectFolderItem( item->project(), folderurl, item );
        }else
        {
            new ProjectFileItem( item->project(), folderurl, item );
        }
    }

//     kDebug(9024) << "adding project file:" << folderitem->projectFile()->absoluteFile();
//     new ProjectFileItem( item->project(),
//                                    KUrl( folderitem->projectFile()->absoluteFile() ),
//                                    item );
    kDebug(9024) << "Added" << folderList.count() << "Elements";


    return folderList;
}

ProjectFolderItem* QMakeProjectManager::import( IProject* project )
{
    KUrl dirName = project->folder();
    if( !dirName.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. QMake support doesn't handle remote projects";
    }else
    {
        QFileInfo fi( dirName.toLocalFile() );
        QDir dir( dirName.toLocalFile() );
        QStringList l = dir.entryList( QStringList() << "*.pro" );

        QString projectfile;

        if( l.count() && l.indexOf( project->name() + ".pro") != -1 )
            projectfile = project->name() + ".pro";
        if( l.isEmpty() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
        {
            projectfile = fi.baseName() + ".pro";
        }else
        {
            projectfile = l.first();
        }

        KUrl projecturl = dirName;
        projecturl.adjustPath( KUrl::AddTrailingSlash );
        projecturl.setFileName( projectfile );
        QHash<QString,QString> qmvars = queryQMake( project );
        QMakeMkSpecs* mkspecs = new QMakeMkSpecs( findBasicMkSpec( qmvars["QMAKE_MKSPECS"] ), qmvars );
        mkspecs->read();
        QMakeCache* cache = findQMakeCache( projecturl.toLocalFile() );
        if( cache )
        {
            cache->setMkSpecs( mkspecs );
            cache->read();
        }
        QMakeProjectFile* scope = new QMakeProjectFile( projecturl.toLocalFile() );
        scope->setMkSpecs( mkspecs );
	if( cache )
	{
            cache->setMkSpecs( mkspecs );
            cache->read();
            scope->setQMakeCache( cache );
	}
        scope->read();
        kDebug(9024) << "top-level scope with variables:" << scope->variables();
        QMakeFolderItem* item = new QMakeFolderItem( project, scope, project->folder() );
	return item;
    }
    return 0;
}

QList<ProjectTargetItem*> QMakeProjectManager::targets(ProjectFolderItem* item) const
{
    Q_UNUSED(item)
    return QList<ProjectTargetItem*>();
}

IProjectBuilder* QMakeProjectManager::builder(ProjectFolderItem*) const
{
    Q_ASSERT(m_builder);
    return m_builder;
}

KUrl::List QMakeProjectManager::includeDirectories(ProjectBaseItem* item) const
{
    KUrl::List list;
    QMakeFolderItem* folder = 0;

    if( item->type() == ProjectBaseItem::File )
    {
        folder =
                dynamic_cast<QMakeFolderItem*>( item->parent() );
        if( !folder )
        {
            folder =
                dynamic_cast<QMakeFolderItem*>( item->parent()->parent() );
        }
    }else if( item->type() == ProjectBaseItem::Target )
    {
        folder =
                dynamic_cast<QMakeFolderItem*>( item->parent() );
    }else
    {
        folder =
                dynamic_cast<QMakeFolderItem*>( item );
    }
    if( folder )
    {
        list += folder->projectFile()->includeDirectories();
    }
    return list;
}

QString QMakeProjectManager::findBasicMkSpec( const QString& mkspecdir ) const
{
    QFileInfo fi( mkspecdir+"/default/qmake.conf" );
    if( !fi.exists() )
        return QString();

    return fi.absoluteFilePath();
}

QHash<QString,QString> QMakeProjectManager::queryQMake( IProject* project ) const
{
    if( !project->folder().isLocalFile() || !m_builder )
        return QHash<QString,QString>();

    QHash<QString,QString> hash;
    KProcess p;
    QStringList queryVariables;
    queryVariables << "QMAKE_MKSPECS" << "QMAKE_VERSION" <<
            "QT_INSTALL_BINS" << "QT_INSTALL_CONFIGURATION" <<
            "QT_INSTALL_DATA" << "QT_INSTALL_DEMOS" << "QT_INSTALL_DOCS" <<
            "QT_INSTALL_EXAMPLES" << "QT_INSTALL_HEADERS" <<
            "QT_INSTALL_LIBS" << "QT_INSTALL_PLUGINS" << "QT_INSTALL_PREFIX" <<
            "QT_INSTALL_TRANSLATIONS" << "QT_VERSION";
    foreach( const QString& var, queryVariables)
    {
        p.clearProgram();
        p.setOutputChannelMode( KProcess::OnlyStdoutChannel );
        p.setWorkingDirectory( project->folder().toLocalFile() );
        //To be implemented when there's an API to fetch Env from Project
        //p.setEnv();
        p << m_builder->qmakeBinary( project ) << "-query" << var;
        p.execute();
        QString result = QString::fromLocal8Bit( p.readAllStandardOutput() ).trimmed();
        if( result != "**Unknown**")
            hash[var] = result;
    }
    kDebug(9024) << "Ran qmake (" << m_builder->qmakeBinary( project ) << "), found:" << hash;
    return hash;
}

QMakeCache* QMakeProjectManager::findQMakeCache( const QString& projectfile ) const
{
    QDir curdir( QFileInfo( projectfile ).canonicalPath() );
    while( !curdir.exists(".qmake.cache") && !curdir.isRoot() )
    {
        curdir.cdUp();
    }

    if( curdir.exists(".qmake.cache") )
    {
        return new QMakeCache( curdir.canonicalPath()+"/.qmake.cache" );
    }
    return 0;
}

#include "qmakemanager.moc"
