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
#include <cmakebuilderconfig.h>

#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/builderjob.h>
#include <projectbuilders/makebuilder/imakebuilder.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kparts/mainwindow.h>
#include <kio/deletejob.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <KLocalizedString>
#include <kdebug.h>
#include <KProcess>
#include <kjob.h>
#include <kurl.h>

#include "cmakejob.h"
#include "prunejob.h"
#include "cmakeutils.h"
#include <cmakemodelitems.h>

K_PLUGIN_FACTORY(CMakeBuilderFactory, registerPlugin<CMakeBuilder>(); )
// K_EXPORT_PLUGIN(CMakeBuilderFactory(KAboutData("kdevcmakebuilder","kdevcmakebuilder", ki18n("CMake Builder"),
//                                                "0.1", ki18n("Support for building CMake projects"), KAboutData::License_GPL)))

CMakeBuilder::CMakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin("kdevcmakebuilder", parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )

    addBuilder("Makefile", QStringList("Unix Makefiles") << "NMake Makefiles", core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder"));
    addBuilder("build.ninja", QStringList("Ninja"), core()->pluginController()->pluginForExtension("org.kdevelop.IProjectBuilder", "KDevNinjaBuilder"));
}

CMakeBuilder::~CMakeBuilder()
{
}

void CMakeBuilder::addBuilder(const QString& neededfile, const QStringList& generators, KDevelop::IPlugin* i)
{
    if( i )
    {
        IProjectBuilder* b = i->extension<KDevelop::IProjectBuilder>();
        if( b )
        {
            m_builders[neededfile] = b;
            foreach(const QString& gen, generators) {
                m_buildersForGenerator[gen] = b;
            }

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

KJob* CMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    KDevelop::IProject* p = dom->project();
    IProjectBuilder* builder = builderForProject(p);
    if( builder )
    {
        KJob* configure = 0;
        if( CMake::checkForNeedingConfigure(dom->project()) )
        {
            kDebug() << "Needing configure, adding item and setting job";
            configure = this->configure(p);
        } else if( CMake::currentBuildDir(p).isEmpty() ) 
        {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                               i18n("No Build Directory configured, cannot build"), i18n("Aborting build") );
            return 0;
        }
        KJob* build = 0;
        if(dom->file())
        {
            IMakeBuilder* makeBuilder = dynamic_cast<IMakeBuilder*>(builder);
            if (!makeBuilder) {
                return 0;
            }
            KDevelop::ProjectFileItem* file = dom->file();
            int lastDot = file->text().lastIndexOf('.');
            QString target = file->text().mid(0, lastDot)+".o";
            build = makeBuilder->executeMakeTarget(dom->parent(), target);
            qDebug() << "create build job for target" << build << dom << target;
        }
        kDebug(9032) << "Building with make";
        if (!build)
        {
            build = builder->build(dom);
        }
        if( configure )
        {
            kDebug() << "creating composite job";
            KDevelop::BuilderJob* builderJob = new KDevelop::BuilderJob;
            builderJob->addCustomJob( KDevelop::BuilderJob::Configure, configure, dom );
            builderJob->addCustomJob( KDevelop::BuilderJob::Build, build, dom );
            builderJob->updateJobName();
            build = builderJob;
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
        if( CMake::checkForNeedingConfigure(item->project()) )
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
            KDevelop::BuilderJob* builderJob = new KDevelop::BuilderJob;
            builderJob->addCustomJob( KDevelop::BuilderJob::Configure, configure, item );
            builderJob->addCustomJob( KDevelop::BuilderJob::Clean, clean, item );
            builderJob->updateJobName();
            clean = builderJob;
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
        if( CMake::checkForNeedingConfigure(item->project()) )
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
            KDevelop::BuilderJob* builderJob = new KDevelop::BuilderJob;
            builderJob->addCustomJob( KDevelop::BuilderJob::Configure, configure, item );
            builderJob->addCustomJob( KDevelop::BuilderJob::Install, install, item );
            builderJob->updateJobName();
            install = builderJob;
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
    return new PruneJob(project);
}

KDevelop::IProjectBuilder* CMakeBuilder::builderForProject(KDevelop::IProject* p) const
{
    QString builddir = CMake::currentBuildDir( p ).toLocalFile();
    QMap<QString, IProjectBuilder*>::const_iterator it = m_builders.constBegin(), itEnd = m_builders.constEnd();
    for(; it!=itEnd; ++it) {
        if(QFile::exists(builddir+'/'+it.key()))
            return it.value();
    }
    //It means that it still has to be generated, so use the builder for
    //the generator we use
    return m_buildersForGenerator[CMakeBuilderSettings::self()->generator()];
}

QList< KDevelop::IProjectBuilder* > CMakeBuilder::additionalBuilderPlugins( KDevelop::IProject* project  ) const
{
    IProjectBuilder* b = builderForProject( project );
    QList< KDevelop::IProjectBuilder* > ret;
    if(b)
        ret << b;
    return ret;
}

#include "cmakebuilder.moc"
