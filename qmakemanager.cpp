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
#include <interfaces/iproject.h>
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

QMakeProjectManager* QMakeProjectManager::m_self = 0;

QMakeProjectManager* QMakeProjectManager::self()
{
    return m_self;
}

QMakeProjectManager::QMakeProjectManager( QObject* parent, const QVariantList& )
        : AbstractFileManagerPlugin( QMakeSupportFactory::componentData(), parent ),
          IBuildSystemManager(),
          m_builder(0)
{
    Q_ASSERT(!m_self);
    m_self = this;

    KDEV_USE_EXTENSION_INTERFACE( IBuildSystemManager )
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IQMakeBuilder" );
    Q_ASSERT(i);
    m_builder = i->extension<IQMakeBuilder>();
    Q_ASSERT(m_builder);

    connect(this, SIGNAL(folderAdded(KDevelop::ProjectFolderItem*)),
            this, SLOT(slotFolderAdded(KDevelop::ProjectFolderItem*)));
}

QMakeProjectManager::~QMakeProjectManager()
{
    m_self = 0;
}

IProjectFileManager::Features QMakeProjectManager::features() const
{
    return Features(Folders | Targets | Files);
}

bool QMakeProjectManager::isValid( const KUrl& url, const bool isFolder, IProject* /*project*/ ) const
{
    if (isFolder && url.fileName() == ".kdev4" ) {
        return false;
    } else {
        // TODO: filter setup
        return true;
    }
}

KUrl QMakeProjectManager::buildDirectory(ProjectBaseItem* item) const
{
    while ( !item->folder() ) {
        item = item->parent();
        Q_ASSERT(item);
    }
    Q_ASSERT( item && item->folder() );
    if ( QMakeFolderItem* qmakeItem = dynamic_cast<QMakeFolderItem*>( item ) ) {
        return qmakeItem->projectFile()->buildDirectory();
    } else {
        return KUrl();
    }
}

ProjectFolderItem* QMakeProjectManager::createFolderItem( IProject* project, const KUrl& url,
                                                          ProjectBaseItem* parent )
{
    if ( !parent ) {
        return projectRootItem( project, url );
    } else if (ProjectFolderItem* buildFolder = buildFolderItem( project, url, parent )) {
        // child folder in a qmake folder
        return buildFolder;
    } else {
        return AbstractFileManagerPlugin::createFolderItem( project, url, parent );
    }
}

ProjectFolderItem* QMakeProjectManager::projectRootItem( IProject* project, const KUrl& url )
{
    QFileInfo fi( url.toLocalFile() );
    QDir dir( url.toLocalFile() );
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

    KUrl projecturl = url;
    projecturl.adjustPath( KUrl::AddTrailingSlash );
    projecturl.setFileName( projectfile );
    QHash<QString,QString> qmvars = queryQMake( project );
    QMakeMkSpecs* mkspecs = new QMakeMkSpecs( findBasicMkSpec( qmvars["QMAKE_MKSPECS"] ), qmvars );
    mkspecs->read();
    QMakeCache* cache = findQMakeCache( projecturl.toLocalFile() );
    if( cache ) {
        cache->setMkSpecs( mkspecs );
        cache->read();
    }
    QMakeProjectFile* scope = new QMakeProjectFile( projecturl.toLocalFile() );
    scope->setMkSpecs( mkspecs );
    if( cache ) {
        scope->setQMakeCache( cache );
    }
    scope->read();
    kDebug(9024) << "top-level scope with variables:" << scope->variables();
    QMakeFolderItem* item = new QMakeFolderItem( project, scope, project->folder() );
    return item;
}

ProjectFolderItem* QMakeProjectManager::buildFolderItem( IProject* project, const KUrl& url,
                                                         ProjectBaseItem* parent )
{
    QMakeFolderItem* qmakeParent = 0;
    ProjectBaseItem* p = parent;
    while (!qmakeParent && p) {
        qmakeParent = dynamic_cast<QMakeFolderItem*>( p );
        p = p->parent();
    }
    if ( !qmakeParent ) {
        return 0;
    }
    QString path = url.toLocalFile( KUrl::RemoveTrailingSlash );
    if ( !qmakeParent->projectFile()->subProjects().contains( path ) ) {
        return 0;
    }
    QMakeProjectFile* qmscope = new QMakeProjectFile( path );

    const QFileInfo info( path );
    const QDir d = info.dir();
    if( d.exists(".qmake.cache") ) {
        QMakeCache* cache = new QMakeCache( d.canonicalPath()+"/.qmake.cache" );
        cache->setMkSpecs( qmakeParent->projectFile()->mkSpecs() );
        cache->read();
        qmscope->setQMakeCache( cache );
    } else {
        qmscope->setQMakeCache( qmakeParent->projectFile()->qmakeCache() );
    }

    qmscope->setMkSpecs( qmakeParent->projectFile()->mkSpecs() );
    if( qmscope->read() ) {
        //TODO: only on read?
        return new QMakeFolderItem( project, qmscope, url, parent );
    } else {
        delete qmscope;
        return 0;
    }
}

void QMakeProjectManager::slotFolderAdded( ProjectFolderItem* folder )
{
    QMakeFolderItem* qmakeParent = dynamic_cast<QMakeFolderItem*>( folder );
    if ( !qmakeParent ) {
        return;
    }

    kDebug(9024) << "adding targets for" << folder->url();
    foreach( const QString& s, qmakeParent->projectFile()->targets() )
    {
        kDebug(9024) << "adding target:" << s;
        QMakeTargetItem* target = new QMakeTargetItem( folder->project(), s, folder );
        foreach( const KUrl& u, qmakeParent->projectFile()->filesForTarget(s) )
        {
            kDebug(9024) << "adding file:" << u;
            new ProjectFileItem( folder->project(), u, target );
            ///TODO: signal?
        }
    }
}

ProjectFolderItem* QMakeProjectManager::import( IProject* project )
{
    KUrl dirName = project->folder();
    if( !dirName.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. QMake support doesn't handle remote projects";
        return 0;
    }

    return AbstractFileManagerPlugin::import( project );
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

QString QMakeProjectManager::qtIncludeDir() const
{
    if ( !m_qtIncludeDir.isEmpty() ) {
        return m_qtIncludeDir;
    }

    // Let's cache the Qt include dir
    KProcess qtInc;
    qtInc << "qmake" << "-query" << "QT_INSTALL_HEADERS";
    qtInc.setOutputChannelMode( KProcess::OnlyStdoutChannel );
    qtInc.start();
    if ( !qtInc.waitForFinished() )
    {
        kWarning() << "Failed to query Qt header path using qmake, is qmake installed?";
    } else
    {
        QByteArray result = qtInc.readAll();
        m_qtIncludeDir = QString::fromLocal8Bit( result ).trimmed();
    }

    return m_qtIncludeDir;
}

#include "qmakemanager.moc"
