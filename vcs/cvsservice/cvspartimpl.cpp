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

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmainwindow.h>
#include <dcopref.h>
#include <repository_stub.h>
#include <cvsservice_stub.h>
#include <cvsjob_stub.h>
// CvsService stuff
#include <repository_stub.h>
#include <cvsservice_stub.h>
#include <cvsjob_stub.h>
// KDevelop SDK stuff
#include <urlutil.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevcore.h>
#include <kdevdifffrontend.h>
#include <kdevmakefrontend.h>
#include <kdevpartcontroller.h>
// Part's widgets
#include "cvsprocesswidget.h"
#include "checkoutdialog.h"
#include "commitdlg.h"
#include "tagdialog.h"
#include "diffdialog.h"
#include "releaseinputdialog.h"
#include "cvslogdialog.h"
#include "editorsdialog.h"

#include "changelog.h"
#include "cvsoptions.h"
#include "cvsdir.h"
#include "cvsentry.h"
#include "jobscheduler.h"
#include "cvsfileinfoprovider.h"

#include "cvspart.h"
#include "cvspartimpl.h"

///////////////////////////////////////////////////////////////////////////////
// class Constants
///////////////////////////////////////////////////////////////////////////////

// Nice name (relative to projectDirectory()) ;-)
const QString CvsServicePartImpl::changeLogFileName( "ChangeLog" );
// Four spaces for every log line (except the first, which includes the
// developers name)
const QString CvsServicePartImpl::changeLogPrependString( "    " );

///////////////////////////////////////////////////////////////////////////////
// class CvsServicePartImpl
///////////////////////////////////////////////////////////////////////////////

CvsServicePartImpl::CvsServicePartImpl( CvsServicePart *part, const char *name )
    : QObject( this, name? name : "cvspartimpl" ),
    m_scheduler( 0 ), m_part( part ), m_widget( 0 )
{
    if (requestCvsService())
    {
        m_widget = new CvsProcessWidget( m_cvsService, part, 0, "cvsprocesswidget" );
        m_scheduler = new DirectScheduler( m_widget );
        m_fileInfoProvider = new CVSFileInfoProvider( part, m_cvsService );
    
        connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    }
    else
    {
        kdDebug() << "CvsServicePartImpl::CvsServicePartImpl(): somebody kills me because"
            "I could not request a valid CvsService!!!! :-((( " << endl;
    }

}

///////////////////////////////////////////////////////////////////////////////

CvsServicePartImpl::~CvsServicePartImpl()
{
    if (processWidget())
    {
        // Inform toplevel, that the output view is gone
        mainWindow()->removeView( m_widget );
        delete m_widget;
    }
    delete m_scheduler;
    //delete m_fileInfoProvider;
    releaseCvsService();
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::prepareOperation( const KURL::List &someUrls, CvsOperation op )
{
    kdDebug() << "===> CvsServicePartImpl::prepareOperation(const KURL::List &, CvsOperation)" << endl;

    bool correctlySetup = (m_cvsService != 0) && (m_repository != 0);
    if (!correctlySetup)
    {
        kdDebug(9006) << "DCOP CvsService is not available!!!" << endl;
        return false;
    }

    KURL::List urls = someUrls;
    URLUtil::dump( urls, "Requested CVS operation for: " );

    if (!m_part->project())
    {
        kdDebug(9006) << "CvsServicePartImpl::prepareOperation(): No project???" << endl;
        KMessageBox::sorry( 0, i18n("Open a project first.\nOperation will be aborted.") );
        return false;
    }

    if (m_widget->isAlreadyWorking())
    {
        if (KMessageBox::warningYesNo( 0,
            i18n("Another CVS operation is executing: do you want to cancel it \n"
                "and start this new one?"),
            i18n("CVS: Operation Already Pending ")) == KMessageBox::Yes)
        {
            m_widget->cancelJob();
        }
        else // Operation canceled
        {
            kdDebug() << "===> Operation canceled by user request" << endl;
            return false;
        }
    }

    validateURLs( projectDirectory(),  urls, op );
    if (urls.count() <= 0) // who knows? ;)
    {
        kdDebug(9006) << "CvsServicePartImpl::prepareOperation(): No valid document URL selected!!!" << endl;
        KMessageBox::sorry( 0, i18n("None of the file(s) you selected seem to be valid for repository.") );
        return false;
    }

    URLUtil::dump( urls );
    // Save for later use
    m_urlList = urls;
    m_lastOperation = op;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::doneOperation( const KURL::List &/*someUrls*/, CvsOperation /*op*/ )
{
    kdDebug(9006) << "CvsServicePartImpl::doneOperation(const KURL::List&, CvsOperation)" << endl;

    // @ todo notify clients (filetree) about changed status?)
}

///////////////////////////////////////////////////////////////////////////////

const KURL::List &CvsServicePartImpl::urlList() const
{
    return m_urlList;
}

///////////////////////////////////////////////////////////////////////////////

QStringList CvsServicePartImpl::fileList( bool relativeToProjectDir ) const
{
    if (relativeToProjectDir)
        return URLUtil::toRelativePaths( projectDirectory(), urlList() );
    else
        return urlList().toStringList();
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::isRegisteredInRepository( const QString &projectDirectory, const KURL &url )
{
    kdDebug(9006) << "===> CvsServicePartImpl::isRegisteredInRepository() here! " << endl;

    // KURL::directory() is a bit tricky when used on file or _dir_ paths ;-)
    KURL projectURL = KURL::fromPathOrURL( projectDirectory );
    kdDebug(9006) << "projectURL = " << projectURL.url() << endl;
    kdDebug(9006) << "url        = " << url.url() << endl;

    if ( projectURL == url)
    {
        CVSDir cvsdir = CVSDir( projectDirectory );
        return cvsdir.isValid();
    }
    else
    {
        CVSDir cvsdir = CVSDir( url.directory() );

        if (!cvsdir.isValid())
        {
            kdDebug(9006) << "===> Error: " << cvsdir.path() << " is not a valid CVS directory " << endl;
            return false;
        }
        CVSEntry entry = cvsdir.fileStatus( url.fileName() );
        return entry.isValid();
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::validateURLs( const QString &projectDirectory, KURL::List &urls, CvsOperation op )
{
    kdDebug(9006) << "CvsServicePartImpl::validateURLs() here!" << endl;

    // If files are to be added, we can avoid to check them to see if they are registered in the
    // repository ;)
    if (op == opAdd)
    {
        kdDebug(9006) << "This is a Cvs Add operation and will not be checked against repository ;-)" << endl;
        return;
    }
    QValueList<KURL>::iterator it = urls.begin();
    while (it != urls.end())
    {
        if (!CvsServicePartImpl::isRegisteredInRepository( projectDirectory, (*it) ))
        {
            kdDebug(9006) << "Warning: file " << (*it).path() << " does NOT belong to repository and will not be used" << endl;

            it = urls.erase( it );
        }
        else
        {
            kdDebug(9006) << "Warning: file " << (*it).path() << " is in repository and will be accepted" << endl;

            ++it;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::addToIgnoreList( const QString &projectDirectory, const KURL &url )
{
    kdDebug(9006) << "===> CvsServicePartImpl::addToIgnoreList() here! " << endl;

    if ( url.path() == projectDirectory )
    {
        kdDebug(9006) << "Can't add to ignore list current project directory " << endl;
        return;
    }

    CVSDir cvsdir( url.directory() );
    cvsdir.ignoreFile( url.fileName() );
}

void CvsServicePartImpl::addToIgnoreList( const QString &projectDirectory, const KURL::List &urls )
{
    for (size_t i=0; i<urls.count(); ++i)
    {
        addToIgnoreList( projectDirectory, urls[i] );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::removeFromIgnoreList( const QString &/*projectDirectory*/, const KURL &url )
{
    kdDebug(9006) << "===> CvsServicePartImpl::removeFromIgnoreList() here! " << endl;

    QStringList ignoreLines;

    CVSDir cvsdir( url.directory() );
    cvsdir.doNotIgnoreFile( url.fileName() );
}

void CvsServicePartImpl::removeFromIgnoreList( const QString &projectDirectory, const KURL::List &urls )
{
    for (size_t i=0; i<urls.count(); ++i)
    {
        removeFromIgnoreList( projectDirectory, urls[i] );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::isValidDirectory( const QDir &dir ) const
{
    CVSDir cvsdir( dir );

    return cvsdir.isValid();
}

///////////////////////////////////////////////////////////////////////////////

CvsProcessWidget *CvsServicePartImpl::processWidget() const
{
    return m_widget;
}

///////////////////////////////////////////////////////////////////////////////

KDevMainWindow *CvsServicePartImpl::mainWindow() const
{
    return m_part->mainWindow();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsServicePartImpl::projectDirectory() const
{
    return m_part->project() ? m_part->project()->projectDirectory() : QString::null;
}

///////////////////////////////////////////////////////////////////////////////

KDevCore *CvsServicePartImpl::core() const
{
    return m_part->core();
}

///////////////////////////////////////////////////////////////////////////////

KDevDiffFrontend *CvsServicePartImpl::diffFrontend() const
{
    return m_part->extension<KDevDiffFrontend>("KDevelop/DiffFrontend");
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::login()
{
    DCOPRef job = m_cvsService->login( this->projectDirectory() );

    m_scheduler->schedule( job );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::logout()
{
    DCOPRef job = m_cvsService->logout( this->projectDirectory() );

    m_scheduler->schedule( job );
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::checkout()
{
    kdDebug() << "CvsServicePartImpl::checkout()" << endl;

    CheckoutDialog dlg( m_cvsService, mainWindow()->main()->centralWidget() );

    if ( dlg.exec() == QDialog::Accepted )
    {
#if KDE_IS_VERSION(3,2,90)
        DCOPRef job = m_cvsService->checkout( dlg.workDir(), dlg.serverPath(),
            dlg.module(), dlg.tag(), dlg.pruneDirs(), "", false
        );
#else
        DCOPRef job = m_cvsService->checkout( dlg.workDir(), dlg.serverPath(),
            dlg.module(), dlg.tag(), dlg.pruneDirs()
        );
#endif
        if (!m_cvsService->ok()) {
            KMessageBox::sorry( mainWindow()->main(), i18n( "Unable to checkout" ) );
        } else {
        	// Save the path for later retrieval since slotCheckoutFinished(bool,int)
        	// will use it for return the info to the caller.
			modulePath = dlg.workDir() + dlg.module();

        	m_scheduler->schedule( job );
        	connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotCheckoutFinished(bool,int)) );
			return true;
		}
    }
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::commit( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::commit() here!" << endl;
    kdDebug(9006) << "Commit requested for " << urlList.count() << " file(s)." << endl;

    if (!prepareOperation( urlList, opCommit ))
        return;

    CommitDialog dlg( projectDirectory() + "/ChangeLog" );
    if (dlg.exec() == QDialog::Rejected)
        return;

    CvsOptions *options = CvsOptions::instance();
    QString logString = dlg.logMessage().join( "\n" );

    DCOPRef cvsJob = m_cvsService->commit( fileList(), logString, options->recursiveWhenCommitRemove() );
    if (!m_cvsService->ok())
    {
        kdDebug( 9006 ) << "Commit of " << fileList().join( ", " ) << " failed!!!" << endl;
        return;
    }

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    // 2. if requested to do so, add an entry to the Changelog too
    if (dlg.mustAddToChangeLog())
    {
        // 2.1 Modify the Changelog
        ChangeLogEntry entry;
        entry.addLines( dlg.logMessage() );
        entry.addToLog( dlg.changeLogFileName() );

        kdDebug( 9999 ) << " *** ChangeLog entry : " <<
            entry.toString( changeLogPrependString ) << endl;
    }

    doneOperation( KURL::List( fileList() ), opCommit );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::update( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::update() here" << endl;

    if (!prepareOperation( urlList, opCommit ))
        return;

    CvsOptions *options = CvsOptions::instance();
    ReleaseInputDialog dlg( mainWindow()->main()->centralWidget() );
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString additionalOptions = dlg.release();
    if (dlg.isRevert())
        additionalOptions = additionalOptions + " " + options->revertOptions();

    DCOPRef cvsJob = m_cvsService->update( fileList(),
        options->recursiveWhenUpdate(),
        options->createDirsWhenUpdate(),
        options->pruneEmptyDirsWhenUpdate(),
        additionalOptions );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::add( const KURL::List& urlList, bool binary )
{
    kdDebug(9006) << "CvsServicePartImpl::add() here" << endl;

    if (!prepareOperation( urlList, opAdd ))
        return;

    DCOPRef cvsJob = m_cvsService->add( fileList(), binary );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::unedit( const KURL::List& urlList)
{
    kdDebug(9006) << "CvsServicePartImpl::unedit()" << endl;

    if (!prepareOperation( urlList, opUnEdit ))
        return;

    DCOPRef cvsJob = m_cvsService->unedit( fileList() );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::edit( const KURL::List& urlList)
{
    kdDebug(9006) << "CvsServicePartImpl::edit()" << endl;

    if (!prepareOperation( urlList, opEdit ))
        return;

    DCOPRef cvsJob = m_cvsService->edit( fileList() );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::editors( const KURL::List& urlList)
{
    kdDebug(9006) << "CvsServicePartImpl::editors()" << endl;

    if (!prepareOperation( urlList, opEditors ))
        return;

    EditorsDialog * f = new EditorsDialog( m_cvsService );
    f->show();
    //the dialog will do all the work
    f->startjob( fileList()[0] );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::remove( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::remove() here" << endl;

    if (!prepareOperation( urlList, opRemove ))
        return;

    DCOPRef cvsJob = m_cvsService->remove( fileList(), true );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::removeStickyFlag( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::revert() here" << endl;

    if (!prepareOperation( urlList, opUpdate ))
        return;

    CvsOptions *options = CvsOptions::instance();

    DCOPRef cvsJob = m_cvsService->update( fileList(),
        options->recursiveWhenUpdate(),
        options->createDirsWhenUpdate(),
        options->pruneEmptyDirsWhenUpdate(),
        "-A" );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::log( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::log() here: " << endl;

    if (!prepareOperation( urlList, opLog ))
        return;

    CVSLogDialog* f = new CVSLogDialog( m_cvsService );
    f->show();
    // Form will do all the work
    f->startLog( projectDirectory(), fileList()[0] );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::diff( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::diff() here" << endl;

    if (!prepareOperation( urlList, opDiff ))
        return;

    DiffDialog dlg;
    if (dlg.exec() != QDialog::Accepted)
        return;

    CvsOptions *options = CvsOptions::instance();
    DCOPRef cvsJob = m_cvsService->diff( fileList()[0], dlg.revA(),
                dlg.revB(), options->diffOptions(), options->contextLines() );
    if (!m_cvsService->ok())
    {
        KMessageBox::sorry( 0, i18n("Sorry, cannot diff."),
            i18n("Error During Diff") );
        return;
    }

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotDiffFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::tag( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::tag() here" << endl;

    if (!prepareOperation( urlList, opTag ))
        return;

    TagDialog dlg( i18n("Creating Tag/Branch for files ..."),
        mainWindow()->main()->centralWidget() );
    if (dlg.exec() != QDialog::Accepted)
        return;

    DCOPRef cvsJob = m_cvsService->createTag( fileList(), dlg.tagName(),
        dlg.isBranch(), dlg.force() );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::unTag( const KURL::List& urlList )
{
    kdDebug(9006) << "CvsServicePartImpl::unTag() here" << endl;

    if (!prepareOperation( urlList, opUnTag ))
        return;

    TagDialog dlg( i18n("Removing Tag/Branch from files ..."),
        mainWindow()->main()->centralWidget() );
    if (dlg.exec() != QDialog::Accepted)
        return;

    DCOPRef cvsJob = m_cvsService->createTag( fileList(), dlg.tagName(),
        dlg.isBranch(), dlg.force() );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)),
        this, SLOT(slotJobFinished(bool,int)) );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::addToIgnoreList( const KURL::List& urlList )
{
    addToIgnoreList( projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::removeFromIgnoreList( const KURL::List& urlList )
{
    removeFromIgnoreList( projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

/**
* \FIXME Current implementation doesn't use CvsService :-( I just ported the
* old code which relies on buildcvs.sh script. [marios]
*/
void CvsServicePartImpl::createNewProject( const QString &dirName,
    const QString &cvsRsh, const QString &location,
    const QString &message, const QString &module, const QString &vendor,
    const QString &release, bool mustInitRoot )
{
    kdDebug( 9006 ) << "====> CvsServicePartImpl::createNewProject( const QString& )" << endl;

    CvsOptions *options = CvsOptions::instance();
    options->setCvsRshEnvVar( cvsRsh );
    options->setLocation( location );
/*
	//virtual DCOPRef import( const QString& workingDir, const QString& repository, const QString& module, const QString& ignoreList, const QString& comment, const
    QString filesToIgnore;
	DCOPRef cvsJob = m_cvsService->import( dirName, location, module, filesToIgnore, message, vendor, release, false );

    m_scheduler->schedule( cvsJob );
    connect( processWidget(), SIGNAL(jobFinished(bool,int)), this, SLOT(slotCheckoutFinished(bool,int)) );
*/
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

    kdDebug( 9006 ) << "  ** Will run the following command: " << endl << cmdLine << endl;
    kdDebug( 9006 ) << "  ** on directory: " << dirName << endl;

    if (KDevMakeFrontend *makeFrontend = m_part->extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand( dirName, cmdLine );
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::requestCvsService()
{
    QCString appId;
    QString error;

    if (KApplication::startServiceByDesktopName( "cvsservice",
        QStringList(), &error, &appId ))
    {
        QString msg = i18n( "Unable to find the Cervisia KPart. \n"
            "Cervisia Integration will not be available. Please check your\n"
            "Cervisia installation and re-try. Reason was:\n" ) + error;
        KMessageBox::error( processWidget(), msg, "DCOP Error" );

        return false;
    }
    else
    {
        m_cvsService = new CvsService_stub( appId, "CvsService" );
        m_repository = new Repository_stub( appId, "CvsRepository" );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::releaseCvsService()
{
    if (m_cvsService)
        m_cvsService->quit();
    delete m_cvsService;
    m_cvsService = 0;
    delete m_repository;
    m_repository = 0;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::flushJobs()
{
    processWidget()->cancelJob();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::addFilesToProject( const QStringList &filesToAdd )
{
    kdDebug( 9006 ) << "====> CvsServicePart::slotAddFilesToProject(const QStringList &)" << endl;

    QStringList filesInCVS = checkFileListAgainstCVS( filesToAdd );
    if (filesInCVS.isEmpty())
        return;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want to be added to CVS repository too?"),
        i18n("CVS - New Files Added to Project"),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenAddingNewFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9006 ) << "Adding these files: " << filesInCVS.join( ", " ) << endl;

        const KURL::List urls = KURL::List( filesInCVS );
        URLUtil::dump( urls );
        add( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::removedFilesFromProject(const QStringList &filesToRemove)
{
    kdDebug( 9006 ) << "====> CvsServicePart::slotRemovedFilesFromProject( const QStringList &)" << endl;

    QStringList filesInCVS = checkFileListAgainstCVS( filesToRemove );
    if (filesInCVS.isEmpty())
        return;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want them to be removed from CVS repository too?\nWarning: They will be removed from disk too."),
        i18n("CVS - Files Removed From Project"),
        KStdGuiItem::yes(),
        KStdGuiItem::no(),
        i18n("askWhenRemovingFiles") );
    if (s == KMessageBox::Yes)
    {
        kdDebug( 9006 ) << "Removing these files: " << filesInCVS.join( ", " ) << endl;
        const KURL::List urls = KURL::List( filesInCVS );
        URLUtil::dump( urls );
        remove( urls );
    }
}

///////////////////////////////////////////////////////////////////////////////

QStringList CvsServicePartImpl::checkFileListAgainstCVS( const QStringList &filesToCheck ) const
{
    QStringList filesInCVS;
    for (QStringList::const_iterator it = filesToCheck.begin(); it != filesToCheck.end(); ++it )
    {
        const QString &fn = (*it);
        QFileInfo fi( fn );
        if (isValidDirectory( fi.dirPath( true ) ))
            filesInCVS += ( m_part->project()->projectDirectory() + QDir::separator() + fn );
    }

    return filesInCVS;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::emitFileStateModified( const KURL::List &/*urls*/, VCSFileInfo::FileState &/*commonState*/ )
{
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider *CvsServicePartImpl::fileInfoProvider() const
{
    return m_fileInfoProvider;
}

///////////////////////////////////////////////////////////////////////////////
// SLOTS here!
///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::slotDiffFinished( bool normalExit, int exitStatus )
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
        KMessageBox::information( 0, i18n("Operation aborted (process killed)."),
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
            i18n("CVS output errors during diff. Do you still want to continue?"),
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

void CvsServicePartImpl::slotCheckoutFinished( bool exitStatus, int )
{
    kdDebug() << "CvsServicePartImpl::slotCheckoutFinished(): job ended with status == "
        << exitStatus << endl;
    // Return a null string if the operation was not succesfull
    if (!exitStatus)
        modulePath = QString::null;

    kdDebug() << "   I'll emit modulePath == " << modulePath << endl;

    emit checkoutFinished( modulePath );
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::slotJobFinished( bool /*exitStatus*/, int exitCode )
{
    // Return a null string if the operation was not succesfull
    kdDebug() << "CvsServicePartImpl::slotJobFinished(): job ended with code == "
        << exitCode << endl;
/*
    // Operation has been successfull
    if (!exitStatus)
        return;

    // 1. Assemble the CVSFileInfoList
    // 2. notify all clients
*/
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::slotProjectOpened()
{
    kdDebug() << "CvsServicePartImpl::slotProjectOpened(): setting work directory to "
        << projectDirectory() << endl;

    if ( m_repository )
    {
        m_repository->setWorkingCopy( projectDirectory() );
    }
}


#include "cvspartimpl.moc"
