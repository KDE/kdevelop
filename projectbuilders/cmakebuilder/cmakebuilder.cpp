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

#include "configureandbuildjob.h"
#include "cmakejob.h"
#include "cmakeutils.h"
#include <cmakemodelitems.h>

K_PLUGIN_FACTORY(CMakeBuilderFactory, registerPlugin<CMakeBuilder>(); )
K_EXPORT_PLUGIN(CMakeBuilderFactory(KAboutData("kdevcmakebuilder","kdevcmakebuilder", ki18n("CMake Builder"),
                                               "0.1", ki18n("Support for building CMake projects"), KAboutData::License_GPL)))

CMakeBuilder::CMakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(CMakeBuilderFactory::componentData(), parent),
      m_builder( 0 )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if( i )
    {
        m_builder = i->extension<IProjectBuilder>();
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
        delete it->parent();
    }
}

KJob* CMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::ProjectBaseItem* builditem = dom;
    KDevelop::IProject* p = dom->project();
    if( m_builder )
    {
        if(dom->file())
        {
            KDevelop::ProjectFileItem* file = dom->file();
            int lastDot = file->text().lastIndexOf('.');
            QString target = file->text().mid(0, lastDot)+".o";
             
            CMakeFolderItem *fldr = new CMakeFolderItem(p, dom->url().upUrl(), KUrl::relativeUrl(p->folder(), file->url().upUrl()), 0);
            KDevelop::ProjectTargetItem *it = new KDevelop::ProjectTargetItem(p, target);
            fldr->appendRow(it);
             
            builditem=it;
            m_deleteWhenDone << it;
        }
        KJob* configure = 0;
        if( CMake::checkForNeedingConfigure(dom) )
        {
            kDebug() << "Needing configure, adding item and setting job";
            configure = this->configure(p);
        } else if( CMake::currentBuildDir(p).isEmpty() ) 
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                               i18n("No Build Directory configured, cannot build"), i18n("Aborting build") );
            return 0;
        }
        
        kDebug(9032) << "Building with make";
        KJob* build = m_builder->build(builditem);
        if( configure ) 
        {
            kDebug() << "creating composite job";
            build = new ConfigureAndBuildJob( configure, build );
        }
        return build;
    }
    return 0;
}

KJob* CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::ProjectBaseItem* item = dom;
    if( m_builder )
    {
        if(dom->file()) //It doesn't work to compile a file
            item=(KDevelop::ProjectBaseItem*) dom->parent();
        
        KJob* configure = 0;
        if( CMake::checkForNeedingConfigure(item) )
        {
            configure = this->configure(item->project());
        } else if( CMake::currentBuildDir( item->project() ).isEmpty() ) 
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                               i18n("No Build Directory configured, cannot clean"), i18n("Aborting clean") );
            return 0;
        }
        
        kDebug(9032) << "Cleaning with make";
        KJob* clean = m_builder->clean(item);
        if( configure ) {
            clean = new ConfigureAndBuildJob( configure, clean );
        }
        return clean;
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
        

        KJob* configure = 0;
        if( CMake::checkForNeedingConfigure(item) )
        {
            configure = this->configure(item->project());
        } else if( CMake::currentBuildDir( item->project() ).isEmpty() ) 
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                               i18n("No Build Directory configured, cannot install"), i18n("Aborting install") );
            return 0;
        }
        
        kDebug(9032) << "Installing with make";
        KJob* install = m_builder->install(item);
        if( configure ) {
            install = new ConfigureAndBuildJob( configure, install );
        }
        return install;

    }
    return 0;
}

KJob* CMakeBuilder::configure( KDevelop::IProject* project )
{
    if( CMake::currentBuildDir( project ).isEmpty() )
    {
        KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                           i18n("No Build Directory configured, cannot configure"), i18n("Aborting configure") );
        return 0;
    }
    CMakeJob* job = new CMakeJob(this);
    job->setProject(project);
    return job;
}

KJob* CMakeBuilder::prune( KDevelop::IProject* project )
{
    KUrl builddir = CMake::currentBuildDir( project );
    if( builddir.isEmpty() )
    {
        KMessageBox::information(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                                 i18n("No Build Directory configured, cannot clear builddir"), i18n("No clearing of builddir possible") );
        return 0;
    }
    else if (!builddir.isLocalFile() || QDir(builddir.toLocalFile()).exists("CMakeLists.txt"))
    {
        KMessageBox::information(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                                 i18n("Wrong build directory, cannot clear the build directory"), i18n("No clearing of builddir possible") );
        return 0;
    }
    QDir d( builddir.toLocalFile() );
    KUrl::List urls;
    foreach( const QString& entry, d.entryList( QDir::NoDotAndDotDot | QDir::AllEntries ) )
    {
        KUrl tmp = builddir;
        tmp.addPath( entry );
        urls << tmp;
    }
    return KIO::del( urls );
}

#include "cmakebuilder.moc"
