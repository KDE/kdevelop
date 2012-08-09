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
    : KDevelop::IPlugin(CMakeBuilderFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )

    addBuilder("Makefile", core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder"));
    addBuilder("build.ninja", core()->pluginController()->pluginForExtension("org.kdevelop.IProjectBuilder", "KDevNinjaBuilder"));
}

CMakeBuilder::~CMakeBuilder()
{
}

void CMakeBuilder::addBuilder(const QString& neededfile, KDevelop::IPlugin* i)
{
    if( i )
    {
        IProjectBuilder* b = i->extension<KDevelop::IProjectBuilder>();
        if( b )
        {
            m_builders[neededfile] = b;
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SLOT(buildFinished(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SLOT(buildFinished(KDevelop::ProjectBaseItem*)));
            
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SIGNAL(built(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SIGNAL(failed(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)), this, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(installed(KDevelop::ProjectBaseItem*)), this, SIGNAL(installed(KDevelop::ProjectBaseItem*)));

            kDebug() << "Added builder " << i->metaObject()->className() << "for" << neededfile;
        }
        else
            kWarning() << "Couldn't add " << i->metaObject()->className() << i->extensions();
    }
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
    IProjectBuilder* builder = builderForProject(p);
    if( builder )
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
        KJob* build = builder->build(builditem);
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
    IProjectBuilder* builder = builderForProject(dom->project());
    if( builder )
    {
        KDevelop::ProjectBaseItem* item = dom;
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
        KJob* clean = builder->clean(item);
        if( configure ) {
            clean = new ConfigureAndBuildJob( configure, clean );
        }
        return clean;
    }
    return 0;
}

KJob* CMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    IProjectBuilder* builder = builderForProject(dom->project());
    if( builder )
    {
        KDevelop::ProjectBaseItem* item = dom;
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
        KJob* install = builder->install(item);
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

KDevelop::IProjectBuilder* CMakeBuilder::builderForProject(KDevelop::IProject* p)
{
    QString builddir = CMake::currentBuildDir( p ).toLocalFile();
    QMap<QString, IProjectBuilder*>::const_iterator it = m_builders.constBegin(), itEnd = m_builders.constEnd();
    for(; it!=itEnd; ++it) {
        if(QFile::exists(builddir+'/'+it.key()))
            return it.value();
    }
    kWarning() << "Couldn't find a builder for "<< builddir;
    //We return makefile because sometimes we just want to configure
    //TODO: polish this further in the future
    return m_builders["Makefile"];
}

#include "cmakebuilder.moc"
