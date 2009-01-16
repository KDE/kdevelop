/* KDevelop CMake Support
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
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

#include "cmakebuilder.h"
#include "imakebuilder.h"

#include <config.h>

#include <QtCore/QStringList>
#include <QtCore/QSignalMapper>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <outputview/ioutputview.h>
#include <outputview/outputmodel.h>
#include <util/commandexecutor.h>
#include <QtDesigner/QExtensionFactory>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kparts/mainwindow.h>
#include <kio/deletejob.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <KProcess>
#include <kjob.h>
#include <kurl.h>
#include <kconfig.h>

#include "cmakejob.h"
#include "../../managers/cmake/settings/cmakebuilddircreator.h"

K_PLUGIN_FACTORY(CMakeBuilderFactory, registerPlugin<CMakeBuilder>(); )
K_EXPORT_PLUGIN(CMakeBuilderFactory(KAboutData("kdevcmakebuilder","kdevcmakebuilder", ki18n("CMake Builder"), "0.1", ki18n("Support for building CMake projects"), KAboutData::License_GPL)))

CMakeBuilder::CMakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(CMakeBuilderFactory::componentData(), parent),
      m_dirty(true), m_builder( 0 )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( ICMakeBuilder )

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if( i )
    {
        m_builder = i->extension<IMakeBuilder>();
        if( m_builder )
        {
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SLOT(buildFinished(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SLOT(buildFinished(KDevelop::ProjectBaseItem*)));
            
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SIGNAL(built(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SIGNAL(failed(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)), this, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(installed(KDevelop::ProjectBaseItem*)), this, SIGNAL(installed(KDevelop::ProjectBaseItem*)));
        }
    }
}

CMakeBuilder::~CMakeBuilder()
{
}

void CMakeBuilder::buildFinished(KDevelop::ProjectBaseItem* it)
{
    if(m_deleteWhenDone.remove(it)) {
        delete it;
    }
}

bool buildDirConfigured( KDevelop::ProjectBaseItem* item )
{
    KConfigGroup cmakeGrp = item->project()->projectConfiguration()->group("CMake");
    KUrl builddir = cmakeGrp.readEntry( "currentBuildDir", KUrl() );
    
    if( !builddir.isValid() || builddir.isEmpty() ) 
    {
        CMakeBuildDirCreator bd( item->project()->projectItem()->url(), KDevelop::ICore::self()->uiController()->activeMainWindow() );
        if( !bd.exec() )
        {
            return false;
        }
        cmakeGrp.writeEntry( "currentBuildDir", bd.buildFolder() );
    }  
    return true;
}

KJob* CMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::ProjectBaseItem* builditem = dom;
    if( m_builder )
    {
        if(dom->file())
        {
            KDevelop::ProjectFileItem* file = dom->file();
            int lastDot = file->text().lastIndexOf('.');
            QString target = file->text().mid(0, lastDot)+".o";
             
            KDevelop::ProjectBuildFolderItem *fldr = new KDevelop::ProjectBuildFolderItem(dom->project(), file->url().upUrl());
            KDevelop::ProjectTargetItem *it = new KDevelop::ProjectTargetItem(dom->project(), target);
            fldr->add(it);
             
            builditem=it;
            m_deleteWhenDone << fldr << it;
        }
        
        if( !buildDirConfigured(builditem) )
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("No Build Directory configured, cannot build"), i18n("Aborting build") );
            return 0;
        }
        
        kDebug(9032) << "Building with make";
        return m_builder->build(builditem);
    }
    return false;
}

KJob* CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::ProjectBaseItem* item = dom;
    if( m_builder )
    {
        if(dom->file()) //It doesn't work to compile a file
            item=(KDevelop::ProjectBaseItem*) dom->parent();
        
        kDebug(9032) << "Cleaning with make";
        if( !buildDirConfigured(item) )
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("No Build Directory configured, cannot clean"), i18n("Aborting clean") );
            return 0;
        }
        return m_builder->clean(item);
    }
    return 0;
}

KJob* CMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::ProjectBaseItem* item = dom;
    if( m_builder )
    {
        if(dom->file())
            item=(KDevelop::ProjectBaseItem*) dom->parent();
        
        kDebug(9032) << "Installing with make";
        if( !buildDirConfigured(item) )
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("No Build Directory configured, cannot install"), i18n("Aborting install") );
            return 0;
        }
        return m_builder->install(dom);
    }
    return 0;
}

KJob* CMakeBuilder::configure( KDevelop::IProject* project )
{
    if( !buildDirConfigured( project->projectItem() ) )
    {
        KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("No Build Directory configured, cannot configure"), i18n("Aborting configure") );
        return 0;
    }
    CMakeJob* job = new CMakeJob(this);
    job->setProject(project);
    return job;
}

KJob* CMakeBuilder::prune( KDevelop::IProject* project )
{
    if( !buildDirConfigured( project->projectItem() ) )
    {
        KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("No Build Directory configured, cannot clear builddir"), i18n("Aborting builddir clearing") );
        return 0;
    }
    KConfigGroup grp = project->projectConfiguration()->group("CMake");
    KUrl builddir = grp.readEntry( "currentBuildDir", KUrl() );
    QDir d( builddir.toLocalFile() );
    KUrl::List urls;
    foreach( const QString& entry, d.entryList( QDir::NoDotAndDotDot ) )
    {
        KUrl tmp = builddir;
        tmp.addPath( entry );
        urls << tmp;
    }
    return KIO::del( urls );
}

#include "cmakebuilder.moc"
