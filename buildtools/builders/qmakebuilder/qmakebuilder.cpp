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
      m_completedMapper( new QSignalMapper( this ) )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IQMakeBuilder )
    m_failedMapper = new QSignalMapper(this);
    connect(m_failedMapper, SIGNAL(mapped( int )),
            this, SLOT(errored( int)));
    m_completedMapper = new QSignalMapper(this);
    connect(m_completedMapper, SIGNAL(mapped( int )),
            this, SLOT(completed( int )));
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        connect( i, SIGNAL( viewRemoved( int ) ),
                 this, SLOT( cleanupModel( int ) ) );
    }
    i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if( i )
    {
        IMakeBuilder* view = i->extension<IMakeBuilder>();
        if( view )
        {
            connect(i, SIGNAL(built(KDevelop::ProjectBaseItem*)),
                this, SLOT(built(KDevelop::ProjectBaseItem*)));
            connect(i, SIGNAL(failed(KDevelop::ProjectBaseItem*)),
                this, SLOT(failed(KDevelop::ProjectBaseItem*)));
        }
    }
}

QMakeBuilder::~QMakeBuilder()
{
}

void QMakeBuilder::cleanupModel( int id )
{
    kDebug(9039) << "view was removed, check wether its one of ours";
    if( m_models.contains( id ) )
    {
        kDebug(9039) << "do some cleanup";
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

bool QMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    kDebug(9039) << "Building";
    if( dom->type() != KDevelop::ProjectBaseItem::BuildFolder )
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
            }else
            {
                id = view->registerView(i18n("QMake: %1", dom->project()->name() ) );
                m_ids[dom->project()] = id;
                m_models[id] = new KDevelop::OutputModel(this);
                view->setModel( id, m_models[id] );
            }
            m_items[id] = dom;
            QString cmd;
            KSharedConfig::Ptr cfg = dom->project()->projectConfiguration();
            KConfigGroup group(cfg.data(), "QMake Builder");
            kDebug(9039) << "Reading setting:" << group.readEntry("QMake Binary");
            KUrl v = group.readEntry("QMake Binary", KUrl( "file:///usr/bin/qmake" ) );
//             kDebug(9039) << v << v.type() << v.userType();
            cmd = v.toLocalFile();
            m_cmds[id] = new KDevelop::CommandExecutor(cmd, this);
            connect(m_cmds[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            connect(m_cmds[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(appendLines(const QStringList&) ) );
            m_failedMapper->setMapping( m_cmds[id], id );
            m_completedMapper->setMapping( m_cmds[id], id );
            m_cmds[id]->setWorkingDirectory( dom->project()->folder().toLocalFile() );
            connect( m_cmds[id], SIGNAL( failed() ), m_failedMapper, SLOT(map()));
            connect( m_cmds[id], SIGNAL( completed() ), m_completedMapper, SLOT(map()));
            m_cmds[id]->start();
            return true;
        }
    }
    return false;
}

bool QMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    Q_UNUSED( dom )
    return false;
}

bool QMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
{
    Q_UNUSED( dom )
    return false;
}

void QMakeBuilder::completed(int id)
{
    kDebug(9039) << "command finished" << id;
    if( m_items.contains(id))
    {
        IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
        if( i )
        {
            IMakeBuilder* builder = i->extension<IMakeBuilder>();
            if( builder )
            {
                kDebug(9039) << "Building with make";
                builder->build(m_items[id]);
            }else kDebug(9039) << "Make builder not with extension";
        } else kDebug(9039) << "Make builder not found";
    }
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

