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
#include "make/imakebuilder.h"
#include "../qmakeconfig.h"

#include <QtCore/QStringList>

#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <outputview/ioutputview.h>
#include <util/commandexecutor.h>

#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "qmakejob.h"

K_PLUGIN_FACTORY(QMakeBuilderFactory, registerPlugin<QMakeBuilder>(); )
K_EXPORT_PLUGIN(QMakeBuilderFactory(KAboutData(
    "kdevqmakebuilder","kdevqmakebuilder", ki18n("QMake Builder"),
    "1.3.60", ki18n("Support for building QMake projects"), KAboutData::License_GPL)))

QMakeBuilder::QMakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(QMakeBuilderFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IQMakeBuilder )

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

KJob* QMakeBuilder::prune( KDevelop::IProject* project )
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
    return 0;
}

KJob* QMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
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
    return 0;
}

KJob* QMakeBuilder::configure( KDevelop::IProject* project )
{
    QMakeJob* job = new QMakeJob(this);
    job->setProject(project);
    return job;
}


KJob* QMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
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
    return 0;
}

KJob* QMakeBuilder::install(KDevelop::ProjectBaseItem *dom)
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
    return 0;
}

#include "qmakebuilder.moc"
