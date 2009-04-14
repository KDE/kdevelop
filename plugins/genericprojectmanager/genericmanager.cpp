/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>

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
#include "genericmanager.h"
#include <project/projectmodel.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/duchain/indexedstring.h>
#include <project/importprojectjob.h>

#include <kdebug.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kaboutdata.h>
#include <kdirwatch.h>

#include <QtCore/QDir>
#include <QtDesigner/QExtensionFactory>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>

K_PLUGIN_FACTORY(GenericSupportFactory, registerPlugin<GenericProjectManager>(); )
K_EXPORT_PLUGIN(GenericSupportFactory(KAboutData("kdevgenericmanager","kdevgenericprojectmanager",ki18n("Generic Project Manager"), "0.1", ki18n("A plugin to support basic project management on a filesystem level"), KAboutData::License_GPL)))

class GenericProjectManagerPrivate
{
};

GenericProjectManager::GenericProjectManager( QObject *parent, const QVariantList & args )
        : KDevelop::IPlugin( GenericSupportFactory::componentData(), parent ), KDevelop::IProjectFileManager(), d( new GenericProjectManagerPrivate )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    Q_UNUSED( args )
}

GenericProjectManager::~GenericProjectManager()
{
    delete d;
}

bool GenericProjectManager::isValid( const QFileInfo &fileInfo,
    const QStringList &includes, const QStringList &excludes ) const
{
    QString fileName = fileInfo.fileName();

    bool ok = fileInfo.isDir();
    for ( QStringList::ConstIterator it = includes.begin(); !ok && it != includes.end(); ++it )
    {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( fileName ) )
        {
            ok = true;
        }
    }

    if ( !ok )
        return false;

    for ( QStringList::ConstIterator it = excludes.begin(); it != excludes.end(); ++it )
    {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( fileName ) )
        {
            return false;
        }
    }

    return true;
}

QList<KDevelop::ProjectFolderItem*> GenericProjectManager::parse( KDevelop::ProjectFolderItem *item )
{
    QDir dir( item->url().toLocalFile() );

    QList<KDevelop::ProjectFolderItem*> folder_list;
    QFileInfoList entries = dir.entryInfoList();

    KConfigGroup filtersConfig = item->project()->projectConfiguration()->group("Filters");
    QStringList includes = filtersConfig.readEntry("Includes", QStringList("*"));
    QStringList excludes = filtersConfig.readEntry("Excludes", QStringList());

    for ( int j = 0; j < item->rowCount(); ++j ) {
        if ( item->child(j)->type() == KDevelop::ProjectBaseItem::Folder ) {
            KDevelop::ProjectFolderItem* f = static_cast<KDevelop::ProjectFolderItem*>( item->child(j) );
            QFileInfo fileInfo(f->url().toLocalFile());
            if (!fileInfo.exists() || !isValid( fileInfo, includes, excludes)) {
                item->removeRow(j);
                j--;
            }
        } else if ( item->child(j)->type() == KDevelop::ProjectBaseItem::File ) {
            KDevelop::ProjectFileItem* f = static_cast<KDevelop::ProjectFileItem*>( item->child(j) );
            QFileInfo fileInfo(f->url().toLocalFile());
            if (!fileInfo.exists() || !isValid( fileInfo, includes, excludes)) {
                item->removeRow(j);
                j--;
            }
        }
    }
    for ( int i = 0; i < entries.count(); ++i )
    {
        QFileInfo fileInfo = entries.at( i );

        if ( !isValid( fileInfo, includes, excludes ) )
        {
            //kDebug(9000) << "skip:" << fileInfo.absoluteFilePath();
        }
        else if ( fileInfo.isDir() && fileInfo.fileName() != QLatin1String( "." )
                  && fileInfo.fileName() != QLatin1String( ".." ) )
        {
            KDevelop::ProjectFolderItem *folder = 0;
            for ( int j = 0; j < item->rowCount(); ++j ) {
                if ( item->child(j)->type() == KDevelop::ProjectBaseItem::Folder ) {
                    KDevelop::ProjectFolderItem* f = static_cast<KDevelop::ProjectFolderItem*>( item->child(j) );
                    if ( f->url() == fileInfo.absoluteFilePath() ) {
                        folder = f;
                        break;
                    }
                }
            }
            if ( ! folder ) {
                folder = new KDevelop::ProjectFolderItem( item->project(), KUrl( fileInfo.absoluteFilePath() ), item );
            }
            folder_list.append( folder );
        }
        else if ( fileInfo.isFile() )
        {
            bool found = false;
            for ( int j = 0; j < item->rowCount(); ++j ) {
                if ( item->child(j)->type() == KDevelop::ProjectBaseItem::File ) {
                    KDevelop::ProjectFileItem* f = static_cast<KDevelop::ProjectFileItem*>( item->child(j) );
                    if ( f->url() == fileInfo.absoluteFilePath() ) {
                        found = true;
                        break;
                    }
                }
            }
            if ( ! found ) {
                new KDevelop::ProjectFileItem( item->project(), KUrl( fileInfo.absoluteFilePath() ), item );
                item->project()->addToFileSet( KDevelop::IndexedString( fileInfo.absoluteFilePath() ) );
            }
        }
    }

    m_watchers[item->project()]->addDir(item->url().toLocalFile(), KDirWatch::WatchDirOnly);

    return folder_list;
}


KDevelop::ProjectFolderItem *GenericProjectManager::import( KDevelop::IProject *project )
{
    KDevelop::ProjectFolderItem *projectRoot=new KDevelop::ProjectFolderItem( project, project->folder(), 0 );
    projectRoot->setProjectRoot(true);
    m_watchers[project] = new KDirWatch( project );
    connect(m_watchers[project], SIGNAL(dirty(QString)), this, SLOT(dirty(QString)));
    return projectRoot;
}

bool GenericProjectManager::reload( KDevelop::ProjectBaseItem* item )
{
    KDevelop::ImportProjectJob* importJob = new KDevelop::ImportProjectJob( item, this );
    KDevelop::ICore::self()->runController()->registerJob( importJob );
    return true;
}

void GenericProjectManager::dirty( const QString &fileName )
{
    foreach ( KDevelop::IProject* p, KDevelop::ICore::self()->projectController()->projects() ) {
        foreach ( KDevelop::ProjectFolderItem* item, p->foldersForUrl( KUrl(fileName) ) ) {
            kDebug() << "reloading item" << item->url().toLocalFile();
            parse(item);
        }
    }
}


KDevelop::ProjectFolderItem* GenericProjectManager::addFolder( const KUrl& url,
        KDevelop::ProjectFolderItem * folder )
{
    Q_UNUSED( url )
    Q_UNUSED( folder )
    return 0;
}


KDevelop::ProjectFileItem* GenericProjectManager::addFile( const KUrl& url,
        KDevelop::ProjectFolderItem * folder )
{
    Q_UNUSED( url )
    Q_UNUSED( folder )
    return 0;
}

bool GenericProjectManager::renameFolder( KDevelop::ProjectFolderItem * folder, const KUrl& url )
{
    Q_UNUSED( folder )
    Q_UNUSED( url )
    return false;
}

bool GenericProjectManager::renameFile( KDevelop::ProjectFileItem * file, const KUrl& url )
{
    Q_UNUSED(file)
    Q_UNUSED(url)
    return false;
}

bool GenericProjectManager::removeFolder( KDevelop::ProjectFolderItem * folder )
{
    Q_UNUSED( folder )
    return false;
}

bool GenericProjectManager::removeFile( KDevelop::ProjectFileItem * file )
{
    Q_UNUSED( file )
    return false;
}


#include "genericmanager.moc"
