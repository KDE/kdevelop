/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmessagebox.h>

#include <qdir.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmainwindow.h>
#include <dcopref.h>
#include <repository_stub.h>
#include <cvsservice_stub.h>
#include <cvsjob_stub.h>

#include <urlutil.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevcore.h>
#include <kdevdifffrontend.h>
#include <kdevmakefrontend.h>

#include "cvsprocesswidget.h"
#include "checkoutdialog.h"
#include "commitdlg.h"
#include "tagdialog.h"
#include "diffdialog.h"
#include "releaseinputdialog.h"
#include "cvslogdialog.h"

#include "changelog.h"
#include "cvsoptions.h"
#include "cvspart.h"
#include "cvsserviceimpl.h"

///////////////////////////////////////////////////////////////////////////////
// class CvsServiceImpl
///////////////////////////////////////////////////////////////////////////////


CvsServiceImpl::CvsServiceImpl( CvsServicePart *part, const char *name )
    : CvsServicePartImpl( part, name? name : "cvsserviceimpl" )
{
    if (requestCvsService())
    {
        m_widget = new CvsProcessWidget( m_cvsService, part, 0, "cvsprocesswidget" );
    }
    else
    {
        kdDebug() << "CvsServiceImpl::CvsServiceImpl(): somebody kills me because"
            "I could not request a valid CvsService!!!! :-((( " << endl;
    }

    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
}

///////////////////////////////////////////////////////////////////////////////

CvsServiceImpl::~CvsServiceImpl()
{
    releaseCvsService();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::login()
{
    DCOPRef job = m_cvsService->login( this->projectDirectory() );

    processWidget()->startJob( job );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::logout()
{
    DCOPRef job = m_cvsService->logout( this->projectDirectory() );

    processWidget()->startJob( job );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::checkout()
{
    kdDebug() << "CvsServiceImpl::checkout()" << endl;

    CheckoutDialog dlg( m_cvsService, mainWindow()->main()->centralWidget() );

    if ( dlg.exec() == QDialog::Accepted )
    {
        DCOPRef job = m_cvsService->checkout( dlg.workDir(), dlg.serverPath(),
            dlg.module(), dlg.tag(), dlg.pruneDirs()
        );
        if (!m_cvsService->ok())
        {
            KMessageBox::sorry( mainWindow()->main(), i18n( "Unable to checkout" ) );
            return;
        }
        // Save the path for later retrieval since slotCheckoutFinished(bool,int)
        // will use it for return the info to the caller.
        modulePath = dlg.workDir() + QDir::separator() + dlg.module();

        processWidget()->startJob( job );
        connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotCheckoutFinished(bool,int)) );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::commit( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::commit() here!" << endl;
    kdDebug(9000) << "Commit requested for " << urlList.count() << " file(s)." << endl;

    if (!prepareOperation( urlList, opCommit ))
        return;

    CommitDialog dlg;
    if (dlg.exec() == QDialog::Rejected)
        return;

//    CvsOptions *options = CvsOptions::instance();
    QString logString = dlg.logMessage().join( "\n" );

    DCOPRef cvsJob = m_cvsService->commit( m_fileList, logString, false );
    if (!m_cvsService->ok())
    {
        kdDebug( 9000 ) << "Commit of " << m_fileList.join( ", " ) << " failed!!!" << endl;
        return;
    }

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    // 2. if requested to do so, add an entry to the Changelog too
    if (dlg.mustAddToChangeLog())
    {
        // 2.1 Modify the Changelog
        ChangeLogEntry entry;
        entry.addLines( dlg.logMessage() );
        entry.addToLog( projectDirectory() + "/ChangeLog" );

        kdDebug( 9999 ) << " *** ChangeLog entry : " <<
            entry.toString( changeLogPrependString ) << endl;
    }

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::update( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::update() here" << endl;

    if (!prepareOperation( urlList, opCommit ))
        return;

    CvsOptions *options = CvsOptions::instance();
    ReleaseInputDialog dlg( mainWindow()->main()->centralWidget() );
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString additionalOptions = dlg.release();
    if (dlg.isRevert())
        additionalOptions = additionalOptions + " " + options->revertOptions();

    DCOPRef cvsJob = m_cvsService->update( m_fileList,
        options->recursiveWhenUpdate(),
        options->createDirsWhenUpdate(),
        options->pruneEmptyDirsWhenUpdate(),
        additionalOptions ); 

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::add( const KURL::List& urlList, bool binary )
{
    kdDebug(9000) << "CvsServiceImpl::add() here" << endl;

    if (!prepareOperation( urlList, opAdd ))
        return;

    DCOPRef cvsJob = m_cvsService->add( m_fileList, binary );

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::remove( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::remove() here" << endl;

    if (!prepareOperation( urlList, opRemove ))
        return;

    DCOPRef cvsJob = m_cvsService->remove( m_fileList, true );

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////
/*
void CvsServiceImpl::revert( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::revert() here" << endl;

    if (!prepareOperation( urlList, opRevert ))
        return;

    CvsOptions *options = CvsOptions::instance();
    QString revertOptions = options->revertOptions();

    ReleaseInputDialog dlg( i18n("Revert to another release ..."),
        mainWindow()->main()->centralWidget() );
    if (dlg.exec() == QDialog::Rejected)
        return;

    revertOptions += dlg.release();
    DCOPRef cvsJob = m_cvsService->update( m_fileList, true, true, true, revertOptions );

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}
*/
///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::removeStickyFlag( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::revert() here" << endl;

    if (!prepareOperation( urlList, opUpdate ))
        return;

    CvsOptions *options = CvsOptions::instance();

    DCOPRef cvsJob = m_cvsService->update( m_fileList,
        options->recursiveWhenUpdate(),
        options->createDirsWhenUpdate(),
        options->pruneEmptyDirsWhenUpdate(),
        "-A" );

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::log( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::log() here: " << endl;

    if (!prepareOperation( urlList, opLog ))
        return;

    CVSLogDialog* f = new CVSLogDialog( m_cvsService );
    f->show();
    // Form will do all the work
    f->startLog( projectDirectory(), m_fileList[0] );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::diff( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::diff() here" << endl;

    if (!prepareOperation( urlList, opDiff ))
        return;

    DiffDialog dlg;
    if (dlg.exec() != QDialog::Accepted)
        return;

    CvsOptions *options = CvsOptions::instance();
    DCOPRef cvsJob = m_cvsService->diff( m_fileList[0], dlg.revA(),
                dlg.revB(), options->diffOptions(), options->contextLines() );
    if (!m_cvsService->ok())
    {
        KMessageBox::sorry( 0, i18n("Sorry, cannot diff!"),
            i18n("Sorry cannot diff 2") );
        return;
    }

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotDiffFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::tag( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::tag() here" << endl;

    if (!prepareOperation( urlList, opTag ))
        return;

    TagDialog dlg( i18n("Creating Tag/Branch for files ..."),
        mainWindow()->main()->centralWidget() );
    if (dlg.exec() != QDialog::Accepted)
        return;

    DCOPRef cvsJob = m_cvsService->createTag( m_fileList, dlg.tagName(),
        dlg.isBranch(), dlg.force() );

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::unTag( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsServiceImpl::unTag() here" << endl;

    if (!prepareOperation( urlList, opUnTag ))
        return;

    TagDialog dlg( i18n("Removing Tag/Branch from files ..."),
        mainWindow()->main()->centralWidget() );
    if (dlg.exec() != QDialog::Accepted)
        return;

    DCOPRef cvsJob = m_cvsService->createTag( m_fileList, dlg.tagName(),
        dlg.isBranch(), dlg.force() );

    processWidget()->startJob( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::addToIgnoreList( const KURL::List& urlList )
{
    CvsServicePartImpl::addToIgnoreList( projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::removeFromIgnoreList( const KURL::List& urlList )
{
    CvsServicePartImpl::removeFromIgnoreList( projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::createNewProject( const QString &dirName,
    const QString &cvsRsh, const QString &location,
    const QString &message, const QString &module, const QString &vendor,
    const QString &release, bool mustInitRoot )
{
    kdDebug( 9000 ) << "====> CvsServiceImpl::createNewProject( const QString& )" << endl;

    CvsOptions *options = CvsOptions::instance();
    options->setCvsRshEnvVar( cvsRsh );
    options->setLocation( location );

    QString rsh_preamble;
    if ( !options->cvsRshEnvVar().isEmpty() )
        rsh_preamble = "CVS_RSH=" + KShellProcess::quote( options->cvsRshEnvVar() );

    QString init;
    if (mustInitRoot)
    {
        init = rsh_preamble + " cvs -d " + KShellProcess::quote( options->location() ) + " init && ";
    }
    QString cmdLine = init + "cd " + KShellProcess::quote(dirName) +
        " && " + rsh_preamble +
        " cvs -d " + KShellProcess::quote(options->location()) +
        " import -m " + KShellProcess::quote(message) + " " +
        KShellProcess::quote(module) + " " +
        KShellProcess::quote(vendor) + " " +
        KShellProcess::quote(release) +
        // CVS build-up magic here ...
        " && sh " +
        locate("data","kdevcvsservice/buildcvs.sh") + " . " +
        KShellProcess::quote(module) + " " +
        KShellProcess::quote(location);

    kdDebug( 9000 ) << "  ** Will run the following command: " << endl << cmdLine << endl;
    kdDebug( 9000 ) << "  ** on directory: " << dirName << endl;

    m_part->makeFrontend()->queueCommand( dirName, cmdLine );
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServiceImpl::requestCvsService()
{
    QCString appId;
    QString error;

    if (KApplication::startServiceByDesktopName( "cvsservice",
        QStringList(), &error, &appId ))
    {
        kdDebug() << "Starting cvsservice failes with message: " <<
            error << endl;
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

void CvsServiceImpl::releaseCvsService()
{
    if (m_cvsService)
        m_cvsService->quit();
    delete m_cvsService;
    delete m_repository;
}

///////////////////////////////////////////////////////////////////////////////
// SLOTS here!
///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::slotDiffFinished( bool normalExit, int exitStatus )
{
    core()->running( m_part, false );

    QString diff = processWidget()->output().join("\n"),
        err = processWidget()->errors().join("\n");

    kdDebug( 9999 ) << "diff = " << diff << endl;
    kdDebug( 9999 ) << "err = " << err << endl;

    if (normalExit)
        kdDebug( 9999 ) << " *** Process died nicely with exit status = " <<
            exitStatus << endl;
    else
        kdDebug( 9999 ) << " *** Process was killed with exit status = " <<
            exitStatus << endl;

    // Now show a message about operation ending status
    if (diff.isEmpty() && (exitStatus != 0))
    {
        KMessageBox::information( 0, i18n("Operation aborted (process killed)"),
            i18n("CVS Diff") );
        return;
    }
    if ( diff.isEmpty() && !err.isEmpty() )
    {
        KMessageBox::detailedError( 0, i18n("CVS outputted errors during diff."),
            err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() )
    {
        int s = KMessageBox::warningContinueCancelList( 0,
            i18n("CVS outputted errors during diff. Do you still want to continue?"),
            QStringList::split( "\n", err, false ), i18n("Errors During Diff")
        );
        if ( s != KMessageBox::Continue )
            return;
    }

    if ( diff.isEmpty() )
    {
        KMessageBox::information( 0, i18n("There is no difference to the repository."),
            i18n("No Difference Found") );
        return;
    }

    Q_ASSERT( diffFrontend() );
    diffFrontend()->showDiff( diff );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::slotCheckoutFinished( bool exitStatus, int )
{
    kdDebug() << "CvsServiceImpl::slotCheckoutFinished(): job ended with status == "
        << exitStatus << endl;
    // Return a null string if the operation was not succesfull
    if (exitStatus)
        modulePath = QString::null;

    kdDebug() << "   I'll emit modulePath == " << modulePath << endl;

    emit checkoutFinished( modulePath );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::slotJobFinished( bool /*exitStatus*/, int exitCode )
{
    // Return a null string if the operation was not succesfull
    kdDebug() << "CvsServiceImpl::slotJobFinished(): job ended with code == "
        << exitCode << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServiceImpl::slotProjectOpened()
{
    kdDebug() << "CvsServiceImpl::slotProjectOpened(): setting work directory to "
        << projectDirectory() << endl;

    m_repository->setWorkingCopy( projectDirectory() );
}

#include "cvsserviceimpl.moc"
