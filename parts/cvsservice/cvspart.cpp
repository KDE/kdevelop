/**************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kurl.h>
#include <kapplication.h>

#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <kgenericfactory.h>

#include <qdir.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "domutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "urlutil.h"
#include "execcommand.h"

#include "cvsutils.h"
#include "changelog.h"
#include "cvspart.h"
#include "cvsprocesswidget.h"
#include "cvsoptions.h"
#include "commitdlg.h"
#include "logform.h"
#include "cvsform.h"
#include "cvsoptionswidget.h"
#include "checkoutdialog.h"

#include <dcopref.h>
#include <repository_stub.h>
#include <cvsservice_stub.h>
#include <cvsjob_stub.h>


QStringList quoted( const QStringList &args )
{
    QStringList qNames;
    for (size_t i=0; i<args.count(); ++i)
    {
        qNames << KShellProcess::quote( args[i] );
    }
    return qNames;
}

QStringList prependToStringList( const QString &s, const QStringList &paths )
{
    QStringList l = paths;
    for (size_t i=0; i<l.count(); ++i)
    {
        l[i] = s + QDir::separator() + l[i];
    }
    return l;
}

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

const QString changeLogFileName = "ChangeLog";

///////////////////////////////////////////////////////////////////////////////
// Global vars
///////////////////////////////////////////////////////////////////////////////

// This is an ugly hack for being able to pass CVS_RSH from CvsPart::create
QString g_tempEnvRsh( "" );

///////////////////////////////////////////////////////////////////////////////
// Plugin factory
///////////////////////////////////////////////////////////////////////////////

typedef KGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvsservice, CvsFactory( "kdevcvsservice" ) );

///////////////////////////////////////////////////////////////////////////////
// class CvsPart
///////////////////////////////////////////////////////////////////////////////

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & )
    : KDevVersionControl( "KDevCvsServicePart", "kdevcvsservicepart", parent, name ? name : "CvsService" ),
    proc( 0 ),
    actionCommit( 0 ), actionDiff( 0 ), actionLog( 0 ), actionAdd( 0 ), actionRemove( 0 ),
    actionUpdate( 0 ), actionRevert( 0 ),
    actionAddToIgnoreList( 0 ), actionRemoveFromIgnoreList( 0 )
{
    setInstance( CvsFactory::instance() );

    setXMLFile( "kdevcvsservicepart.rc" );

    init();
}

///////////////////////////////////////////////////////////////////////////////

CvsPart::~CvsPart()
{
    if (m_widget)
    {
        mainWindow()->removeView( m_widget ); // Inform toplevel, that the output view is gone
    }
    delete m_widget;
    delete m_cvsConfigurationForm;
    releaseCvsService();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::init()
{
    setupActions();

    // Load / store project configuration every time the project is opened/closed
    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );

    // Context menu
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)), this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)), this, SLOT(slotStopButtonClicked(KDevPlugin*)) );

    requestCvsService();

    m_widget = new CvsProcessWidget( appId, this, 0, "cvsprocesswidget" );

    mainWindow()->embedOutputView( m_widget, i18n("CvsService"), i18n("cvs output") );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::setupActions()
{

    KAction * action = new KAction( i18n("CVS repository"), 0, this, SLOT(slotCheckOut()),
        actionCollection(), "cvsservice_checkout" );
    action->setStatusText( i18n("Check-out from an existing CVS repository") );
    actionCommit = new KAction( i18n("Commit"), 0, this, SLOT(slotActionCommit()),
        actionCollection(), "cvsservice_commit" );
    actionDiff = new KAction( i18n("Diff"), 0, this, SLOT(slotActionDiff()),
        actionCollection(), "cvsservice_diff" );
    actionLog = new KAction( i18n("Log"), 0, this, SLOT(slotActionLog()),
        actionCollection(), "cvsservice_log" );
    actionAdd = new KAction( i18n("Add"), 0, this, SLOT(slotActionAdd()),
        actionCollection(), "cvsservice_add" );
    actionRemove = new KAction( i18n("Remove From Repository"), 0, this, SLOT(slotActionRemove()),
        actionCollection(), "cvsservice_remove" );
    actionUpdate = new KAction( i18n("Update"), 0, this, SLOT(slotActionUpdate()),
        actionCollection(), "cvsservice_update" );
    actionRevert = new KAction( i18n("Replace with Copy From Repository"), 0, this, SLOT(slotActionRevert()),
        actionCollection(), "cvsservice_revert" );
    actionAddToIgnoreList = new KAction( i18n("Ignore this file when doing cvs operation"), 0,
        this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "cvsservice_ignore" );
    actionRemoveFromIgnoreList = new KAction( i18n("Do not Ignore this file when doing cvs operation"), 0,
        this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "cvsservice_donot_ignore" );
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::requestCvsService()
{
    QString error;

    if (KApplication::startServiceByDesktopName( "cvsservice", QStringList(), &error, &appId ))
    {
        kdDebug() << "Starting cvsservice failes with message: " << error << endl;
        return false;
    }
    else
    {
        m_cvsService = new CvsService_stub( appId, "CvsService" );
    }

    // create stub for repository
    m_repository = new Repository_stub( appId, "CvsRepository" );

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::releaseCvsService()
{
    if (m_cvsService)
        m_cvsService->quit();
    delete m_cvsService;
    delete m_repository;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::urlFocusedDocument( KURL &url )
{
    kdDebug(9000) << "CvsPart::retrieveUrlFocusedDocument() here!" << endl;
    KParts::ReadOnlyPart *part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
    if ( part )
    {
        if (part->url().isLocalFile() )
        {
            url = part->url();
            return true;
        }
        else
        {
            kdDebug(9027) << "Cannot handle non-local files!" << endl;
        }
    }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::prepareOperation( CvsOperation op )
{
    if (!project())
    {
        kdDebug(9000) << "CvsPart::prepareOperation(): No project???" << endl;
        KMessageBox::sorry( 0, i18n("Open a project first.\nOperation will be aborted.") );
        return false;
    }

    CvsUtils::validateURLs( project()->projectDirectory(),  urls, op );
    if (urls.count() <= 0) // who knows? ;)
    {
        kdDebug(9000) << "CvsPart::prepareOperation(): No valid document URL selected!!!" << endl;
        KMessageBox::sorry( 0, i18n("None of the file(s) you selected seems to be valid for repository.") );
        return false;
    }
    URLUtil::dump( urls );
    // FIXME: Commented because it may break slotLog() and perhaps some another worker
//    quote( this->urls );

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::doneOperation()
{
    urls.clear(); // Ok, clean-up file list
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotCheckOut()
{
    (new CheckoutDialog(0))->show();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::createNewProject( const QString& dirName )
{
    kdDebug( 9000 ) << "====> CvsPart::createNewProject( const QString& )" << endl;

    if (!m_cvsConfigurationForm)
        return;
    QString init("");

    // FIXME: Store rsh setting. Here doesn't store it in CvsOptions because:
    // createNewProject() is called _before_ projectOpened() signal is emitted.
    g_tempEnvRsh = m_cvsConfigurationForm->cvs_rsh->text();
    QString rsh;
    if ( !g_tempEnvRsh.isEmpty() )
        rsh = "CVS_RSH=" + KShellProcess::quote( g_tempEnvRsh );


    if (m_cvsConfigurationForm->init_check->isChecked())
    {
        QString cvs_rsh = m_cvsConfigurationForm->root_edit->text();
        init = rsh + " cvs -d " + KShellProcess::quote(cvs_rsh) + " init && ";
    }
    QString command = init + "cd " + KShellProcess::quote(dirName) +
        " && " + rsh +
        " cvs -d " + KShellProcess::quote(m_cvsConfigurationForm->root_edit->text()) +
        " import -m " + KShellProcess::quote(m_cvsConfigurationForm->message_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->repository_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->vendor_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->release_edit->text()) + " && sh " +
        locate("data","kdevcvsservice/buildcvs.sh") + " . " +
        KShellProcess::quote(m_cvsConfigurationForm->repository_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->root_edit->text());

    kdDebug( 9000 ) << "  ** Will run the following command: " << endl << command << endl;
    kdDebug( 9000 ) << "  ** on directory: " << dirName << endl;

    makeFrontend()->queueCommand( dirName, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::contextMenu( QPopupMenu *popup, const Context *context )
{
    if (context->hasType( "file" ))
    {
        kdDebug(9000) << "contextMenu()" << endl;

        const FileContext *fcontext = static_cast<const FileContext*>( context );

        // THis stuff should end up into prepareOperation()
        urls = fcontext->urls();
        URLUtil::dump( urls );
        // FIXME: Here we currently avoid context menu on document because there is no document
        // selected and we'll need to connect slotAction*() instead of these.
        if (urls.count() <= 0)
            return;

        KPopupMenu *subMenu = new KPopupMenu( popup );
//        subMenu->insertTitle( i18n("Available actions") );

        subMenu->insertItem( actionCommit->text(), this, SLOT(slotCommit()) );
        subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
        subMenu->insertItem( actionAdd->text(), this, SLOT(slotAdd()) );
        subMenu->insertItem( actionRemove->text(), this, SLOT(slotRemove()) );
        subMenu->insertItem( actionRevert->text(), this, SLOT(slotRevert()) );

        // CvsService let to do log and diff operations only on one file (or directory) at time
        if (urls.count() == 1)
        {
            subMenu->insertSeparator();
            subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
            subMenu->insertItem( actionDiff->text(), this, SLOT(slotDiff()) );
        }

        subMenu->insertSeparator();
        subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );

        // Now insert in parent menu
        popup->insertSeparator();
        popup->insertItem( i18n("CvsService"), subMenu );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionCommit()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        commit( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionUpdate()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        update( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionAdd()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        add( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionRemove()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        remove( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionRevert()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        revert( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionLog()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        log( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionDiff()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        diff( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionAddToIgnoreList()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        addToIgnoreList( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotActionRemoveFromIgnoreList()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        removeFromIgnoreList( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotCommit()
{
    commit( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotUpdate()
{
    update( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAdd()
{
    add( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemove()
{
    remove( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRevert()
{
    revert( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotLog()
{
    log( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotDiff()
{
    diff( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAddToIgnoreList()
{
    addToIgnoreList( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemoveFromIgnoreList()
{
    removeFromIgnoreList( urls );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotDiffFinished( bool normalExit, int exitStatus )
{
    Q_ASSERT( proc );

    core()->running( this, false );

    QString diff = m_widget->output(),
        err = m_widget->errors();

    kdDebug( 9999 ) << "diff = " << diff << endl;
    kdDebug( 9999 ) << "err = " << err << endl;

    if (normalExit)
        kdDebug( 9999 ) << " *** Process died nicely with exit status = " << exitStatus << endl;
    else
        kdDebug( 9999 ) << " *** Process was killed with exit status = " << exitStatus << endl;

    // delete proc
    delete proc; proc = 0;

    // Now show a message about operation ending status
    if (diff.isEmpty() && (exitStatus != 0))
    {
        KMessageBox::information( 0, i18n("Operation aborted (process killed)"), i18n("CVS Diff") );
        return;
    }
    if ( diff.isEmpty() && !err.isEmpty() )
    {
        KMessageBox::detailedError( 0, i18n("CVS outputted errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() )
    {
        int s = KMessageBox::warningContinueCancelList( 0, i18n("CVS outputted errors during diff. Do you still want to continue?"),
                QStringList::split( "\n", err, false ), i18n("Errors During Diff") );
        if ( s != KMessageBox::Continue )
            return;
    }

    if ( diff.isEmpty() )
    {
        KMessageBox::information( 0, i18n("There is no difference to the repository"), i18n("No Difference found") );
        return;
    }

    Q_ASSERT( diffFrontend() );
    diffFrontend()->showDiff( diff );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotStopButtonClicked( KDevPlugin* which )
{
    if ( which != 0 && which != this )
        return;

    m_widget->cancelJob();
/*
    if ( !proc )
        return;
    if ( !proc->kill() ) {
        KMessageBox::sorry( 0, i18n("Unable to kill process, you might want to kill it by hand.") );
        return;
    }
*/
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::commit( const KURL::List& urlList )
{
    if (!prepareOperation( opCommit ))
        return;

    kdDebug(9000) << "CvsPart::commit() here!" << endl;

    CommitDialog dlg;
    if (dlg.exec() == QDialog::Rejected)
        return;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    CvsOptions *options = CvsOptions::instance();
    QString logString = dlg.logMessage().join( "\n" );
    DCOPRef cvsJob = m_cvsService->commit( fileList, logString, false );
    if (options->rsh().isEmpty())
    {
        cvsJob.call( "setRSH", options->rsh() );
    }
    m_widget->startJob();
    connect( m_widget, SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    // 2. if requested to do so, add an entry to the Changelog too
    if (dlg.mustAddToChangeLog())
    {
        // 2.1 Modify the Changelog
        ChangeLogEntry entry;
        entry.addLines( dlg.logMessage() );
        entry.addToLog( this->project()->projectDirectory() + "/ChangeLog" );

        kdDebug( 9999 ) << " *** ChangeLog entry : " << entry.toString( "\t" ) << endl;

        // 2.2 Commit modifications (needed?)
        //command = buildCommitCmd( this->project()->projectDirectory(), changeLogFileName, dlg.logMessage() );
        //m_widget->startCommand( this->project()->projectDirectory(), command );
    }

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::update( const KURL::List& urlList )
{
    if (!prepareOperation( opUpdate ))
        return;

    kdDebug(9000) << "CvsPart::update() here" << endl;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    CvsOptions *options = CvsOptions::instance();
    DCOPRef cvsJob = m_cvsService->update( fileList, true, true, true, options->update() );
    if (options->rsh().isEmpty())
    {
        cvsJob.call( "setRSH", options->rsh() );
    }

    m_widget->startJob();
    connect( m_widget, SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::add( const KURL::List& urlList )
{
    if (!prepareOperation( opAdd ))
        return;

    kdDebug(9000) << "CvsPart::add() here" << endl;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    CvsOptions *options = CvsOptions::instance();
    // FIXME: We must commit in separate runs binary and _non_ binary files. For now we assume
    // they are _non_ binary.
    DCOPRef cvsJob = m_cvsService->add( fileList, false );
    if (options->rsh().isEmpty())
    {
        cvsJob.call( "setRSH", options->rsh() );
    }

    m_widget->startJob();
    connect( m_widget, SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::remove( const KURL::List& urlList )
{
    if (!prepareOperation( opRemove ))
        return;

    kdDebug(9000) << "CvsPart::remove() here" << endl;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    CvsOptions *options = CvsOptions::instance();
    DCOPRef cvsJob = m_cvsService->remove( fileList, true  );
    if (options->rsh().isEmpty())
    {
        cvsJob.call( "setRSH", options->rsh() );
    }

    m_widget->startJob();
    connect( m_widget, SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::revert( const KURL::List& urlList )
{
    if (!prepareOperation( opRevert ))
        return;

    kdDebug(9000) << "CvsPart::revert() here" << endl;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    CvsOptions *options = CvsOptions::instance();
    DCOPRef cvsJob = m_cvsService->update( fileList, true, true, true, options->revert() );
    if (options->rsh().isEmpty())
    {
        cvsJob.call( "setRSH", options->rsh() );
    }

    m_widget->startJob();
    connect( m_widget, SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::log( const KURL::List& urlList )
{
    if (!prepareOperation( opLog ))
        return;

    kdDebug(9000) << "CvsPart::log() here: " << endl;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    LogForm* f = new LogForm();
    f->show();
    // Form will do all the work
    f->start( m_cvsService, project()->projectDirectory(), fileList[0] );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::diff( const KURL::List& urlList )
{
    if (!prepareOperation( opDiff ))
        return;

    kdDebug(9000) << "CvsPart::diff() here" << endl;

    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    CvsOptions *options = CvsOptions::instance();
    DCOPRef cvsJob = m_cvsService->diff( fileList[0], QString::null /* revA */,
                QString::null /* revB */, options->diff(), options->contextLines() );
    if (options->rsh().isEmpty())
    {
        cvsJob.call( "setRSH", options->rsh() );
    }

    if (!m_cvsService->ok())
    {
        KMessageBox::sorry( 0, i18n("Sorry, cannot diff!"), i18n("Sorry cannot diff 2") );
        return;
    }

    m_widget->startJob( cvsJob.app(), cvsJob.obj() );
    connect( m_widget, SIGNAL(jobFinished(bool,int)), this, SLOT(slotDiffFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::addToIgnoreList( const KURL::List& urlList )
{
    CvsUtils::addToIgnoreList( project()->projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::removeFromIgnoreList( const KURL::List& urlList )
{
    CvsUtils::removeFromIgnoreList( project()->projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAddFilesToProject( const QStringList &filesToAdd )
{
    kdDebug( 9000 ) << "====> CvsPart::slotAddFilesToProject(const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want to be added to CVS repository too?"),
        i18n("CVS - New Files added to project ..."),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenAddingNewFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9000 ) << "Adding these files: " << filesToAdd.join( ", " ) << endl;

        urls = KURL::List( prependToStringList( project()->projectDirectory(), filesToAdd ) );
        URLUtil::dump( urls );
        add( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemovedFilesFromProject(const QStringList &fileToRemove)
{
    kdDebug( 9000 ) << "====> CvsPart::slotRemovedFilesFromProject( const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want them to be removed from CVS repository too?\nWarning: They will be removed from disk too!"),
        i18n("CVS - Files removed from project ..."),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenRemovingFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9000 ) << "Removing these files: " << fileToRemove.join( ", " ) << endl;

        urls = KURL::List( prependToStringList( project()->projectDirectory(), fileToRemove ) );
        URLUtil::dump( urls );
        remove( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotProjectOpened()
{
    kdDebug(9000) << "CvsPart::slotProjectOpened() here!" << endl;

    CvsOptions *options = CvsOptions::instance();
    options->load( *projectDom() );

    // If createNewProject() has set this var then we have to get it.
    if (!g_tempEnvRsh.isEmpty())
    {
        options->setRsh( g_tempEnvRsh );
        // Reset so next since this var is plugin global and may affect other
        // projects that could be loaded after the current one
        g_tempEnvRsh = "";
    }

    // When files are added to project they may be added to/removed from repository too
    connect( project(), SIGNAL(addedFilesToProject(const QStringList&)), this, SLOT(slotAddFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList&)), this, SLOT(slotRemovedFilesFromProject(const QStringList &)) );

    // set directory of working copy
    m_repository->setWorkingCopy( project()->projectDirectory() );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotProjectClosed()
{
    kdDebug(9000) << "CvsPart::slotProjectClosed() here!" << endl;

    CvsOptions *options = CvsOptions::instance();
    options->save( *projectDom() );
    delete options;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotJobFinished( bool normalExit, int exitStatus )
{
    // TODO: Display something?
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox = dlg->addVBoxPage( i18n("CvsService") );
    CvsOptionsWidget *w = new CvsOptionsWidget( (QWidget *)vbox, "cvs config widget" );
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

///////////////////////////////////////////////////////////////////////////////

QWidget* CvsPart::newProjectWidget( QWidget *parent )
{
    m_cvsConfigurationForm = new CvsForm( parent, "cvsform" );
    return m_cvsConfigurationForm;
}

#include "cvspart.moc"

