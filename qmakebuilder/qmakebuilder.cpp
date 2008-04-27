/* KDevelop QMake Support
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

#include "qmakebuilder.h"
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

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

K_PLUGIN_FACTORY(QMakeBuilderFactory, registerPlugin<QMakeBuilder>(); )
K_EXPORT_PLUGIN(QMakeBuilderFactory("kdevqmakebuilder"))

QMakeBuilder::QMakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(QMakeBuilderFactory::componentData(), parent),
      m_failedMapper( new QSignalMapper( this ) ),
      m_qmakeCompletedMapper( new QSignalMapper( this ) )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IQMakeBuilder )
    connect(m_failedMapper, SIGNAL(mapped( int )),
            this, SLOT(errored( int)));
    connect(m_qmakeCompletedMapper, SIGNAL(mapped( int )),
            this, SLOT(qmakeCompleted( int )));
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        connect( i, SIGNAL( viewRemoved( int, int ) ),
                 this, SLOT( cleanupModel( int, int ) ) );
    }
    m_makeBuilder = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if( m_makeBuilder )
    {
        IMakeBuilder* mbuilder = m_makeBuilder->extension<IMakeBuilder>();
        if( mbuilder )
        {
            connect( m_makeBuilder, SIGNAL( built( KDevelop::ProjectBaseItem* ) ),
                this, SIGNAL( built( KDevelop::ProjectBaseItem* ) ) );
            connect( m_makeBuilder, SIGNAL( cleaned( KDevelop::ProjectBaseItem* ) ),
                this, SIGNAL( cleaned( KDevelop::ProjectBaseItem* ) ) );
            connect( m_makeBuilder, SIGNAL( installed( KDevelop::ProjectBaseItem* ) ),
                this, SIGNAL( installed( KDevelop::ProjectBaseItem* ) ) );
            connect( m_makeBuilder, SIGNAL( failed( KDevelop::ProjectBaseItem* ) ),
                this, SIGNAL( failed( KDevelop::ProjectBaseItem* ) ) );
            connect( m_makeBuilder, SIGNAL( makeTargetBuilt( KDevelop::ProjectBaseItem*, const QString& ) ),
                this, SIGNAL( pruned( KDevelop::ProjectBaseItem* ) ) );
        }
    }
}

QMakeBuilder::~QMakeBuilder()
{
}

void QMakeBuilder::cleanupModel( int, int id )
{
    kDebug(9039) << "view was removed, check wether its one of ours";
    if( m_models.contains( id ) )
    {
        kDebug(9039) << "do some cleanup";
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
        m_qmakeCompletedMapper->removeMappings(cmd);
        delete model;
        delete cmd;
    }
}

bool QMakeBuilder::prune( KDevelop::IProject* project )
{
    kDebug(9039) << "Distcleaning";
    if( m_makeBuilder )
    {
        IMakeBuilder* builder = m_makeBuilder->extension<IMakeBuilder>();
        if( builder )
        {
            kDebug(9039) << "Distcleaning with make";
            return builder->executeMakeTarget(project->projectItem(), "distclean");
        }
    }
    return false;
}

bool QMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    kDebug(9039) << "Building";
    if( m_makeBuilder )
    {
        IMakeBuilder* builder = m_makeBuilder->extension<IMakeBuilder>();
        if( builder )
        {
            kDebug(9039) << "Building with make";
            return builder->build(dom);
        }
    }
    return false;
}

bool QMakeBuilder::configure( KDevelop::IProject* project )
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
                int tvid = view->standardToolView( KDevelop::IOutputView::BuildView );
                id = view->registerOutputInToolView( tvid, i18n("QMake: %1", project->name()), KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
                m_ids[project->projectItem()] = id;
                m_models[id] = new KDevelop::OutputModel(this);
                view->setModel( id, m_models[id] );
            }
            m_items[id] = project->projectItem();
            QString cmd = qmakeBinary( project );
            m_cmds[id] = new KDevelop::CommandExecutor(cmd, this);
            connect(m_cmds[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            connect(m_cmds[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            m_failedMapper->setMapping( m_cmds[id], id );
            m_qmakeCompletedMapper->setMapping( m_cmds[id], id );
            m_cmds[id]->setWorkingDirectory( project->folder().toLocalFile() );
            connect( m_cmds[id], SIGNAL( failed() ), m_failedMapper, SLOT( map() ) );
            connect( m_cmds[id], SIGNAL( completed() ), m_qmakeCompletedMapper, SLOT( map() ) );
            m_cmds[id]->start();
            return true;
        }
    }
    return false;
}


bool QMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    kDebug(9039) << "Cleaning";
    if( m_makeBuilder )
    {
        IMakeBuilder* builder = m_makeBuilder->extension<IMakeBuilder>();
        if( builder )
        {
            kDebug(9039) << "Cleaning with make";
            return builder->clean(dom);
        }
    }
    return false;
}

bool QMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    kDebug(9039) << "Installing";
    if( m_makeBuilder )
    {
        IMakeBuilder* builder = m_makeBuilder->extension<IMakeBuilder>();
        if( builder )
        {
            kDebug(9039) << "Installing with make";
            return builder->install(dom);
        }
    }
    return false;
}


void QMakeBuilder::distcleanCompleted( KDevelop::ProjectBaseItem* item, const QString& )
{
    emit pruned( item );
}

void QMakeBuilder::qmakeCompleted(int id)
{
    if( m_items.contains( id ) )
        emit configured( m_items[id] );
}

void QMakeBuilder::errored(int id)
{
    if( m_items.contains(id))
        emit failed(m_items[id]);
}

QString QMakeBuilder::qmakeBinary( KDevelop::IProject* project )
{
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "QMake Builder");
    KUrl v = group.readEntry("QMake Binary", KUrl( "file:///usr/bin/qmake" ) );
    return v.toLocalFile();
}

#include "qmakebuilder.moc"

