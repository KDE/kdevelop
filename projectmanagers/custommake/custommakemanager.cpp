/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 * Copyright 2011 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "custommakemanager.h"
#include "custommakemodelitems.h"
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
#include <KDirWatch>

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
    : KDevelop::AbstractFileManagerPlugin( CustomMakeSupportFactory::componentData(), parent )
    , d( new Private )
{
    Q_UNUSED(args)
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )

    setXMLFile( "kdevcustommakemanager.rc" );

    // TODO use CustomMakeBuilder
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IMakeBuilder" );
    Q_ASSERT(i);
    d->m_builder = i->extension<IMakeBuilder>();
    Q_ASSERT(d->m_builder);

    connect(this, SIGNAL(reloadedFileItem(KDevelop::ProjectFileItem*)),
            this, SLOT(reloadMakefile(KDevelop::ProjectFileItem*)));
}

CustomMakeManager::~CustomMakeManager()
{
    delete d;
}

IProjectBuilder* CustomMakeManager::builder() const
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

bool CustomMakeManager::addFilesToTarget(const QList< ProjectFileItem* > &files, ProjectTargetItem* parent)
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

bool CustomMakeManager::removeFilesFromTargets(const QList< ProjectFileItem* > &targetFiles)
{
    Q_UNUSED( targetFiles )
    return false;
}

KUrl CustomMakeManager::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    ProjectFolderItem *fi=dynamic_cast<ProjectFolderItem*>(item);
    for(; !fi && item; )
    {
        item=item->parent();
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

static bool isMakefile(const QString& fileName)
{
    return  ( fileName == QLatin1String("Makefile")
        || fileName == QLatin1String("makefile")
        || fileName == QLatin1String("GNUmakefile")
        || fileName == QLatin1String("BSDmakefile") );
}

void CustomMakeManager::createTargetItems(IProject* project, const KUrl& url, ProjectBaseItem* parent)
{
    Q_ASSERT(isMakefile(url.fileName()));
    foreach(const QString& target, parseCustomMakeFile( url ))
    {
        if(!isValid(KUrl(parent->url(), target), false, project)){
            continue;
        }
        new CustomMakeTargetItem( project, target, parent );
//         d->m_testItems.append( targetItem ); // debug
    }
}

ProjectFileItem* CustomMakeManager::createFileItem(IProject* project, const KUrl& url, ProjectBaseItem* parent)
{
    ProjectFileItem* item = new ProjectFileItem(project, url, parent);
    if (isMakefile(url.fileName())){
        createTargetItems(project, url, parent);
    }
    return item;
}

void CustomMakeManager::reloadMakefile(ProjectFileItem* file)
{
    if( !isMakefile(file->url().fileName())){
        return;
    }
    ProjectBaseItem* parent = file->parent();
    // remove the items that are Makefile targets
    foreach(ProjectBaseItem* item, parent->children()){
        if (item->target()){
            delete item;
        }
    }
    // Recreate the targets.
    createTargetItems(parent->project(), file->url(), parent);
}

ProjectFolderItem* CustomMakeManager::createFolderItem(IProject* project, const KUrl& url, ProjectBaseItem* parent)
{
    // TODO more faster algorithm. should determine whether this directory
    // contains makefile or not.
    return new KDevelop::ProjectBuildFolderItem( project, url, parent );
}

KDevelop::ProjectFolderItem* CustomMakeManager::import(KDevelop::IProject *project)
{
    KUrl dirName = project->folder();
    if( !dirName.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. Custom make support doesn't handle remote projects";
        return 0;
    }

    return AbstractFileManagerPlugin::import( project );
}

/////////////////////////////////////////////////////////////////////////////
// private slots

///TODO: move to background thread, probably best would be to use a proper ParseJob
QStringList CustomMakeManager::parseCustomMakeFile( const KUrl &makefile )
{
    if( !makefile.isValid() )
        return QStringList();

    QStringList ret; // the list of targets
    QFile f( makefile.toLocalFile() );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        kDebug(9025) << "could not open" << makefile;
        return ret;
    }

    QRegExp targetRe( "^ *([^\\t$.#]\\S+) *:(?!=).*$" );
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

