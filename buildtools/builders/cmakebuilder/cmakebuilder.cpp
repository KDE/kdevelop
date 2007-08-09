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


#include <projectmodel.h>

#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ioutputview.h>
#include <outputmodel.h>
#include <commandexecutor.h>
#include <QtDesigner/QExtensionFactory>

#include <kgenericfactory.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#define CMAKE_COMMAND "cmake"

typedef KGenericFactory<CMakeBuilder> CMakeBuilderFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevcmakebuilder,
                            CMakeBuilderFactory( "kdevcmakebuilder" ) )

CMakeBuilder::CMakeBuilder(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(CMakeBuilderFactory::componentData(), parent),
      m_failedMapper( new QSignalMapper( this ) ),
      m_completedMapper( new QSignalMapper( this ) )
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
        IMakeBuilder* view = i->extension<IMakeBuilder>();
        if( view )
        {
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SLOT(built(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SLOT(failed(KDevelop::ProjectBaseItem*)));
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
        foreach( KDevelop::IProject* p, m_ids.keys() )
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

bool CMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    kDebug(9032) << "Building";
    if( dom->type() != KDevelop::ProjectBaseItem::Project )
        return false;
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {

            int id;
            if( m_ids.contains( dom->project() ) )
            {
                id = m_ids[dom->project()];
                m_models[id]->clear();
                if( m_cmds.contains(id) )
                    delete m_cmds[id];
            }
            else
            {
                id = view->registerView(i18n("CMake: %1", dom->project()->name() ) );
                m_ids[dom->project()] = id;
                m_models[id] = new KDevelop::OutputModel(this);
                view->setModel( id, m_models[id] );
            }
            m_items[id] = dom;
            
            QString cmd;
            QStringList args;
            KSharedConfig::Ptr cfg = dom->project()->projectConfiguration();
            KConfigGroup group(cfg.data(), "CMake");
            kDebug(9032) << "Reading setting (CMake Binary):" << group.readEntry("CMake Binary");
            KUrl cmdUrl = group.readEntry("CMake Binary", KUrl( "file:///usr/bin/cmake" ) );
            kDebug(9032) << "Reading setting (Prefix):" << group.readEntry("Prefix");
            args += dom->project()->folder().toLocalFile();
            KUrl prefixUrl = group.readEntry("Prefix");
            if(prefixUrl.isEmpty())
            {
                kDebug(9032) << "error. Prefix not defined";
                return false;
            }
            else
            {
                kDebug(9032) << "Installing to: " << prefixUrl;
                args += "-DCMAKE_INSTALL_PREFIX="+prefixUrl.toLocalFile();
            }
            
            kDebug(9032) << "Reading setting (Build Type):" << group.readEntry("Build Type");
            QString buildType = group.readEntry("Build Type", "-1");
            if(buildType=="-1")
            {
                kDebug(9032) << "Build Type not defined";
            }
            else
            {
                kDebug(9032) << "Type of build: " << buildType;
                args += "-DCMAKE_BUILD_TYPE="+buildType;
            }
            
            kDebug(9032) << "Reading setting (Build Dir):" << group.readEntry("Build Dir");
            KUrl buildUrl = group.readEntry("Build Dir");
            if(buildUrl.isEmpty())
            {
                kDebug(9032) << "Build Directory not defined";
                return false;
            }
            
            cmd = cmdUrl.toLocalFile();
            m_cmds[id] = new KDevelop::CommandExecutor(cmd, this);
            connect(m_cmds[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            connect(m_cmds[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            m_failedMapper->setMapping( m_cmds[id], id );
            m_completedMapper->setMapping( m_cmds[id], id );
            m_cmds[id]->setArguments(args);
            m_cmds[id]->setWorkingDirectory( buildUrl.toLocalFile() );
            connect( m_cmds[id], SIGNAL( failed() ), m_failedMapper, SLOT(map()));
            connect( m_cmds[id], SIGNAL( completed() ), m_completedMapper, SLOT(map()));
            m_cmds[id]->start();
            return true;
        }
    }
    return false;
}

bool CMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    Q_UNUSED( dom )
    return false;
}

void CMakeBuilder::completed(int id)
{
    kDebug(9032) << "command finished" << id;
    if( m_items.contains(id))
    {
        IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
        if( i )
        {
            IMakeBuilder* builder = i->extension<IMakeBuilder>();
            if( builder )
            {
                kDebug(9032) << "Building with make";
                if(!builder->build(m_items[id]))
                    kDebug(9032) << "The build failed.";
            }
            else
                kDebug(9032) << "Make builder not with extension";
        }
        else
            kDebug(9032) << "Make builder not found";
    }
}

void CMakeBuilder::errored(int id)
{
    if( m_items.contains(id))
        emit failed(m_items[id]);
}

QString CMakeBuilder::cmakeBinary( KDevelop::IProject* project )
{
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake Builder");
    KUrl v = group.readEntry("CMake Binary", KUrl( "file:///usr/bin/cmake" ) );
    return v.toLocalFile();
}

#include "cmakebuilder.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
