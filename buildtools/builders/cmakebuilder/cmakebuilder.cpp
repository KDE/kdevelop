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

#define CMAKE_COMMAND "cmake"

K_PLUGIN_FACTORY(CMakeBuilderFactory, registerPlugin<CMakeBuilder>(); )
K_EXPORT_PLUGIN(CMakeBuilderFactory("kdevcmakebuilder"))

CMakeBuilder::CMakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(CMakeBuilderFactory::componentData(), parent),
      m_failedMapper( new QSignalMapper( this ) ),
      m_completedMapper( new QSignalMapper( this ) ),
      m_dirty(true), m_builder( 0 )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( ICMakeBuilder )
    m_failedMapper = new QSignalMapper(this);
    connect(m_failedMapper, SIGNAL(mapped( int )), this, SLOT(errored( int)));
    m_completedMapper = new QSignalMapper(this);
    connect(m_completedMapper, SIGNAL(mapped( int )), this, SLOT(completed( int )));

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        connect( i, SIGNAL( viewRemoved( int ) ), this, SLOT( cleanupModel( int ) ) );
    }
    i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
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

void CMakeBuilder::cleanupModel( int id )
{
    kDebug(9032) << "view was removed, check wether its one of ours";
    if( m_models.contains( id ) )
    {
        kDebug(9032) << "do some cleanup";
        KDevelop::OutputModel* model = m_models[id];
        KDevelop::CommandExecutor* cmd = m_cmds[id];
        foreach( KDevelop::ProjectBaseItem* p, m_ids.keys() )
        {
            if( m_ids[p] == id )
            {
                m_ids.remove(p);
                break;
            }
        }
        m_models.remove(id);
        m_cmds.remove(id);
        m_items.remove(id);
        m_failedMapper->removeMappings(cmd);
        m_completedMapper->removeMappings(cmd);
        delete model;
        delete cmd;
    }
}

bool CMakeBuilder::build(KDevelop::ProjectBaseItem *item)
{
    if( m_builder )
    {
        kDebug(9032) << "Building with make";
        return m_builder->build(item);
    }
    return false;
}

bool CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
        kDebug(9032) << "Cleaning with make";
        return m_builder->clean(dom);
    }
    return false;
}

bool CMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    if( m_builder )
    {
        kDebug(9032) << "Installing with make";
        return m_builder->install(dom);
    }
    return false;
}


void CMakeBuilder::completed(int id)
{
    kDebug(9032) << "command finished" << id;
}

void CMakeBuilder::errored(int id)
{
    if( m_items.contains(id))
        emit failed(m_items[id]);
}

bool CMakeBuilder::configure( KDevelop::IProject* project )
{
    if( !project )
        return false;
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {

            int id;
            if( m_ids.contains( project->projectItem() ) )
            {
                id = m_ids[project->projectItem()];
                m_models[id]->clear();
                if( m_cmds.contains(id) )
                    delete m_cmds[id];
            }else
            {
                id = view->registerView(i18n("CMake: %1", project->name() ) );
                m_ids[project->projectItem()] = id;
                m_models[id] = new KDevelop::OutputModel(this);
                view->setModel( id, m_models[id] );
            }
            m_items[id] = project->projectItem();
            QString cmd = cmakeBinary( project );
            m_cmds[id] = new KDevelop::CommandExecutor(cmd, this);
            connect(m_cmds[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            connect(m_cmds[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            m_failedMapper->setMapping( m_cmds[id], id );
            m_completedMapper->setMapping( m_cmds[id], id );
            m_cmds[id]->setWorkingDirectory( buildDir( project ).toLocalFile() );
            m_cmds[id]->setArguments( cmakeArguments( project ) );
            connect( m_cmds[id], SIGNAL( failed() ), m_failedMapper, SLOT( map() ) );
            connect( m_cmds[id], SIGNAL( completed() ), m_completedMapper, SLOT( map() ) );
            m_cmds[id]->start();
            return true;
        }
    }
    return false;
}

QString CMakeBuilder::cmakeBinary( KDevelop::IProject* project )
{
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    KUrl v = group.readEntry("Current CMake Binary", KUrl( "file:///usr/bin/cmake" ) );
    return v.toLocalFile();
}

KUrl CMakeBuilder::buildDir( KDevelop::IProject* project )
{
    KDevelop::IBuildSystemManager* manager = project->buildSystemManager();
    if( manager )
    {
        return manager->buildDirectory( project->projectItem() );
    }
    return project->folder();
}

QStringList CMakeBuilder::cmakeArguments( KDevelop::IProject* project )
{
    QStringList args;
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    args << QString("-DCMAKE_INSTALL_PREFIX=%1").arg(group.readEntry("CurrentInstallDir", "/usr/local"));
    args << QString("-DCMAKE_BUILD_TYPE=%1").arg(group.readEntry("CurrentBuildType", "Release"));
    args << project->folder().toLocalFile();
    return args;
}

#include "cmakebuilder.moc"


