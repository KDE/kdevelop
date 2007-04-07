/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#include <projectmodel.h>

#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ioutputview.h>
#include <QtDesigner/QExtensionFactory>

#include <kgenericfactory.h>
#include <k3process.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#define QMAKE_COMMAND "qmake"

typedef KGenericFactory<QMakeBuilder> QMakeBuilderFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevqmakebuilder,
                            QMakeBuilderFactory( "kdevqmakebuilder" ) )

QMakeBuilder::QMakeBuilder(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(QMakeBuilderFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IQMakeBuilder )
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            connect(i, SIGNAL(commandFinished(const QStringList &)),
                this, SLOT(commandFinished(const QStringList &)));
            connect(i, SIGNAL(commandFailed(const QStringList &)),
                this, SLOT(commandFailed(const QStringList &)));
        }
    }
    i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
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

bool QMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    kDebug(9024) << "Building" << endl;
    if( dom->type() != KDevelop::ProjectBaseItem::Project )
        return false;
    KDevelop::ProjectItem* item = static_cast<KDevelop::ProjectItem*>(dom);
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            QStringList cmd;
            KSharedConfig::Ptr cfg = dom->project()->projectConfiguration();
            KConfigGroup group(cfg.data(), "QMake Builder");
            kDebug(9024) << "Reading setting: " << group.readEntry("QMake Binary") << endl;
            KUrl v = group.readEntry("QMake Binary", KUrl( "file:///usr/bin/qmake" ) );
//             kDebug(9024) << v << v.type() << v.userType() << endl;
            cmd << v.path();
            m_queue << QPair<QStringList, KDevelop::ProjectBaseItem*>( cmd, dom );
            view->queueCommand( item->url(), cmd, QStringList() );
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

void QMakeBuilder::commandFinished(const QStringList &command)
{
    kDebug(9024) << "command finished " << command << endl;
    if( !m_queue.isEmpty() )
    {
        kDebug(9024) << "queue not empty" << endl;
        QPair< QStringList, KDevelop::ProjectBaseItem* > pair = m_queue.front();

        if( pair.first == command )
        {
            kDebug(9024) << "found command" << endl;
            m_queue.pop_front();
            IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IMakeBuilder");
            if( i )
            {
                IMakeBuilder* builder = i->extension<IMakeBuilder>();
                if( builder )
                {
                    kDebug(9024) << "Building with make" << endl;
                    builder->build(pair.second);
                }else kDebug(9024) << "Make builder not with extension" << endl;
            }
            else kDebug(9024) << "Make builder not found" << endl;
        }
    }
}

void QMakeBuilder::commandFailed(const QStringList &command)
{
    if( !m_queue.isEmpty() )
    {
        QPair<QStringList, KDevelop::ProjectBaseItem*> pair = m_queue.front();
        if( pair.first == command )
        {
            m_queue.pop_front();
            emit failed(pair.second);
        }
    }
}

#include "qmakebuilder.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
