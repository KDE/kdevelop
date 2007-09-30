/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#include "makebuilder.h"
#include "makeoutputmodel.h"
#include <config.h>

#include <QtCore/QStringList>

#include <projectmodel.h>

#include <ibuildsystemmanager.h>
#include <commandexecutor.h>
#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ioutputview.h>
#include <QtDesigner/QExtensionFactory>
#include <QtCore/QSignalMapper>
#include <QtCore/QProcess>
#include <QtGui/QAction>
#include <QtGui/QKeySequence>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <ksharedconfig.h>

#include <environmentgrouplist.h>

#include "makeoutputdelegate.h"

K_PLUGIN_FACTORY(MakeBuilderFactory, registerPlugin<MakeBuilder>(); )
K_EXPORT_PLUGIN(MakeBuilderFactory("kdevmakebuilder"))


MakeBuilder::MakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(MakeBuilderFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IMakeBuilder )
    errorMapper = new QSignalMapper(this);
    successMapper = new QSignalMapper(this);
    connect(errorMapper, SIGNAL(mapped(int)),
            this, SLOT(commandFailed(int) ));
    connect(successMapper, SIGNAL(mapped(int)),
            this, SLOT(commandFinished(int) ));
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        connect( i, SIGNAL( viewRemoved( const QString& ) ),
                 this, SLOT( cleanupModel( const QString& ) ) );
    }

}

MakeBuilder::~MakeBuilder()
{
}

void MakeBuilder::cleanupModel( int id )
{
    kDebug(9038) << "view was removed, check wether its one of ours";
    if( m_models.contains( id ) )
    {
        kDebug(9038) << "do some cleanup";
        MakeOutputModel* model = m_models[id];
        KDevelop::CommandExecutor* cmd = m_commands[id];
        MakeOutputDelegate* delegate = m_delegates[id];
        foreach( KDevelop::IProject* p, m_ids.keys() )
        {
            if( m_ids[p] == id )
            {
                m_ids.remove(p);
                break;
            }
        }
        m_delegates.remove(id);
        m_models.remove(id);
        m_commands.remove(id);
        m_items.remove(id);
        errorMapper->removeMappings(cmd);
        successMapper->removeMappings(cmd);
        delete model;
        delete delegate;
        delete cmd;
    }
}

bool MakeBuilder::build( KDevelop::ProjectBaseItem *dom )
{
    kDebug(9038) << "Building with make";
    if( ! (dom->type() == KDevelop::ProjectBaseItem::Target ||
           dom->type() == KDevelop::ProjectBaseItem::BuildFolder ) )
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
            int id;
            if( m_ids.contains(dom->project()) )
            {
                id = m_ids[dom->project()];
                m_models[id]->clear();
            }else
            {
                id = view->registerView(i18n("Make: %1", dom->project()->name() ) );
                m_ids[dom->project()] = id;
                m_models[id] = new MakeOutputModel(this, this);
                m_delegates[id] = new MakeOutputDelegate(this);
                view->setModel(id, m_models[id]);
                view->setDelegate(id, m_delegates[id]);
            }
            m_models[id]->appendRow( new QStandardItem( cmd.join(" ") ) );

            if( m_commands.contains(id) )
                delete m_commands[id];

            m_commands[id] = new KDevelop::CommandExecutor(cmd.first());
            m_commands[id]->setWorkingDirectory(buildDir.toLocalFile() );
            cmd.pop_front();
            m_commands[id]->setArguments( cmd );
            QMap<QString, QString> envMap = environmentVars( dom );
            m_commands[id]->setEnvironment( envMap );

            connect(m_commands[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(addStandardOutput(const QStringList&)));
            connect(m_commands[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(addStandardError(const QStringList&)));

            connect( m_commands[id], SIGNAL( failed() ), errorMapper, SLOT( map() ) );
            connect( m_commands[id], SIGNAL( completed() ), successMapper, SLOT( map() ) );
            errorMapper->setMapping( m_commands[id], id );
            successMapper->setMapping( m_commands[id], id );
            kDebug(9038) << "Starting build:" << cmd << "Build directory" << buildDir;
            m_commands[id]->start();
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

void MakeBuilder::commandFinished(int id)
{
    if( m_items.contains(id) )
    {
        emit built( m_items[id] );
    }
}

void MakeBuilder::commandFailed(int id)
{
    if( m_items.contains(id) )
    {
        emit failed( m_items[id] );
    }
}

KUrl MakeBuilder::computeBuildDir( KDevelop::ProjectBaseItem* item )
{
    KUrl buildDir;
    if( item->type() == KDevelop::ProjectBaseItem::BuildFolder )
    {
        KDevelop::ProjectBuildFolderItem* prjitem = static_cast<KDevelop::ProjectBuildFolderItem*>(item);
        KDevelop::IBuildSystemManager *bldMan = prjitem->project()->buildSystemManager();
        if( bldMan )
            buildDir = bldMan->buildDirectory( prjitem ); // the correct build dir
        else
            buildDir = prjitem->url();
    }

    else if( item->type() == KDevelop::ProjectBaseItem::Target )
    {
        KDevelop::ProjectTargetItem* targetItem = static_cast<KDevelop::ProjectTargetItem*>(item);
        // get top build directory, specified by build system manager
        KDevelop::IBuildSystemManager *bldMan = targetItem->project()->buildSystemManager();
        KDevelop::ProjectFolderItem *prjItem = targetItem->project()->projectItem();
        KUrl topBldDir;
        // ### buildDirectory only takes ProjectItem as an argument. Why it can't be
        // any ProjectBaseItem?? This will make the algorithms belows much easier
        if( prjItem )
        {
            if( bldMan )
                topBldDir = bldMan->buildDirectory( prjItem ); // the correct build dir
            else
            {
                kDebug(9038) << "Warning: fail to get build manager";
                topBldDir = prjItem->url();
            }
        }
        else
        {
            // just set to top project dir, since we can't call buildDirectory without ProjectItem
            kDebug(9038) << "Warning: fail to retrieve KDevelop::ProjectItem";
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
                targetItem->parent()->type() == KDevelop::ProjectBaseItem::BuildFolder )
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

    else if( item->type() == KDevelop::ProjectBaseItem::BuildFolder )
    {
        KDevelop::ProjectBuildFolderItem *bldFolderItem = static_cast<KDevelop::ProjectBuildFolderItem*>(item);
        // get top build directory, specified by build system manager
        KDevelop::IBuildSystemManager *bldMan = bldFolderItem->project()->buildSystemManager();
        KDevelop::ProjectFolderItem *prjItem = bldFolderItem->project()->projectItem();
        KUrl topBldDir;
        if( !prjItem || !bldMan )
        {
            // can't find top build dir. Just set to item's url
            buildDir = bldFolderItem->url();
            return buildDir;
        }
        else
        {
            topBldDir = bldMan->buildDirectory( prjItem ); // the correct build dir
        }
        // now compute rel_dir between prjItem::url() and buildfolderItem::url()
        QString relative;
        KUrl rootUrl = prjItem->url();
        rootUrl.adjustPath(KUrl::AddTrailingSlash);
        relative = KUrl::relativeUrl( rootUrl, bldFolderItem->url() );
        // and append that difference to top_build_dir found above.
        buildDir = topBldDir;
        buildDir.addPath( relative );
    }
    return buildDir;
}

QStringList MakeBuilder::computeBuildCommand( KDevelop::ProjectBaseItem *item )
{
    QStringList cmdline;
//     QString cmdline = DomUtil::readEntry(dom, makeTool);
//     int prio = DomUtil::readIntEntry(dom, priority);
//     QString nice;
//     if (prio != 0) {
//         nice = QString("nice -n%1 ").arg(prio);
//     }

    KSharedConfig::Ptr configPtr = item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );

    QString makeBin = builderGroup.readEntry("Make Binary", "make");
    cmdline << makeBin;

    if( ! builderGroup.readEntry("Abort on First Error", true) )
    {
        cmdline << "-k";
    }
    if( builderGroup.readEntry("Run Multiple Jobs", false ) )
    {
        int jobnumber = builderGroup.readEntry("Number Of Jobs", 1);
        QString jobNumberArg = QString("-j%1").arg(jobnumber);
        cmdline << jobNumberArg;
    }
    if( builderGroup.readEntry("Display Only", false) )
    {
        cmdline << "-n";
    }
    QString extraOptions = builderGroup.readEntry("Additional Options", QString());
    if( ! extraOptions.isEmpty() )
    {
        cmdline << extraOptions;
    }

    if( item->type() == KDevelop::ProjectBaseItem::Target )
    {
        KDevelop::ProjectTargetItem* targetItem = static_cast<KDevelop::ProjectTargetItem*>(item);
        cmdline << targetItem->text();
    }
    else if( item->type() == KDevelop::ProjectBaseItem::BuildFolder )
    {
        QString target = builderGroup.readEntry("Default Target", QString());
        if( !target.isEmpty() )
            cmdline << target;
    }

//     Q_ASSERT(item->folder());

//     cmdline.prepend("&&");
//     cmdline.prepend( item->folder()->text() );
//     cmdline.prepend("cd");

    return cmdline;
}

QMap<QString, QString> MakeBuilder::environmentVars( KDevelop::ProjectBaseItem* item )
{
    KSharedConfig::Ptr configPtr = item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );
    QString defaultProfile = builderGroup.readEntry(
            "Default Make Environment Profile", "default" );

    QStringList procDefaultList = QProcess::systemEnvironment();
    QMap<QString, QString> retMap;
    foreach( QString _line, procDefaultList )
    {
        QString varName = _line.section( '=', 0, 0 );
        QString varValue = _line.section( '=', 1 );
        retMap.insert( varName, varValue );
    }
    if( defaultProfile.isEmpty() )
        return retMap;

    const KDevelop::EnvironmentGroupList l(configPtr);
    const QMap<QString, QString> userMap = l.variables( defaultProfile );

    for( QMap<QString, QString>::const_iterator it = userMap.begin();
         it != userMap.end(); ++it )
    {
        retMap.insert( it.key(), it.value() );
    }

    return retMap;
}

#include "makebuilder.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
