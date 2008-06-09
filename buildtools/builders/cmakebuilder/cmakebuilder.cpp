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


#include <projectmodel.h>

#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ibuildsystemmanager.h>
#include <ioutputview.h>
#include <outputmodel.h>
#include <commandexecutor.h>
#include <QtDesigner/QExtensionFactory>

#include <kpluginfactory.h>
#include <kpluginloader.h>
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
K_EXPORT_PLUGIN(CMakeBuilderFactory("kdevcmakebuilder"))

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

KJob* CMakeBuilder::build(KDevelop::ProjectBaseItem *item)
{
    if( m_builder )
    {
        kDebug(9032) << "Building with make";
        return m_builder->build(item);
    }
    return false;
}

KJob* CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
        kDebug(9032) << "Cleaning with make";
        return m_builder->clean(dom);
    }
    return false;
}

KJob* CMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
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
