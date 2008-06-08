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
    m_errorMapper = new QSignalMapper(this);
    m_successMapper = new QSignalMapper(this);
    connect(m_errorMapper, SIGNAL(mapped(int)),
            this, SLOT(commandFailed(int) ));
    connect(m_successMapper, SIGNAL(mapped(int)),
            this, SLOT(commandFinished(int) ));
    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        connect( i, SIGNAL( outputRemoved( int, int ) ),
                 this, SLOT( cleanupModel( int, int ) ) );
    }

}

MakeBuilder::~MakeBuilder()
{
}

void MakeBuilder::cleanupModel( int, int id )
{
    if( m_models.contains( id ) )
    {
        MakeOutputModel* model = m_models[id];
        KDevelop::CommandExecutor* cmd = m_commands[id];
        MakeOutputDelegate* delegate = m_delegates[id];
        foreach( KDevelop::ProjectBaseItem* p, m_ids.keys() )
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
        m_errorMapper->removeMappings(cmd);
        m_successMapper->removeMappings(cmd);
        delete model;
        delete delegate;
        delete cmd;
    }
}

bool MakeBuilder::build( KDevelop::ProjectBaseItem *dom )
{
    return runMake( dom, MakeBuilder::BuildCommand );
}

bool MakeBuilder::clean( KDevelop::ProjectBaseItem *dom )
{
    return runMake( dom, CleanCommand, "clean" );
}

bool MakeBuilder::install( KDevelop::ProjectBaseItem *dom )
{
    return runMake( dom, InstallCommand, "install" );
}


void MakeBuilder::commandFinished(int id)
{
    if( m_items.contains(id) )
    {
        switch( m_commandTypes[id] )
        {
            case BuildCommand:
                emit built( m_items[id] );
                break;
            case InstallCommand:
                emit installed( m_items[id] );
                break;
            case CleanCommand:
                emit cleaned( m_items[id] );
                break;
            case CustomTargetCommand:
                emit makeTargetBuilt( m_items[id], m_customTargets[id] );
                break;
        }
        m_models[id]->appendRow( new QStandardItem( "Done." ) );
    }
}

void MakeBuilder::commandFailed(int id)
{
    if( m_items.contains(id) )
    {
        emit failed( m_items[id] );
        m_models[id]->appendRow( new QStandardItem( "Failed." ) );
    }
}

KUrl MakeBuilder::computeBuildDir( KDevelop::ProjectBaseItem* item )
{
    KUrl buildDir;
    KDevelop::IBuildSystemManager *bldMan = item->project()->buildSystemManager();
    if( bldMan )
        buildDir = bldMan->buildDirectory( item ); // the correct build dir
    else
    {
        switch( item->type() )
        {
            case KDevelop::ProjectBaseItem::Folder:
            case KDevelop::ProjectBaseItem::BuildFolder:
                return static_cast<KDevelop::ProjectFolderItem*>(item)->url();
                break;
            case KDevelop::ProjectBaseItem::Target:
            case KDevelop::ProjectBaseItem::File:
                buildDir = computeBuildDir( static_cast<KDevelop::ProjectBaseItem*>( item->parent() ) );
                break;
        }
    }
    return buildDir;
}

QStringList MakeBuilder::computeBuildCommand( KDevelop::ProjectBaseItem *item, const QString& overrideTarget )
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

    if( overrideTarget.isEmpty() )
    {
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
    }else
    {
        cmdline << overrideTarget;
    }

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

    const KDevelop::EnvironmentGroupList l( KGlobal::config() );
    const QMap<QString, QString> userMap = l.variables( defaultProfile );

    for( QMap<QString, QString>::const_iterator it = userMap.begin();
         it != userMap.end(); ++it )
    {
        retMap.insert( it.key(), it.value() );
    }

    return retMap;
}

bool MakeBuilder::executeMakeTarget(KDevelop::ProjectBaseItem* item,
                                    const QString& targetname )
{
    return runMake( item, MakeBuilder::CustomTargetCommand, targetname );
}

bool MakeBuilder::runMake( KDevelop::ProjectBaseItem* item, CommandType c,  const QString& overrideTarget )
{
    kDebug(9037) << "Building with make" << c << overrideTarget;

    if( item->type() == KDevelop::ProjectBaseItem::File )
        return false;

    IPlugin* i = core()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            KUrl buildDir = computeBuildDir( item );
            if( !buildDir.isValid() )
                return false;
            QStringList cmd = computeBuildCommand( item, overrideTarget );
            if( cmd.isEmpty() )
                return false;
            int id;
            if( m_ids.contains(item) )
            {
                id = m_ids[item];
                m_models[id]->clear();
            }else
            {
                QString target;
                if( !overrideTarget.isEmpty() )
                {
                    target = overrideTarget;
                }else
                {
                    item->text();
                }
                int toolviewid = view->standardToolView(KDevelop::IOutputView::BuildView );
                id = view->registerOutputInToolView(toolviewid, i18n("Make: %1", target), KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
                m_ids[item] = id;
                m_models[id] = new MakeOutputModel(this, this);
                m_delegates[id] = new MakeOutputDelegate(this);
                view->setModel(id, m_models[id]);
                view->setDelegate(id, m_delegates[id]);
            }
            m_items[id] = item;
            view->raiseOutput(id);
            m_models[id]->appendRow( new QStandardItem( cmd.join(" ") ) );

            if( m_commands.contains(id) )
                delete m_commands[id];

            m_commands[id] = new KDevelop::CommandExecutor(cmd.first());
            m_commands[id]->setWorkingDirectory(buildDir.toLocalFile() );
            cmd.pop_front();
            m_commands[id]->setArguments( cmd );
            QMap<QString, QString> envMap = environmentVars( item );
            m_commands[id]->setEnvironment( envMap );

            connect(m_commands[id], SIGNAL(receivedStandardOutput(const QStringList&)),
                    m_models[id], SLOT(addStandardOutput(const QStringList&)));
            connect(m_commands[id], SIGNAL(receivedStandardError(const QStringList&)),
                    m_models[id], SLOT(addStandardError(const QStringList&)));

            m_errorMapper->setMapping( m_commands[id], id );
            m_successMapper->setMapping( m_commands[id], id );
            connect( m_commands[id], SIGNAL( failed() ), m_errorMapper, SLOT( map() ) );
            connect( m_commands[id], SIGNAL( completed() ), m_successMapper, SLOT( map() ) );
            kDebug(9037) << "Starting build:" << cmd << "Build directory" << buildDir;
            m_commandTypes[id] = c;
            m_customTargets[id] = overrideTarget;
            m_commands[id]->start();
            return true;
        } // end of if(view)
    }
    return false;
}

#include "makebuilder.moc"
