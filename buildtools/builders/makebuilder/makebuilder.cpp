/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <QtCore/QStringList>

#include "makebuilder.h"
#include <projectmodel.h>

#include <ibuildsystemmanager.h>
#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ioutputview.h>
#include <QtDesigner/QExtensionFactory>

#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#define MAKE_COMMAND "make"

typedef KGenericFactory<MakeBuilder> MakeBuilderFactory ;
K_EXPORT_COMPONENT_FACTORY(kdevmakebuilder, MakeBuilderFactory("kdevmakebuilder"))

MakeBuilder::MakeBuilder(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(MakeBuilderFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IMakeBuilder )
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            connect(i, SIGNAL(commandFinished(const QString &)),
                this, SLOT(commandFinished(const QString &)));
            connect(i, SIGNAL(commandFailed(const QString &)),
                this, SLOT(commandFailed(const QString &)));
        }
    }
}

MakeBuilder::~MakeBuilder()
{
}

bool MakeBuilder::build( KDevelop::ProjectBaseItem *dom )
{
    kDebug(9038) << "Building with make" << endl;
    if( ! (dom->type() == KDevelop::ProjectBaseItem::Project || dom->type() == KDevelop::ProjectBaseItem::Target) )
        return false;

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            KUrl buildDir = computeBuildDir( dom );
            if( !buildDir.isValid() )
                return false;
            QStringList cmd = computeBuildCommand( dom );
            if( cmd.isEmpty() )
                return false;
            m_queue << QPair<QStringList, KDevelop::ProjectBaseItem*>( cmd, dom );

            kDebug(9038) << "Starting build: " << cmd << " Build directory " << buildDir << endl;
            view->queueCommand( buildDir, cmd, QMap<QString,QString>() );
            return true;
        } // end of if(view)
    }
    return false;
}

bool MakeBuilder::clean( KDevelop::ProjectBaseItem *dom )
{
    Q_UNUSED(dom);
    return false;
}

void MakeBuilder::commandFinished(const QString &command)
{
    if( !m_queue.isEmpty() )
    {
        QPair< QStringList, KDevelop::ProjectBaseItem* > pair = m_queue.front();

        if( pair.first.join(" ") == command )
        {
            m_queue.pop_front();
            emit built( pair.second );
        }
    }
}

void MakeBuilder::commandFailed(const QString &command)
{
    if( !m_queue.isEmpty() )
    {
        QPair<QStringList, KDevelop::ProjectBaseItem*> pair = m_queue.front();
        if( pair.first.join(" ") == command )
        {
            m_queue.pop_front();
            emit failed(pair.second);
        }
    }
}

KUrl MakeBuilder::computeBuildDir( KDevelop::ProjectBaseItem* item )
{
    KUrl buildDir;
    if( item->type() == KDevelop::ProjectBaseItem::Project )
    {
        KDevelop::ProjectItem* prjitem = static_cast<KDevelop::ProjectItem*>(item);
        KDevelop::IPlugin *plugin = prjitem->project()->managerPlugin();
        KDevelop::IBuildSystemManager *bldMan = plugin->extension<KDevelop::IBuildSystemManager>();
        if( bldMan )
            buildDir = bldMan->buildDirectory( prjitem ); // the correct build dir
        else
            buildDir = prjitem->url();
    }
    else if( item->type() == KDevelop::ProjectBaseItem::Target )
    {
        KDevelop::ProjectTargetItem* targetItem = static_cast<KDevelop::ProjectTargetItem*>(item);
        // get top build directory, specified by build system manager
        KDevelop::IPlugin *plugin = targetItem->project()->managerPlugin();
        KDevelop::IBuildSystemManager *bldMan = plugin->extension<KDevelop::IBuildSystemManager>();
        KDevelop::ProjectItem *prjItem = targetItem->project()->projectItem();
        KUrl topBldDir;
        // ### buildDirectory only takes ProjectItem as an argument. Why it can't be
        // any ProjectBaseItem?? This will make the algorithms belows much easier
        if( prjItem )
        {
            if( bldMan )
                topBldDir = bldMan->buildDirectory( prjItem ); // the correct build dir
            else
            {
                kDebug(9038) << " Warning: fail to get build manager " << endl;
                topBldDir = prjItem->url();
            }
        }
        else
        {
            // just set to top project dir, since we can't call buildDirectory without ProjectItem
            kDebug(9038) << " Warning: fail to retrieve KDevelop::ProjectItem " << endl;
            topBldDir = targetItem->project()->folder();
        }

        // now compute relative directory: itemDir - topProjectDir,
        // and append the difference to top_build_dir found above.
        if( targetItem->parent() == NULL )
        {
            // This case shouldn't happen. Just set to project top build dir.
            buildDir = topBldDir;
        }
        else
        {
            if( targetItem->parent()->type() == KDevelop::ProjectBaseItem::Folder ||
                targetItem->parent()->type() == KDevelop::ProjectBaseItem::BuildFolder ||
                targetItem->parent()->type() == KDevelop::ProjectBaseItem::Project )
            {
                KDevelop::ProjectFolderItem *parentOfTarget =
                        static_cast<KDevelop::ProjectFolderItem*>( targetItem->parent() );
                KUrl itemDir = parentOfTarget->url();

                QString relative;
                if( prjItem )
                {
                    // desired case
                    KUrl rootUrl = prjItem->url();
                    rootUrl.adjustPath(KUrl::AddTrailingSlash);
                    relative = KUrl::relativeUrl( rootUrl, itemDir );
                }
                else
                {
                    KUrl topProjectDir = targetItem->project()->folder();
                    topProjectDir.adjustPath(KUrl::AddTrailingSlash);
                    relative = KUrl::relativeUrl( topProjectDir, itemDir );
                }

                buildDir = topBldDir;
                buildDir.addPath( relative );
            }
            else
            {
                // This case shouldn't happen too. Just set to top build dir.
                buildDir = topBldDir;
            }
        }
    } // end of else if( type() == ProjectTargetItem )
    return buildDir;
}

QStringList MakeBuilder::computeBuildCommand( KDevelop::ProjectBaseItem *item )
{
    //FIXME Get this from the new project file format
//     QDomDocument &dom = *KDevApi::self()->projectDom();

    QStringList cmdline;
//     QString cmdline = DomUtil::readEntry(dom, makeTool);
//     int prio = DomUtil::readIntEntry(dom, priority);
//     QString nice;
//     if (prio != 0) {
//         nice = QString("nice -n%1 ").arg(prio);
//     }

    if (cmdline.isEmpty())
        cmdline << MAKE_COMMAND;

//     if (!DomUtil::readBoolEntry(dom, abortOnError))
//         cmdline += " -k";
//     int jobs = DomUtil::readIntEntry(dom, numberOfJobs);
//     if (jobs != 0) {
//         cmdline += " -j";
//         cmdline += QString::number(jobs);
//     }
//     if (DomUtil::readBoolEntry(dom, dontAct))
//         cmdline += " -n";
    if( item->type() == KDevelop::ProjectBaseItem::Target )
    {
        KDevelop::ProjectTargetItem* targetItem = static_cast<KDevelop::ProjectTargetItem*>(item);
        cmdline << targetItem->text();
    }
//     cmdline.prepend(nice);
//     cmdline.prepend(makeEnvironment());

//     Q_ASSERT(item->folder());

//     cmdline.prepend("&&");
//     cmdline.prepend( item->folder()->text() );
//     cmdline.prepend("cd");

    return cmdline;
}

#include "makebuilder.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
