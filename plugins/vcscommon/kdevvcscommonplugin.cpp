/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevvcscommonplugin.h"

#include <QtDesigner/QExtensionFactory>
#include <QAction>
#include <QVariant>
#include <QMenu>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kaction.h>

#include <kparts/mainwindow.h>

#include <interfaces/context.h>
#include <interfaces/icore.h>
#include <interfaces/iplugin.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <language/interfaces/codecontext.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/widgets/vcscommitdialog.h>
#include <vcs/vcsjob.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/indexedstring.h>

K_PLUGIN_FACTORY(KDevVcsCommonFactory, registerPlugin<KDevVcsCommonPlugin>(); )
K_EXPORT_PLUGIN(KDevVcsCommonFactory("kdevvcscommon"))

KDevVcsCommonPlugin::KDevVcsCommonPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin(KDevVcsCommonFactory::componentData(), parent)
{
    KAction* a = new KAction( i18n( "Commit..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( commit() ) );
    m_actions.insert( "commit", a );
}

KDevVcsCommonPlugin::~KDevVcsCommonPlugin()
{
}

KDevelop::IPlugin* KDevVcsCommonPlugin::findVcsPluginForProjectItem( KDevelop::ProjectBaseItem* item ) const
{
    KDevelop::IProject* project = item->project();
    if( project->versionControlPlugin() )
    {
        return project->versionControlPlugin();
    }
    return findVcsPluginForNonProjectUrl( urlForItem( item ) );
}

KDevelop::IPlugin* KDevVcsCommonPlugin::findVcsPluginForUrl( const KUrl& url ) const
{
    KDevelop::IProject* project = core()->projectController()->findProjectForUrl( url );
    if( project )
    {
        foreach(KDevelop::ProjectFileItem* item, project->filesForUrl( url ) )
        {
            KDevelop::IPlugin * plugin = findVcsPluginForProjectItem( item );
            if( plugin )
            {
                return plugin;
            }
        }
    }
    return findVcsPluginForNonProjectUrl( url );
}

KDevelop::IPlugin* KDevVcsCommonPlugin::findVcsPluginForNonProjectUrl( const KUrl& url ) const
{
    foreach( KDevelop::IPlugin* plugin, core()->pluginController()->allPluginsForExtension( "org.kdevelop.IBasicVersionControl" ) )
    {
        KDevelop::IBasicVersionControl* iface = plugin->extension<KDevelop::IBasicVersionControl>();

        if( iface  && iface->isVersionControlled( url ) )
        {
            return plugin;
        }
    }
    return 0;
}

KUrl KDevVcsCommonPlugin::urlForItem( KDevelop::ProjectBaseItem* item ) const
{
    KUrl url;
    if( item->file() )
        url = item->file()->url();
    else if( item->folder() )
        url = item->folder()->url();
    else
    {
        KDevelop::ProjectBaseItem* item = dynamic_cast<KDevelop::ProjectBaseItem*>( item->parent() );
        if( item )
        {
            url = item->folder()->url();
        }
    }
    return url;

}

KDevelop::ContextMenuExtension KDevVcsCommonPlugin::contextMenuExtension( KDevelop::Context* context )
{
    m_ctxUrls.clear();
    KDevelop::ContextMenuExtension menuExt;
    {
        KDevelop::ProjectItemContext* prjctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
        if( prjctx )
        {
            foreach( KDevelop::ProjectBaseItem* item, prjctx->items() )
            {
                IPlugin* plugin = findVcsPluginForProjectItem( item );
                if( plugin )
                {
                    m_ctxUrls.insert( urlForItem( item ), plugin );
                }
            }
        }
    }
    {
        KDevelop::EditorContext* editctx = dynamic_cast<KDevelop::EditorContext*>( context );
        if( editctx )
        {
            KDevelop::IPlugin* plugin = findVcsPluginForUrl( editctx->url() );
            if( plugin )
            {
                m_ctxUrls.insert( editctx->url(), plugin );
            }
        }
    }
    {
        KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>( context );
        if( filectx )
        {
            foreach( const KUrl url, filectx->urls() )
            {
                KDevelop::IPlugin* plugin = findVcsPluginForUrl( url );
                if( plugin )
                {
                    m_ctxUrls.insert( url, plugin );
                }
            }
        }
    }
    {
        KDevelop::CodeContext* codectx = dynamic_cast<KDevelop::CodeContext*>( context );
        if( codectx )
        {
            KUrl url = codectx->item()->url().toUrl();
            KDevelop::IPlugin* plugin = findVcsPluginForUrl( url );
            if( plugin )
            {
                m_ctxUrls.insert( url, plugin );
            }
        }
    }

    if( m_ctxUrls.isEmpty() )
    {
        foreach( KAction* a, m_actions.values() )
        {
            a->setEnabled( false );
        }
    }
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("commit") );
    return menuExt;
}

void KDevVcsCommonPlugin::commit()
{
    Q_ASSERT( !m_ctxUrls.isEmpty() );
    QHash<KDevelop::IPlugin*, KUrl::List > urlsPerPlugin;
    foreach( const KUrl& url , m_ctxUrls.keys() )
    {
        urlsPerPlugin[ m_ctxUrls.value(url) ].append( url );
    }
    foreach( KDevelop::IPlugin* plugin, urlsPerPlugin.keys() )
    {
        KDevelop::VcsCommitDialog* dlg = new KDevelop::VcsCommitDialog( plugin->extension<KDevelop::IBasicVersionControl>(), core()->uiController()->activeMainWindow() );
        dlg->setCommitCandidates( urlsPerPlugin.value( plugin ) );
        KConfigGroup vcsGroup( KGlobal::config(), "VcsCommon" );
        dlg->setOldMessages( vcsGroup.readEntry( "OldCommitMessages", QStringList() ) );
        dlg->setRecursive( true );
        connect(dlg, SIGNAL(doCommit(KDevelop::VcsCommitDialog*)), this, SLOT(executeCommit(KDevelop::VcsCommitDialog*)));
        connect(dlg, SIGNAL(cancelCommit(KDevelop::VcsCommitDialog*)), this, SLOT(cancelCommit(KDevelop::VcsCommitDialog*)));
        dlg->show();
    }
}

void KDevVcsCommonPlugin::executeCommit( KDevelop::VcsCommitDialog* dlg )
{
    KDevelop::IBasicVersionControl* iface = dlg->versionControlIface();
    KConfigGroup vcsGroup( KGlobal::config(), "VcsCommon" );
    QStringList oldMessages = vcsGroup.readEntry( "OldCommitMessages", QStringList() );
    oldMessages << dlg->message();
    vcsGroup.writeEntry("OldCommitMessages", oldMessages);
    core()->runController()->registerJob( iface->commit( dlg->message(), dlg->checkedUrls(), dlg->recursive() ? KDevelop::IBasicVersionControl::Recursive : KDevelop::IBasicVersionControl::NonRecursive ) );
    dlg->deleteLater();
}

void KDevVcsCommonPlugin::cancelCommit( KDevelop::VcsCommitDialog* dlg )
{
    dlg->deleteLater();
}

#include "kdevvcscommonplugin.moc"

