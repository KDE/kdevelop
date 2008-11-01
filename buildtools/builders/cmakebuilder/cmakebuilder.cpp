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


#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <outputview/ioutputview.h>
#include <outputview/outputmodel.h>
#include <util/commandexecutor.h>
#include <QtDesigner/QExtensionFactory>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <KProcess>
#include <kjob.h>

#include "cmakejob.h"

#define CMAKE_COMMAND "cmake"

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

KJob* CMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
        if(dom->file())
        {
            int lastDot=dom->file()->text().lastIndexOf('.');
            QString target=dom->file()->text().mid(0, lastDot)+".o";
            
            KDevelop::ProjectTargetItem *it = new KDevelop::ProjectTargetItem(dom->project(), target);
            
            dom=it;
            m_deleteWhenDone.insert(static_cast<KDevelop::ProjectBaseItem*>(it));
        }
        
        kDebug(9032) << "Building with make";
        return m_builder->build(dom);
    }
    return false;
}

KJob* CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
        if(dom->file()) //It doesn't work to compile a file
            dom=(KDevelop::ProjectBaseItem*) dom->parent();
        
        kDebug(9032) << "Cleaning with make";
        return m_builder->clean(dom);
    }
    return false;
}

KJob* CMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
        if(dom->file())
            dom=(KDevelop::ProjectBaseItem*) dom->parent();
        
        kDebug(9032) << "Installing with make";
        return m_builder->install(dom);
    }
    return false;
}

KJob* CMakeBuilder::configure( KDevelop::IProject* project )
{
    CMakeJob* job = new CMakeJob(this);
    job->setProject(project);
    return job;
}

#include "cmakebuilder.moc"
