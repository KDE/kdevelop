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
      m_dirty(true)
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
    if(dom->folder())
        kDebug(9032) << "Building folder: " << dom->folder()->url();
    else if(dom->file())
        kDebug(9032) << "Building file: " << dom->file()->url();
    else if(dom->target())
        kDebug(9032) << "Building target";

//     kDebug(9032) << "Building " << dom->folder()->url();
//     if( dom->type() != KDevelop::ProjectBaseItem::Project )
//         return false;
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

            if(!updateConfig(dom->project()))
            {
                kDebug(9032) << "Input not correct";
                return false;
            }

            KUrl cmakeCachePath=m_buildDirectory;
            cmakeCachePath.addPath("CMakeCache.txt");
            if(QFile::exists(cmakeCachePath.toLocalFile())) //We do not want to run cmake always
            {
                completed(id);
                return true;
            }
            QStringList args(dom->project()->folder().toLocalFile());
            kDebug(9032) << "Type of build: " << m_buildType;
            kDebug(9032) << "Installing to: " << m_installPrefix;
            kDebug(9032) << "Build directory: " << m_buildDirectory;
            args += "-DCMAKE_INSTALL_PREFIX="+m_installPrefix.toLocalFile();
            args += "-DCMAKE_BUILD_TYPE="+m_buildType;

            QString cmd = m_cmakeBinary.toLocalFile();
            m_cmds[id] = new KDevelop::CommandExecutor(cmd, this);
            connect(m_cmds[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            connect(m_cmds[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            m_failedMapper->setMapping( m_cmds[id], id );
            m_completedMapper->setMapping( m_cmds[id], id );
            m_cmds[id]->setArguments(args);
            m_cmds[id]->setWorkingDirectory( m_buildDirectory.toLocalFile() );
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
                KDevelop::ProjectBaseItem* item = m_items[id];
                kDebug(9032) << "Building with make";
                if(!builder->build(item))
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

bool CMakeBuilder::updateConfig( KDevelop::IProject* project )
{
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");

    m_cmakeBinary = group.readEntry("CMake Binary");
    m_buildDirectory = group.readEntry("Build Dir");
    m_installPrefix = group.readEntry("Prefix");
    m_buildType = group.readEntry("Build Type", "-1");
    if(m_cmakeBinary.isEmpty())
    {
        KProcess p;
        p.setOutputChannelMode(KProcess::MergedChannels);
        p.setProgram("which", QStringList("cmake"));
        p.start();

        if(!p.waitForFinished())
        {
            kDebug(9032) << "failed to execute: 'which' command";
        }

        if(p.exitCode()!=0)
        {
            kDebug(9032) << "failed to execute: 'which' command. Bad return";
        }

        QByteArray b = p.readAllStandardOutput();
        QString t;
        t.prepend(b.trimmed());
        m_cmakeBinary = KUrl::fromPath(t);

        if(!m_cmakeBinary.isEmpty())
        {
            group.writeEntry("CMake Binary", m_cmakeBinary);
        }
        else
        {
            kDebug(9032) << "error!!! Could not find cmake in the path";
        }
    }
//     return m_cmakeBinary.isEmpty() || m_buildDirectory.isEmpty();
    return true;
}

#include "cmakebuilder.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

