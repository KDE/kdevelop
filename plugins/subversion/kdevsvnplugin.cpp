/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevsvnplugin.h"

#include <QtDesigner/QExtensionFactory>
#include <QAction>
#include <QVariant>
#include <QTextStream>

#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kurlrequesterdialog.h>
#include <kfile.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>

#include <iuicontroller.h>
#include <idocumentcontroller.h>
#include <idocument.h>
#include <iplugincontroller.h>
#include <icore.h>
#include <ioutputview.h>
#include <projectmodel.h>
#include <context.h>

#include <svncpp/apr.hpp>

#include "svncommitjob.h"
#include "svnstatusjob.h"
#include "svnaddjob.h"
#include "svnrevertjob.h"
#include "svnremovejob.h"
#include "svnupdatejob.h"
#include "svninfojob.h"
#include "svndiffjob.h"
#include "svncopyjob.h"
#include "svnmovejob.h"
#include "svnlogjob.h"
#include "svnblamejob.h"

#include "svnoutputdelegate.h"
#include "svnoutputmodel.h"
#include "svncommitdialog.h"
#include "svnlogwidget.h"
#include "svnannotationwidget.h"
#include "svndiffwidget.h"
#include "svnimportmetadatawidget.h"

K_PLUGIN_FACTORY(KDevSvnFactory, registerPlugin<KDevSvnPlugin>(); )
K_EXPORT_PLUGIN(KDevSvnFactory("kdevsubversion"))

class KDevSvnViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevSvnViewFactory(KDevSvnPlugin *part): m_part(part) {}
    virtual QWidget* create(QWidget */*parent*/ = 0)
    {
        return 0;
//         return new KDevSvnView(m_part, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    KDevSvnPlugin *m_part;
    svn::Apr apr;
};


KDevSvnPlugin::KDevSvnPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin(KDevSvnFactory::componentData(), parent)
    , m_outputmodel(0), m_outputdelegate(0), m_factory(0)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBasicVersionControl)
//     m_factory = new KDevSvnViewFactory(this);
//     core()->uiController()->addToolView("Subversion", m_factory);
    qRegisterMetaType<KDevelop::VcsStatusInfo>();
    qRegisterMetaType<SvnInfoHolder>();
    qRegisterMetaType<KDevelop::VcsEvent>();
    qRegisterMetaType<KDevelop::VcsRevision>();
    qRegisterMetaType<KDevelop::VcsAnnotation>();
    qRegisterMetaType<KDevelop::VcsAnnotationLine>();
    m_outputmodel = new SvnOutputModel( this, this );

    IPlugin* plugin = core()->pluginController()->pluginForExtension( "org.kdevelop.IOutputView" );
    Q_ASSERT( plugin );
    if( plugin )
    {
        m_outputdelegate = new SvnOutputDelegate( this );
        KDevelop::IOutputView* iface = plugin->extension<KDevelop::IOutputView>();
        int id = iface->registerView( "Subversion", KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
        iface->setModel( id, m_outputmodel );
        iface->setDelegate( id, m_outputdelegate );
    }
}

KDevSvnPlugin::~KDevSvnPlugin()
{
}

bool KDevSvnPlugin::isVersionControlled( const KUrl& localLocation )
{
    SvnInfoJob* job = new SvnInfoJob( this );

    job->setLocation( localLocation );
    if( job->exec() )
    {
        QVariant result = job->fetchResults();
        if( result.isValid() )
        {
            SvnInfoHolder h = qVariantValue<SvnInfoHolder>( result );
            return !h.name.isEmpty();
        }
    }else
    {
        kDebug(9510) << "Couldn't execute job";
    }
    return false;
}

KDevelop::VcsJob* KDevSvnPlugin::repositoryLocation( const KUrl& localLocation )
{
    SvnInfoJob* job = new SvnInfoJob( this );

    job->setLocation( localLocation );
    job->setProvideInformation( SvnInfoJob::RepoUrlOnly );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::status( const KUrl::List& localLocations,
                                    KDevelop::IBasicVersionControl::RecursionMode mode )
{
    SvnStatusJob* job = new SvnStatusJob( this );
    job->setLocations( localLocations );
    job->setRecursive( ( mode == KDevelop::IBasicVersionControl::Recursive ) );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::add( const KUrl::List& localLocations,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    SvnAddJob* job = new SvnAddJob( this );
    job->setLocations( localLocations );
    job->setRecursive( ( recursion == KDevelop::IBasicVersionControl::Recursive ) );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::remove( const KUrl::List& localLocations )
{
    SvnRemoveJob* job = new SvnRemoveJob( this );
    job->setLocations( localLocations );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::edit( const KUrl& /*localLocation*/ )
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::unedit( const KUrl& /*localLocation*/ )
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::localRevision( const KUrl& localLocation, KDevelop::VcsRevision::RevisionType type )
{
    SvnInfoJob* job = new SvnInfoJob( this );

    job->setLocation( localLocation );
    job->setProvideInformation( SvnInfoJob::RevisionOnly );
    job->setProvideRevisionType( type );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::copy( const KUrl& localLocationSrc, const KUrl& localLocationDstn )
{
    SvnCopyJob* job = new SvnCopyJob( this );
    job->setSourceLocation( localLocationSrc );
    job->setDestinationLocation( localLocationDstn );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::move( const KUrl& localLocationSrc, const KUrl& localLocationDst )
{
    SvnMoveJob* job = new SvnMoveJob( this );
    job->setSourceLocation( localLocationSrc );
    job->setDestinationLocation( localLocationDst );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::revert( const KUrl::List& localLocations,
                                               KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    SvnRevertJob* job = new SvnRevertJob( this );
    job->setLocations( localLocations );
    job->setRecursive( ( recursion == KDevelop::IBasicVersionControl::Recursive ) );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::update( const KUrl::List& localLocations,
                                    const KDevelop::VcsRevision& rev,
                                    KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    SvnUpdateJob* job = new SvnUpdateJob( this );
    job->setLocations( localLocations );
    job->setRevision( rev );
    job->setRecursive( ( recursion == KDevelop::IBasicVersionControl::Recursive ) );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::commit( const QString& message, const KUrl::List& localLocations,
                    KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    SvnCommitJob* job = new SvnCommitJob( this );
    kDebug(9510) << "Commiting locations:" << localLocations << endl;
    job->setUrls( localLocations );
    job->setCommitMessage( message ) ;
    job->setRecursive( ( recursion == KDevelop::IBasicVersionControl::Recursive ) );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::diff( const KDevelop::VcsLocation& src,
                const KDevelop::VcsLocation& dst,
                const KDevelop::VcsRevision& srcRevision,
                const KDevelop::VcsRevision& dstRevision,
                KDevelop::VcsDiff::Type diffType,
                KDevelop::IBasicVersionControl::RecursionMode recurse )
{
    SvnDiffJob* job = new SvnDiffJob( this );
    job->setSource( src );
    job->setDestination( dst );
    job->setSrcRevision( srcRevision );
    job->setDstRevision( dstRevision );
    job->setDiffType( diffType );
    job->setRecursive( ( recurse == KDevelop::IBasicVersionControl::Recursive ) );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::log( const KUrl& localLocation, const KDevelop::VcsRevision& rev, unsigned long limit )
{
    SvnLogJob* job = new SvnLogJob( this );
    job->setLocation( localLocation );
    job->setStartRevision( rev );
    job->setLimit( limit );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::log( const KUrl& localLocation,
            const KDevelop::VcsRevision& startRev,
            const KDevelop::VcsRevision& endRev )
{
    SvnLogJob* job = new SvnLogJob( this );
    job->setLocation( localLocation );
    job->setStartRevision( startRev );
    job->setEndRevision( endRev );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::annotate( const KUrl& localLocation,
                const KDevelop::VcsRevision& rev )
{
    SvnBlameJob* job = new SvnBlameJob( this );
    job->setLocation( localLocation );
    job->setEndRevision( rev );
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::merge( const KDevelop::VcsLocation& localOrRepoLocationSrc,
            const KDevelop::VcsLocation& localOrRepoLocationDst,
            const KDevelop::VcsRevision& srcRevision,
            const KDevelop::VcsRevision& dstRevision,
            const KUrl& localLocation )
{
    // TODO implement merge
    Q_UNUSED( localOrRepoLocationSrc )
    Q_UNUSED( localOrRepoLocationDst )
    Q_UNUSED( srcRevision )
    Q_UNUSED( dstRevision )
    Q_UNUSED( localLocation )
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::resolve( const KUrl::List& /*localLocations*/,
                KDevelop::IBasicVersionControl::RecursionMode /*recursion*/ )
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::import( const KDevelop::VcsMapping& localLocation, const QString& commitMessage )
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::checkout( const KDevelop::VcsMapping& mapping )
{
    // TODO understand vcsmapping, implement checkoutjob
    Q_UNUSED( mapping )
    return 0;
}


const KUrl KDevSvnPlugin::urlFocusedDocument()
{
    KParts::ReadOnlyPart *part =
            dynamic_cast<KParts::ReadOnlyPart*>( core()->partManager()->activePart() );
    if ( part ) {
        if (part->url().isLocalFile() ) {
            return part->url();
        }
    }
    return KUrl();
}

QPair<QString,QList<QAction*> > KDevSvnPlugin::requestContextMenuActions( KDevelop::Context* context )
{
    if( context->type() == KDevelop::Context::ProjectItemContext )
    {
        KDevelop::ProjectItemContext *itemCtx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if( itemCtx )
        {
            QList<KDevelop::ProjectBaseItem *> baseItemList = itemCtx->items();

            // hook special case -- just one item was requested for folder context menu.
    //         if( baseItemList.count() == 1 && baseItemList.first()->folder() ){
    //             KDevelop::ProjectFolderItem *folderItem =
    //                     dynamic_cast<KDevelop::ProjectFolderItem*>( baseItemList.first()->folder() );
    //
    //             if( !isVersionControlled( folderItem->url() ) ){
    //                 // checkout only can be done under the non-vcs dir.
    //                 m_ctxUrlList << folderItem->url();
    //                 QList<QAction*> actions;
    //                 QAction *action = new QAction(i18n("Checkout.."), this);
    //                 connect( action, SIGNAL(triggered()), this, SLOT(ctxCheckout()) );
    //                 actions << action;
    //                 return qMakePair( QString("Subversion"), actions );
    //             }
    //         }

            // now general case
            KUrl::List ctxUrlList;
            foreach( KDevelop::ProjectBaseItem* _item, baseItemList )
            {
                if( _item->folder() ){
                    KDevelop::ProjectFolderItem *folderItem = dynamic_cast<KDevelop::ProjectFolderItem*>(_item);
                    ctxUrlList << folderItem->url();
                }
                else if( _item->file() ){
                    KDevelop::ProjectFileItem *fileItem = dynamic_cast<KDevelop::ProjectFileItem*>(_item);
                    ctxUrlList << fileItem->url();
                }
            }

            if( ctxUrlList.isEmpty() )
                return KDevelop::IPlugin::requestContextMenuActions( context );

            m_ctxUrlList = ctxUrlList;
        }
    }else if( context->type() == KDevelop::Context::EditorContext )
    {
        KDevelop::EditorContext *itemCtx = dynamic_cast<KDevelop::EditorContext*>(context);
        m_ctxUrlList << itemCtx->url();
    }else if( context->type() == KDevelop::Context::FileContext )
    {
        KDevelop::FileContext *itemCtx = dynamic_cast<KDevelop::FileContext*>(context);
        m_ctxUrlList += itemCtx->urls();
    }
    QList<QAction*> actions;
    QAction *action;

    action = new QAction(i18n("Commit..."), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxCommit()) );
    actions << action;

    action = new QAction(i18n("Add to Repository"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxAdd()) );
    actions << action;

    action = new QAction(i18n("Remove from Repository"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxRemove()) );
    actions << action;

    action = new QAction(i18n("Update to Head"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxUpdate()) );
    actions << action;

    action = new QAction(i18n("Revert"), this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxRevert()) );
    actions << action;

    action = new QAction("Diff to Head", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxDiffHead()) );
    actions << action;

    action = new QAction("Diff to Base", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxDiffBase()) );
    actions << action;

    action = new QAction("Copy...", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxCopy()) );
    actions << action;

    action = new QAction("Move...", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxMove()) );
    actions << action;

    action = new QAction("History...", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxHistory()) );
    actions << action;

    action = new QAction("Annotation...", this);
    connect( action, SIGNAL(triggered()), this, SLOT(ctxBlame()) );
    actions << action;


//         action = new QAction(i18n("Blame/Annotate..."), this);
//         connect( action, SIGNAL(triggered()), this, SLOT(ctxBlame()) );
//         actions << action;
//
//         action = new QAction(i18n("Cat..."), this);
//         connect( action, SIGNAL(triggered()), this, SLOT(ctxCat()) );
//         actions << action;
//
//         action = new QAction(i18n("Diff to..."), this);
//         connect( action, SIGNAL(triggered()), this, SLOT(ctxDiff()) );
//         actions << action;
//
//         action = new QAction(i18n("Information..."), this);
//         connect( action, SIGNAL(triggered()), this, SLOT(ctxInfo()) );
//         actions << action;
//
//         action = new QAction(i18n("Status..."), this);
//         connect( action, SIGNAL(triggered()), this, SLOT(ctxStatus()) );
//         actions << action;


    if( !m_ctxUrlList.isEmpty() )
    {
        return qMakePair( QString("Subversion"), actions );
    }

    return KDevelop::IPlugin::requestContextMenuActions( context );
}

void KDevSvnPlugin::ctxHistory()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
    KDevelop::VcsRevision start;
    start.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Head ),
                            KDevelop::VcsRevision::Special );
    KDevelop::VcsJob *job = log( m_ctxUrlList.first(), start, 0 );
    KDialog* dlg = new KDialog();
    dlg->setButtons( KDialog::Close );
    dlg->setCaption( i18n( "Subversion Log (%1)", m_ctxUrlList.first().path() ) );
    SvnLogWidget* logWidget = new SvnLogWidget( m_ctxUrlList.first(), job, dlg );
    dlg->setMainWidget( logWidget );
    connect( dlg, SIGNAL( destroyed( QObject* ) ), job, SLOT( deleteLater() ) );
    dlg->show();
}
void KDevSvnPlugin::ctxBlame()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }

    if( m_ctxUrlList.first().isLocalFile() )
    {
        KMessageBox::error( 0, i18n("Annotate is only supported on local files") );
        return;
    }

    KDevelop::IDocument* doc = core()->documentController()->documentForUrl( m_ctxUrlList.first() );
    if( !doc )
        doc = core()->documentController()->openDocument( m_ctxUrlList.first() );

    if( doc && doc->textDocument() )
    {
        KDevelop::VcsRevision head;
        head.setRevisionValue( qVariantFromValue( KDevelop::VcsRevision::Head ),
                            KDevelop::VcsRevision::Special );
        KDevelop::VcsJob* job = annotate( m_ctxUrlList.first(), head );
        KTextEditor::MarkInterface* markiface = 0;
                //qobject_cast<KTextEditor::MarkInterface*>(doc->textDocument());
        if( markiface )
        {
            //@TODO: Work with Kate devs towards a new interface for adding
            //       annotation information to the KTE's in KDE 4.1
        }else
        {
            KDialog* dlg = new KDialog();
            dlg->setButtons( KDialog::Close );
            dlg->setCaption( i18n("Annotation (%1)", m_ctxUrlList.first().prettyUrl() ) );
            SvnAnnotationWidget* w = new SvnAnnotationWidget( m_ctxUrlList.first(), job, dlg );
            dlg->setMainWidget( w );
            connect( dlg, SIGNAL( destroyed( QObject* ) ), job, SLOT( deleteLater() ) );
            dlg->show();
        }
    }else
    {
        KMessageBox::error( 0, i18n("Cannot execute annotate action because the "
                                    "document wasn't found or was not a text "
                                    "document:\n%1", m_ctxUrlList.first().prettyUrl() ) );
    }

}
void KDevSvnPlugin::ctxCommit()
{
    if( !m_ctxUrlList.isEmpty() )
    {
        SvnCommitDialog* dlg = new SvnCommitDialog( this );
        dlg->setCommitCandidates( m_ctxUrlList );
        dlg->setRecursive( true );
        connect( dlg, SIGNAL( okClicked( SvnCommitDialog* ) ), this, SLOT( doCommit( SvnCommitDialog* ) ) );
        connect( dlg, SIGNAL( cancelClicked( SvnCommitDialog* ) ), this, SLOT( cancelCommit( SvnCommitDialog* ) ) );
        dlg->show();
    }
}


void KDevSvnPlugin::doCommit( SvnCommitDialog* dlg )
{
    KDevelop::IBasicVersionControl::RecursionMode mode;
    if( dlg->recursive() )
    {
        mode = KDevelop::IBasicVersionControl::Recursive;
    }else
    {
        mode = KDevelop::IBasicVersionControl::NonRecursive;
    }
    KDevelop::VcsJob* job = commit( dlg->message(), dlg->checkedUrls(), mode );
    job->exec();
    delete job;
    dlg->deleteLater();
}

void KDevSvnPlugin::cancelCommit( SvnCommitDialog* dlg )
{
    dlg->deleteLater();
}

void KDevSvnPlugin::ctxUpdate()
{
    KDevelop::VcsRevision rev;
    rev.setRevisionValue( QVariant( KDevelop::VcsRevision::Head ), KDevelop::VcsRevision::Special );
    KDevelop::VcsJob* job = update( m_ctxUrlList, rev );
    job->exec();
    delete job;
}

void KDevSvnPlugin::ctxAdd()
{
    KDevelop::VcsJob* job = add( m_ctxUrlList );
    job->exec();
    delete job;
}
void KDevSvnPlugin::ctxRemove()
{
    KDevelop::VcsJob* job = remove( m_ctxUrlList );
    job->exec();
    delete job;
}

void KDevSvnPlugin::ctxRevert()
{
    //@TODO: If one of the urls is a directory maybe ask whether all files in the dir should be reverted?
    KDevelop::VcsJob* job = revert( m_ctxUrlList );
    job->exec();
    delete job;
}

void KDevSvnPlugin::ctxCheckout()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
}

void KDevSvnPlugin::ctxDiff()
{
    // TODO correct port
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
}

void KDevSvnPlugin::ctxDiffHead()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
    KDevelop::VcsRevision srcRev,dstRev;
    srcRev.setRevisionValue( KDevelop::VcsRevision::Head, KDevelop::VcsRevision::Special );
    dstRev.setRevisionValue( KDevelop::VcsRevision::Working, KDevelop::VcsRevision::Special );
    KDevelop::VcsJob* job = diff( m_ctxUrlList.first(), m_ctxUrlList.first(), srcRev, dstRev );

    SvnDiffDialog* dlg = new SvnDiffDialog( job );
    connect( dlg, SIGNAL( destroyed( QObject* ) ), job, SLOT( deleteLater() ) );
    dlg->show();
}
void KDevSvnPlugin::ctxDiffBase()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
    KDevelop::VcsRevision srcRev,dstRev;
    srcRev.setRevisionValue( KDevelop::VcsRevision::Base, KDevelop::VcsRevision::Special );
    dstRev.setRevisionValue( KDevelop::VcsRevision::Working, KDevelop::VcsRevision::Special );
    KDevelop::VcsJob* job = diff( m_ctxUrlList.first(), m_ctxUrlList.first(), srcRev, dstRev );

    SvnDiffDialog* dlg = new SvnDiffDialog( job );
    connect( dlg, SIGNAL( destroyed( QObject* ) ), job, SLOT( deleteLater() ) );
    dlg->show();
}
void KDevSvnPlugin::ctxInfo()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
}
void KDevSvnPlugin::ctxStatus()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
}

void KDevSvnPlugin::ctxCopy()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
    KUrl source = m_ctxUrlList.first();
    if( source.isLocalFile() )
    {
        QString dir = source.path();
        bool isFile = QFileInfo( source.path() ).isFile();
        if( isFile )
        {
            dir = source.directory();
        }
        KUrlRequesterDialog dlg( dir, i18n("Destination file/directory"), 0 );
        if( isFile )
        {
            dlg.urlRequester()->setMode( KFile::File | KFile::Directory | KFile::LocalOnly );
        }else
        {
            dlg.urlRequester()->setMode( KFile::Directory | KFile::LocalOnly );
        }
        if( dlg.exec() == QDialog::Accepted )
        {
            KDevelop::VcsJob* job = copy( source, dlg.selectedUrl() );
            job->exec();
            delete job;
        }
    }else
    {
        KMessageBox::error( 0, i18n("Copying only works on local files") );
        return;
    }

}

void KDevSvnPlugin::ctxMove()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
        KUrl source = m_ctxUrlList.first();
    if( source.isLocalFile() )
    {
        QString dir = source.path();
        bool isFile = QFileInfo( source.path() ).isFile();
        if( isFile )
        {
            dir = source.directory();
        }
        KUrlRequesterDialog dlg( dir, i18n("Destination file/directory"), 0 );
        if( isFile )
        {
            dlg.urlRequester()->setMode( KFile::File | KFile::Directory | KFile::LocalOnly );
        }else
        {
            dlg.urlRequester()->setMode( KFile::Directory | KFile::LocalOnly );
        }
        if( dlg.exec() == QDialog::Accepted )
        {
            KDevelop::VcsJob* job = move( source, dlg.selectedUrl() );
            job->exec();
            delete job;
        }
    }else
    {
        KMessageBox::error( 0, i18n("Moving only works on local files/dirs") );
        return;
    }
}

void KDevSvnPlugin::ctxCat()
{
    if( m_ctxUrlList.count() > 1 ){
        KMessageBox::error( 0, i18n("Please select only one item for this operation") );
        return;
    }
}


SvnOutputModel* KDevSvnPlugin::outputModel() const
{
    return m_outputmodel;
}


QString KDevSvnPlugin::name() const
{
    return i18n("Subversion");
}

KDevelop::VcsImportMetadataWidget* KDevSvnPlugin::createImportMetadataWidget( QWidget* parent )
{
    return new SvnImportMetadataWidget( parent );
}

#include "kdevsvnplugin.moc"

