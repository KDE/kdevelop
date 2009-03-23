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
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kaction.h>

#include <ktexteditor/annotationinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <kparts/mainwindow.h>

#include <interfaces/context.h>
#include <interfaces/icore.h>
#include <interfaces/iplugin.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <language/interfaces/codecontext.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <vcs/widgets/vcscommitdialog.h>
#include <vcs/models/vcsannotationmodel.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsdiff.h>
#include <vcs/widgets/vcseventwidget.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/interfaces/editorcontext.h>

K_PLUGIN_FACTORY(KDevVcsCommonFactory, registerPlugin<KDevVcsCommonPlugin>(); )
K_EXPORT_PLUGIN(KDevVcsCommonFactory(KAboutData("kdevvcscommon","kdevvcscommon", ki18n("Common VCS"), "0.1", ki18n("Common VCS operations"), KAboutData::License_GPL)))


KDevVcsCommonPlugin::KDevVcsCommonPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin(KDevVcsCommonFactory::componentData(), parent)
{
    KAction* a = new KAction( i18n( "Commit..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( commit() ) );
    m_actions.insert( "commit", a );
    a = new KAction( i18n( "Add" ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( add() ) );
    m_actions.insert( "add", a );
    a = new KAction( i18n( "Remove" ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( remove() ) );
    m_actions.insert( "remove", a );
    a = new KAction( i18n( "Update" ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( update() ) );
    m_actions.insert( "update", a );
    a = new KAction( i18n( "Compare to Head..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( diffToHead() ) );
    m_actions.insert( "diffToHead", a );
    a = new KAction( i18n( "Compare to Base..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( diffToBase() ) );
    m_actions.insert( "diffToBase", a );
    a = new KAction( i18n( "Revert" ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( revert() ) );
    m_actions.insert( "revert", a );
    a = new KAction( i18n( "History..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( history() ) );
    m_actions.insert( "history", a );
    a = new KAction( i18n( "Annotation..." ), this );
    connect( a, SIGNAL( triggered() ), this, SLOT( annotation() ) );
    m_actions.insert( "annotation", a );
    a = new KAction( this );
    a->setSeparator( true );
    m_actions.insert( "commitseperator", a);
    a = new KAction( this );
    a->setSeparator( true );
    m_actions.insert( "logseperator", a);
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
        KDevelop::ProjectBaseItem* parentitem = dynamic_cast<KDevelop::ProjectBaseItem*>( item->parent() );
        if( parentitem )
        {
            url = parentitem->folder()->url();
        }
    }
    return url;

}

KDevelop::ContextMenuExtension KDevVcsCommonPlugin::contextMenuExtension( KDevelop::Context* context )
{
    /* TODO: the following logic to determine which actions need to be enabled
     * or disabled does not work properly. What needs to be implemented is that
     * project items that are vc-controlled enable all except add, project 
     * items that are not vc-controlled enable add action. For urls that cannot
     * be made into a project item, or if the project has no associated VC 
     * plugin we need to check whether a VC controls the parent dir, if we have 
     * one we assume the urls can be added but are not currently controlled. If
     * the url is already version controlled then just enable all except add
     */
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
                    m_ctxUrls[plugin].append( urlForItem( item ) );
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
                m_ctxUrls[plugin].append( editctx->url() );
            }
        }
    }
    {
        KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>( context );
        if( filectx )
        {
            foreach( const KUrl &url, filectx->urls() )
            {
                KDevelop::IPlugin* plugin = findVcsPluginForUrl( url );
                if( plugin )
                {
                    m_ctxUrls[plugin].append( url );
                }
            }
        }
    }
    if(m_ctxUrls.isEmpty())
        return menuExt;

    foreach( const QString& id, m_actions.keys() )
    {
        if( ( id == "history" || id == "annotation" || id == "diffToHead" || id == "diffToBase" ) )
        {
            m_actions[id]->setEnabled( m_ctxUrls.count() == 1 && m_ctxUrls.begin().value().count() == 1 );
        } else
        {
            m_actions[id]->setEnabled( !m_ctxUrls.isEmpty() );
        }
    }

    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("commit") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("update") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("commitseperator") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("add") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("remove") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("revert") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("logseperator") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("diffToBase") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("diffToHead") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("history") );
    menuExt.addAction( KDevelop::ContextMenuExtension::VcsGroup, m_actions.value("annotation") );
    return menuExt;
}

#define EXECUTE_VCS_METHOD( method ) \
foreach( KDevelop::IPlugin* plugin, m_ctxUrls.keys() ) \
{ \
    KDevelop::IBasicVersionControl* iface = plugin->extension<KDevelop::IBasicVersionControl>(); \
    core()->runController()->registerJob( iface-> method ( m_ctxUrls.value(plugin) ) ); \
}

#define SINGLEURL_SETUP_VARS \
KDevelop::IPlugin* plugin =  m_ctxUrls.keys().at(0); \
KDevelop::IBasicVersionControl* iface = plugin->extension<KDevelop::IBasicVersionControl>(); \
KUrl url = m_ctxUrls.value(plugin).at(0);


void KDevVcsCommonPlugin::revert()
{
    EXECUTE_VCS_METHOD( revert );
}

void KDevVcsCommonPlugin::diffToHead()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob* job = iface->diff( url,
                                         KDevelop::VcsRevision::createSpecialRevision( KDevelop::VcsRevision::Head ),
                                         KDevelop::VcsRevision::createSpecialRevision( KDevelop::VcsRevision::Working ) );

    connect( job, SIGNAL( finished( KJob* ) ), this, SLOT( diffJobFinished( KJob* ) ) );
    core()->runController()->registerJob( job );
}

void KDevVcsCommonPlugin::diffJobFinished( KJob* job )
{
    KDevelop::VcsJob* vcsjob = dynamic_cast<KDevelop::VcsJob*>( job );
    Q_ASSERT(vcsjob);
    if( vcsjob )
    {
        if( vcsjob->status() == KDevelop::VcsJob::JobSucceeded )
        {
            KDevelop::VcsDiff d = vcsjob->fetchResults().value<KDevelop::VcsDiff>();
            QString diff = d.diff();
            core()->documentController()->openDocumentFromText( diff );
        } else
        {
            KMessageBox::error( core()->uiController()->activeMainWindow(), vcsjob->errorString(), i18n("Unable to get difference.") );
        }

        vcsjob->disconnect( this );
    }
}

void KDevVcsCommonPlugin::diffToBase()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob* job = iface->diff( url,
                                         KDevelop::VcsRevision::createSpecialRevision( KDevelop::VcsRevision::Base ),
                                         KDevelop::VcsRevision::createSpecialRevision( KDevelop::VcsRevision::Working ) );

    connect( job, SIGNAL( finished( KJob* ) ), this, SLOT( diffJobFinished( KJob* ) ) );
    core()->runController()->registerJob( job );
}

void KDevVcsCommonPlugin::history()
{
    SINGLEURL_SETUP_VARS
    KDevelop::VcsJob *job = iface->log( url );
    KDialog* dlg = new KDialog();
    dlg->setButtons( KDialog::Close );
    dlg->setCaption( i18n( "%2 History (%1)", url.pathOrUrl(), iface->name() ) );
    KDevelop::VcsEventWidget* logWidget = new KDevelop::VcsEventWidget( url, job, dlg );
    dlg->setMainWidget( logWidget );
    connect( dlg, SIGNAL( closeClicked() ), job, SLOT( deleteLater() ) );
    dlg->show();
}

void KDevVcsCommonPlugin::annotation()
{
    SINGLEURL_SETUP_VARS
    KDevelop::IDocument* doc = core()->documentController()->documentForUrl( url );
    if( !doc )
        doc = core()->documentController()->openDocument( url );

    if( doc && doc->textDocument() )
    {
        KDevelop::VcsJob* job = iface->annotate( url );
        KTextEditor::AnnotationInterface* annotateiface = qobject_cast<KTextEditor::AnnotationInterface*>(doc->textDocument());
        KTextEditor::AnnotationViewInterface* viewiface = qobject_cast<KTextEditor::AnnotationViewInterface*>(doc->textDocument()->activeView());
        if( annotateiface && viewiface )
        {
            KDevelop::VcsAnnotationModel* model = new KDevelop::VcsAnnotationModel( job, url, doc->textDocument() );
            annotateiface->setAnnotationModel( model );
            viewiface->setAnnotationBorderVisible( true );
        }else
        {
            KMessageBox::error( 0, i18n( "Cannot display annotations, missing interface KTextEditor::AnnotationInterface for the editor." ) );
            delete job;
        }
    }else
    {
        KMessageBox::error( 0, i18n("Cannot execute annotate action because the "
        "document was not found, or was not a text document:\n%1", url.pathOrUrl() ) );
    }
}

void KDevVcsCommonPlugin::update()
{
    EXECUTE_VCS_METHOD( update )
}

void KDevVcsCommonPlugin::remove()
{
    EXECUTE_VCS_METHOD( remove )
}

void KDevVcsCommonPlugin::add()
{
    EXECUTE_VCS_METHOD( add )
}

void KDevVcsCommonPlugin::commit()
{
    Q_ASSERT( !m_ctxUrls.isEmpty() );
    foreach( KDevelop::IPlugin* plugin, m_ctxUrls.keys() )
    {
        KDevelop::VcsCommitDialog* dlg = new KDevelop::VcsCommitDialog( plugin, core()->uiController()->activeMainWindow() );
        dlg->setCommitCandidates( m_ctxUrls.value( plugin ) );
        KConfigGroup vcsGroup( KSharedConfig::openConfig( componentData() ), "VcsCommon" );
        dlg->setOldMessages( vcsGroup.readEntry( "OldCommitMessages", QStringList() ) );
        dlg->setRecursive( true );
        connect(dlg, SIGNAL(doCommit(KDevelop::VcsCommitDialog*)), this, SLOT(executeCommit(KDevelop::VcsCommitDialog*)));
        connect(dlg, SIGNAL(cancelCommit(KDevelop::VcsCommitDialog*)), this, SLOT(cancelCommit(KDevelop::VcsCommitDialog*)));
        dlg->show();
    }
}

void KDevVcsCommonPlugin::executeCommit( KDevelop::VcsCommitDialog* dlg )
{
    KConfigGroup vcsGroup( KSharedConfig::openConfig( componentData() ), "VcsCommon" );
    QStringList oldMessages = vcsGroup.readEntry( "OldCommitMessages", QStringList() );
    oldMessages << dlg->message();
    vcsGroup.writeEntry("OldCommitMessages", oldMessages);

    KDevelop::IBasicVersionControl* iface = dlg->versionControlPlugin()->extension<KDevelop::IBasicVersionControl>();
    core()->runController()->registerJob( iface->commit( dlg->message(), dlg->checkedUrls(),
         dlg->recursive() ?  KDevelop::IBasicVersionControl::Recursive : KDevelop::IBasicVersionControl::NonRecursive ) );

    dlg->deleteLater();
}

void KDevVcsCommonPlugin::cancelCommit( KDevelop::VcsCommitDialog* dlg )
{
    dlg->deleteLater();
}

#include "kdevvcscommonplugin.moc"
