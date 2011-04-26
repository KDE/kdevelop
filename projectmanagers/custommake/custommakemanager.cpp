/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "custommakemanager.h"
#include "custommakemodelitems.h"
#include "custommaketreesynchronizer.h"
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include "imakebuilder.h"
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <project/projectmodel.h>
#include <project/helper.h>
#include <language/duchain/indexedstring.h>

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QApplication>

#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(CustomMakeSupportFactory, registerPlugin<CustomMakeManager>(); )
K_EXPORT_PLUGIN(CustomMakeSupportFactory(KAboutData("kdevcustommakemanager","kdevcustommake", ki18n("Custom Makefile Manager"), "0.1", ki18n("Support for managing custom makefile projects"), KAboutData::License_GPL)))

class CustomMakeManager::Private
{
public:
    Private() : m_builder(0) {}

    IMakeBuilder *m_builder;

//     QList< KDevelop::ProjectBaseItem* > m_testItems; // for debug
};

CustomMakeManager::CustomMakeManager( QObject *parent, const QVariantList& args )
    : KDevelop::IPlugin( CustomMakeSupportFactory::componentData(), parent )
    , d( new Private )
{
    Q_UNUSED(args)
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )

    setXMLFile( "kdevcustommakemanager.rc" );

    // TODO use CustomMakeBuilder
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IMakeBuilder" );
    Q_ASSERT(i);
    d->m_builder = i->extension<IMakeBuilder>();
    Q_ASSERT(d->m_builder);
}

CustomMakeManager::~CustomMakeManager()
{
    delete d;
}

IProjectBuilder* CustomMakeManager::builder(KDevelop::ProjectFolderItem*) const
{
    Q_ASSERT(d->m_builder);
    return d->m_builder;
}

KUrl::List CustomMakeManager::includeDirectories(KDevelop::ProjectBaseItem*) const
{
    return KUrl::List();
}

QHash<QString,QString> CustomMakeManager::defines(KDevelop::ProjectBaseItem*) const
{
    return QHash<QString,QString>();
}

ProjectTargetItem* CustomMakeManager::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED(target)
    Q_UNUSED(parent)
    return NULL;
}

bool CustomMakeManager::addFilesToTarget(QList< ProjectFileItem* > files, ProjectTargetItem* parent)
{
    Q_UNUSED( files )
    Q_UNUSED( parent )
    return false;
}

bool CustomMakeManager::removeTarget(KDevelop::ProjectTargetItem *target)
{
    Q_UNUSED( target )
    return false;
}

bool CustomMakeManager::removeFilesFromTargets(QList< ProjectFileItem* > targetFiles)
{
    Q_UNUSED( targetFiles )
    return false;
}

KUrl CustomMakeManager::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    ProjectFolderItem *fi=dynamic_cast<ProjectFolderItem*>(item);
    for(; !fi && item; )
    {
        item=dynamic_cast<ProjectBaseItem*>(item->parent());
        fi=dynamic_cast<ProjectFolderItem*>(item);
    }
    if(!fi) {
        return item->project()->folder();
    }
    return fi->url();
}

QList<ProjectTargetItem*> CustomMakeManager::targets(KDevelop::ProjectFolderItem*) const
{
    QList<ProjectTargetItem*> ret;
    return ret;
}

QList<ProjectFolderItem*> CustomMakeManager::parse(KDevelop::ProjectFolderItem *item)
{
    QList<KDevelop::ProjectFolderItem*> folder_list;
    QDir dir( item->url().toLocalFile() );

    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    CustomMakeFolderItem *topItem = dynamic_cast<CustomMakeFolderItem*>( item->project()->projectItem() );

    foreach ( const QFileInfo& fileInfo, entries )
    {
        QString fileName = fileInfo.fileName();
        QString absFilePath = fileInfo.absoluteFilePath();
        
        if ( fileInfo.isDir() )
        {
            //TODO: make filtering generic
            if ( fileName == QLatin1String(".svn") || fileName == QLatin1String("CVS") || fileName == QLatin1String(".git") || fileName == QLatin1String(".bzr") || fileName == QLatin1String(".hg") || fileName == QLatin1String("_darcs") )
            {
                continue;
            }
//             KDevelop::ProjectFolderItem *cmfi= new KDevelop::ProjectFolderItem(
//                     item->project(), KUrl( fileInfo.absoluteFilePath() ), item );
            // TODO more faster algorithm. should determine whether this directory
            // contains makefile or not.
            KDevelop::ProjectBuildFolderItem *cmfi = new KDevelop::ProjectBuildFolderItem(
                    item->project(), KUrl( absFilePath ), item );
            folder_list.append( cmfi );
//             d->m_testItems.append( cmfi ); // debug
            if( topItem )
                topItem->fsWatcher()->addDirectory( cmfi );
        }
        //TODO: make filtering generic
        else if ( fileInfo.isFile() && !fileName.endsWith('~') && !fileName.endsWith(".o") )
        {
            KUrl fileUrl( absFilePath );
            KDevelop::ProjectFileItem *fileItem =
                new KDevelop::ProjectFileItem( item->project(), fileUrl, item );
            if( topItem && fileName == "Makefile" )
            {
                topItem->fsWatcher()->addFile( fileItem );
                QStringList targetlist = this->parseCustomMakeFile( fileUrl );
                foreach( const QString &target, targetlist )
                {
                    new CustomMakeTargetItem( item->project(), target, item );
    //             d->m_testItems.append( targetItem ); // debug
                }
            }
        }
    }
    // find makefile, parse and get the target list
//     KUrl makefileUrl = this->findMakefile( item );
//     if( makefileUrl.isValid() )
//     {
//         QStringList targetlist = this->parseCustomMakeFile( makefileUrl );
//         foreach(const  QString& target, targetlist )
//         {
//             new CustomMakeTargetItem( item->project(), target, item );
// //             d->m_testItems.append( targetItem ); // debug
//         }
//         if( topItem )
//             topItem->fsWatcher()->addFile( makefileUrl.toLocalFile() );
//     }

    return folder_list;
}

KDevelop::ProjectFolderItem* CustomMakeManager::import(KDevelop::IProject *project)
{
    if( !project ) return NULL;
//     return new KDevelop::ProjectFolderItem( project, project->folder().pathOrUrl(), NULL );
    CustomMakeFolderItem *item = new CustomMakeFolderItem( this, project, project->folder(), NULL );
    item->fsWatcher()->addDirectory( item );

    return item;
}

ProjectFolderItem* CustomMakeManager::addFolder(const KUrl& folder, KDevelop::ProjectFolderItem *parent)
{
    KDevelop::createFolder(folder);
    Q_UNUSED(parent);
    return 0;
}

ProjectFileItem* CustomMakeManager::addFile(const KUrl& file, KDevelop::ProjectFolderItem *parent)
{
    KDevelop::createFile(file);
    Q_UNUSED(parent);
    return 0;
}

bool CustomMakeManager::removeFilesAndFolders(QList<KDevelop::ProjectBaseItem*> items)
{
    foreach(KDevelop::ProjectBaseItem* item, items)
    {
        Q_ASSERT(item->folder() || item->file());
        Q_ASSERT(!item->file() || !item->file()->parent()->target());

        if (!KDevelop::removeUrl(item->project(), item->url(), false))
            return false;
    }
    return true;
}

bool CustomMakeManager::renameFile(KDevelop::ProjectFileItem* oldFile, const KUrl& newFile)
{
    CustomMakeFolderItem* p = dynamic_cast<CustomMakeFolderItem*>(oldFile->project()->projectItem());
    Q_ASSERT(p);
    p->fsWatcher()->stopWatcher();
    bool success = KDevelop::renameUrl( oldFile->project(), oldFile->url(), newFile );
    if (success) {
        foreach(KDevelop::ProjectFolderItem* folder, oldFile->project()->foldersForUrl(newFile.upUrl())) {
            if (folder != oldFile->parent()) {
                oldFile->parent()->takeRow(oldFile->row());
                folder->appendRow(oldFile);
            }
            break;
        }
    }
    p->fsWatcher()->continueWatcher();
    return success;
}

bool CustomMakeManager::renameFolder(KDevelop::ProjectFolderItem* oldFolder, const KUrl& newFolder )
{
    return KDevelop::renameUrl( oldFolder->project(), oldFolder->url(), newFolder );
}

bool CustomMakeManager::moveFilesAndFolders(QList< ProjectBaseItem* > items, ProjectFolderItem* newParent)
{
    Q_UNUSED(items);
    Q_UNUSED(newParent);
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// private slots

///TODO: move to background thread, probably best would be to use a proper ParseJob
QStringList CustomMakeManager::parseCustomMakeFile( const KUrl &makefile )
{
    if( !makefile.isValid() )
        return QStringList();

    QStringList ret; // the list of targets
//     KUrl absFileUrl = dir;
    // TODO support Makefile, Makefile.xxx, makefile
//     absFileUrl.addPath( "Makefile" );
    QFile f( makefile.toLocalFile() );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        kDebug(9025) << "could not open" << makefile;
        return ret;
    }

    QRegExp targetRe( "^ *([^\\t$.#]\\S+) *:.*$" );
    targetRe.setMinimal( true );

    QString str;
    QTextStream stream( &f );
    while ( !stream.atEnd() )
    {
        str = stream.readLine();

        if ( targetRe.indexIn( str ) != -1 )
        {
            QString tmpTarget = targetRe.cap( 1 ).simplified();
//             if ( tmpTarget.endsWith( ".o" ) )
//             {
//                 if ( ! ret.contains(tmpTarget) )
//                     ret.append( tmpTarget );
//             }
//             else if ( tmpTarget.contains( '.' ) )
//             {
//                 if ( ! ret.contains(tmpTarget) )
//                     ret.append( tmpTarget );
//             }
//             else
//             {
            if ( ! ret.contains( tmpTarget ) )
                ret.append( tmpTarget );
//             }
        }
    }
    f.close();
    return ret;
}

#include "custommakemanager.moc"

