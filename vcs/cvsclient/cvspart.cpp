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
#include <kmainwindow.h>
#include <kaction.h>
#include <kurl.h>
#include <kdeversion.h>
#include <kiconloader.h>

#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <kdevgenericfactory.h>

#include <qdir.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

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
#include "cvswidget.h"
#include "cvsoptions.h"
#include "commitdlg.h"
#include "tagdialog.h"
#include "logform.h"
#include "cvsform.h"
#include "cvsoptionswidget.h"
#include "checkoutdialog.h"

#include <kdeversion.h>
#if (KDE_VERSION_MINOR==0) && (KDE_VERSION_MAJOR==3)
#include <kdevkurl.h>
#endif


KURL KURL_fromPathOrURL( const QString& text )
{
#if (KDE_VERSION_MINOR!=0) || (KDE_VERSION_MAJOR!=3)
    return KURL::fromPathOrURL(text);
#else
    return KdevKURL::fromPathOrURL(text);
#endif
}

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

static const KAboutData data("kdevcvs", I18N_NOOP("CVS"), "1.0");

typedef KDevGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvs, CvsFactory( &data ) )

///////////////////////////////////////////////////////////////////////////////
// class CvsPart
///////////////////////////////////////////////////////////////////////////////

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & )
    : KDevVersionControl( "KDevCvsPart", "kdevcvspart", parent, name ? name : "CVS" ),
    proc( 0 ),
    actionCommit( 0 ), actionDiff( 0 ), actionLog( 0 ), actionAdd( 0 ), actionRemove( 0 ),
    actionUpdate( 0 ), actionRevert( 0 ), actionTag( 0 ),
    actionAddToIgnoreList( 0 ), actionRemoveFromIgnoreList( 0 )
{
    setInstance( CvsFactory::instance() );
    
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

    m_widget = new CvsWidget( this );
    m_widget->setIcon( SmallIcon("db") );
    m_widget->setCaption(i18n("CVS Output"));
    mainWindow()->embedOutputView( m_widget, i18n("CVS"), i18n("CVS output") ); // i18n("CVS") ?!?? ;)
    QWhatsThis::add(m_widget, i18n("<b>CVS</b><p>Concurrent Versions System operations window. Shows results of cvs command execution."));
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::setupActions()
{

/*    KAction * action = new KAction( i18n("CVS Repository"), 0, this, SLOT(slotCheckOut()),
        actionCollection(), "cvs_checkout" );
    action->setStatusText( i18n("Check-out from an existing CVS repository") );*/
    actionCommit = new KAction( i18n("Commit"), 0, this, SLOT(slotActionCommit()),
        actionCollection(), "cvs_commit" );
    actionCommit->setToolTip(i18n("Commit"));
    actionCommit->setWhatsThis(i18n("<b>Commit</b><p>Performs <b>cvs commit</b> command. Use this command when you wish to ''publish'' your changes to other developers, by incorporating them in the source repository."));
    actionDiff = new KAction( i18n("Diff"), 0, this, SLOT(slotActionDiff()),
        actionCollection(), "cvs_diff" );
    actionDiff->setToolTip(i18n("Diff"));
    actionDiff->setWhatsThis(i18n("<b>Diff</b><p>Performs <b>cvs diff</b> command. Shows differences between files in working directory and source repository, or between two revisions in source repository."));
    actionLog = new KAction( i18n("Log"), 0, this, SLOT(slotActionLog()),
        actionCollection(), "cvs_log" );
    actionLog->setToolTip(i18n("Log"));
    actionLog->setWhatsThis(i18n("<b>Log</b><p>Performs <b>cvs log</b> command. Displays log information."));
    actionAdd = new KAction( i18n("Add"), 0, this, SLOT(slotActionAdd()),
        actionCollection(), "cvs_add" );
    actionAdd->setToolTip(i18n("Add"));
    actionAdd->setWhatsThis(i18n("<b>Add</b><p>Performs <b>cvs add</b> command. Adds a new file or directory to the repository."));
    actionRemove = new KAction( i18n("Remove From Repository"), 0, this, SLOT(slotActionRemove()),
        actionCollection(), "cvs_remove" );
    actionRemove->setToolTip(i18n("Remove from repository"));
    actionRemove->setWhatsThis(i18n("<b>Remove from repository</b><p>Performs <b>cvs remove</b> command. Removes files from the source repository."));
    actionUpdate = new KAction( i18n("Update"), 0, this, SLOT(slotActionUpdate()),
        actionCollection(), "cvs_update" );
    actionUpdate->setToolTip(i18n("Update"));
    actionUpdate->setWhatsThis(i18n("<b>Update</b><p>Performs <b>cvs update</b> command. Brings your working directory up to date with changes from the repository."));
    actionRevert = new KAction( i18n("Replace with Copy From Repository"), 0, this, SLOT(slotActionRevert()),
        actionCollection(), "cvs_revert" );
    actionRevert->setToolTip(i18n("Replace with copy from repository"));
    actionRevert->setWhatsThis(i18n("<b>Replace with copy from repository</b><p>Performs <b>cvs revert</b> command. Replaces your version of file with the version contained in the repository."));
    actionTag = new KAction( i18n("Tag/Branch Selected File(s)"), 0, this, SLOT(slotTag()),
        actionCollection(), "cvs_tag" );
    actionTag->setToolTip(i18n("Tag/branch selected file(s)"));
    actionTag->setWhatsThis(i18n("<b>Tag/branch selected file(s)</b><p>Performs <b>cvs tag</b> command. Specifies a symbolic tag for files in the repository."));

    actionAddToIgnoreList = new KAction( i18n("Ignore This File When Doing CVS Operation"), 0,
        this, SLOT(slotActionAddToIgnoreList()), actionCollection(), "cvs_ignore" );
    actionAddToIgnoreList->setToolTip(i18n("Ignore this file when doing CVS operation"));
    actionAddToIgnoreList->setWhatsThis(i18n("<b>Ignore this file when doing CVS operation</b><p>Puts the name of this file onto a .cvsignore list."));
    actionRemoveFromIgnoreList = new KAction( i18n("Do Not Ignore This File When Doing CVS Operation"), 0,
        this, SLOT(slotActionRemoveFromIgnoreList()), actionCollection(), "cvs_donot_ignore" );
    actionRemoveFromIgnoreList->setToolTip(i18n("Do not ignore this file when doing CVS operation"));
    actionRemoveFromIgnoreList->setWhatsThis(i18n("<b>Do not ignore this file when doing CVS operation</b><p>Removes the name of this file from a .cvsignore list."));
}

///////////////////////////////////////////////////////////////////////////////

QString CvsPart::cvs_rsh() const
{
    CvsOptions *options = CvsOptions::instance();
    QString env = options->rsh();
    if ( !env.isEmpty() )
        return QString( "CVS_RSH=" ) + KShellProcess::quote( env );
    return QString::null;
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
        KMessageBox::sorry( 0, i18n("None of the file(s) you selected seem to be valid for repository.") );
        return false;
    }
    URLUtil::dump( urls );
    /// \FIXME Commented because it may break slotLog() and perhaps some another worker
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
    /// @todo
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::createNewProject( const QString& dirName )
{
    kdDebug( 9000 ) << "====> CvsPart::createNewProject( const QString& )" << endl;

    if (!m_cvsConfigurationForm)
        return;
    QString init("");

    /** \FIXME Store rsh setting. Here doesn't store it in CvsOptions because:
      * createNewProject() is called _before_ projectOpened() signal is emitted.
      */
    g_tempEnvRsh = m_cvsConfigurationForm->cvs_rsh->text();
    QString rsh;
    if ( !g_tempEnvRsh.isEmpty() )
        rsh = "CVS_RSH=" + KShellProcess::quote( g_tempEnvRsh );

    if (m_cvsConfigurationForm->init_check->isChecked())
    {
        init = rsh + " cvs -d " + KShellProcess::quote(m_cvsConfigurationForm->root_edit->text()) + " init && ";
    }
    QString command = init + "cd " + KShellProcess::quote(dirName) +
        " && " + rsh +
        " cvs -d " + KShellProcess::quote(m_cvsConfigurationForm->root_edit->text()) +
        " import -m " + KShellProcess::quote(m_cvsConfigurationForm->message_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->repository_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->vendor_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->release_edit->text()) + " && sh " +
        locate("data","kdevcvs/buildcvs.sh") + " . " +
        KShellProcess::quote(m_cvsConfigurationForm->repository_edit->text()) + " " +
        KShellProcess::quote(m_cvsConfigurationForm->root_edit->text());

    kdDebug( 9000 ) << "  ** Will run the following command: " << endl << command << endl;
    kdDebug( 9000 ) << "  ** on directory: " << dirName << endl;

    makeFrontend()->queueCommand( dirName, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::contextMenu( QPopupMenu *popup, const Context *context )
{

// If the current project doesn't support CVS, we don't
// want to confuse the user with a CVS popup menu.
if(!project() || !isValidDirectory(project()->projectDirectory()))
  return;

    if (context->hasType( Context::FileContext ))
    {
        kdDebug(9000) << "CvsPart::contextMenu()" << endl;

        const FileContext *fcontext = static_cast<const FileContext*>( context );
        /// \FIXME Here we must hope that fcontext->fileName() returns an absolute path ;(

        // This stuff should end up into prepareOperation()
        urls = fcontext->urls();
        if (urls.count()==0)
        {
            kdDebug(9000) << "fcontext->urls() returned an empty list. I'll try to get the single file." << endl;
            QString singleFileName = fcontext->fileName();
            if (singleFileName.isEmpty())
            {
                kdDebug(9000) << "fcontext->fileName() returned an empty string. I'm sorry but I give up!" << endl;
                return;
            }
            urls << KURL_fromPathOrURL( singleFileName );
        }
        URLUtil::dump( urls );

        KPopupMenu *subMenu = new KPopupMenu( popup );
//        subMenu->insertTitle( i18n("Available Actions") );

        int id = subMenu->insertItem( actionCommit->text(), this, SLOT(slotCommit()) );
        subMenu->setWhatsThis(id, i18n("<b>Commit</b><p>Performs <b>cvs commit</b> command. Use this command when you wish to ''publish'' your changes to other developers, by incorporating them in the source repository."));
        id = subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
        subMenu->setWhatsThis(id, i18n("<b>Log</b><p>Performs <b>cvs log</b> command. Displays log information."));
        id = subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
        subMenu->setWhatsThis(id, i18n("<b>Update</b><p>Performs <b>cvs update</b> command. Brings your working directory up to date with changes from the repository."));
        id = subMenu->insertItem( actionAdd->text(), this, SLOT(slotAdd()) );
        subMenu->setWhatsThis(id, i18n("<b>Add</b><p>Performs <b>cvs add</b> command. Adds a new file or directory to the repository."));
        id = subMenu->insertItem( actionDiff->text(), this, SLOT(slotDiff()) );
        subMenu->setWhatsThis(id, i18n("<b>Diff</b><p>Performs <b>cvs diff</b> command. Shows differences between files in working directory and source repository, or between two revisions in source repository."));

        subMenu->insertSeparator();

        id = subMenu->insertItem( actionRemove->text(), this, SLOT(slotRemove()) );
        subMenu->setWhatsThis(id, i18n("<b>Remove from repository</b><p>Performs <b>cvs remove</b> command. Removes files from the source repository."));
        id = subMenu->insertItem( actionRevert->text(), this, SLOT(slotRevert()) );
        subMenu->setWhatsThis(id, i18n("<b>Replace with copy from repository</b><p>Performs <b>cvs revert</b> command. Replaces your version of file with the version contained in the repository."));
        id = subMenu->insertItem( actionTag->text(), this, SLOT(slotTag()) );
        subMenu->setWhatsThis(id, i18n("<b>Tag/branch selected file(s)</b><p>Performs <b>cvs tag</b> command. Specifies a symbolic tag for files in the repository."));

        subMenu->insertSeparator();

        id = subMenu->insertItem( actionAddToIgnoreList->text(), this, SLOT(slotAddToIgnoreList()) );
        subMenu->setWhatsThis(id, i18n("<b>Ignore this file when doing CVS operation</b><p>Puts the name of this file onto a .cvsignore list."));
        id = subMenu->insertItem( actionRemoveFromIgnoreList->text(), this, SLOT(slotRemoveFromIgnoreList()) );
        subMenu->setWhatsThis(id, i18n("<b>Do not ignore this file when soing CVS operation</b><p>Removes the name of this file from a .cvsignore list."));

        // Now insert in parent menu
        popup->insertSeparator();
        id = popup->insertItem( i18n("CVS"), subMenu );
        popup->setWhatsThis(id, i18n("<b>CVS</b><p>Concurrent Versions System operations."));
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

void CvsPart::slotActionTag()
{
    KURL currDocument;
    if (urlFocusedDocument( currDocument ))
    {
        urls << currDocument;
        tag( urls );
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

void CvsPart::slotTag()
{
    tag( urls );
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

void CvsPart::slotDiffFinished()
{
    Q_ASSERT( proc );

    core()->running( this, false );

    QString &diff = stdOut,
        &err = stdErr;
    bool normalExit = proc->normalExit();
    int exitStatus = proc->exitStatus();

    kdDebug( 9999 ) << "diff = " << diff << endl;
    kdDebug( 9999 ) << "err = " << err << endl;

    if (normalExit)
        kdDebug( 9999 ) << " *** Process " << proc->name() << " died nicely with exit status = " << exitStatus << endl;
    else
        kdDebug( 9999 ) << " *** Process " << proc->name() << " was killed with exit status = " << exitStatus << endl;

    // delete proc
    delete proc; proc = 0;

    // Now show a message about operation ending status
    if (diff.isEmpty() && (exitStatus != 0))
    {
        KMessageBox::information( 0, i18n("Operation aborted (process killed)."), i18n("CVS Diff") );
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
        KMessageBox::information( 0, i18n("There is no difference to the repository."), i18n("No Difference Found") );
        return;
    }

    Q_ASSERT( diffFrontend() );
    diffFrontend()->showDiff( diff );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::receivedStdout( KProcess*, char* buffer, int buflen )
{
    stdOut += QString::fromUtf8( buffer, buflen );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::receivedStderr( KProcess*, char* buffer, int buflen )
{
    stdErr += QString::fromUtf8( buffer, buflen );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotStopButtonClicked( KDevPlugin* which )
{
    if ( which != 0 && which != this )
        return;
    if ( !proc )
        return;
    if ( !proc->kill() ) {
        KMessageBox::sorry( 0, i18n("Unable to kill process, you might want to kill it by hand.") );
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString CvsPart::buildCommitCmd( const QString _directoryName, const QStringList &paths, const QString _logMessage )
{
    CvsOptions *options = CvsOptions::instance();

    QString command( "cd " );
    command += KShellProcess::quote( _directoryName );
    command += " && ";
    command += cvs_rsh(); // yes, it is already quoted
    command += " cvs ";
    command += options->cvs();
    command += " commit ";
    command += options->commit();
    command += " -m " + KShellProcess::quote( _logMessage );
    command += " ";
    command += paths.join( " " );

    return command;
}

///////////////////////////////////////////////////////////////////////////////

// Four spaces for every log line (except the first which includes the
// developers name)
#define CHANGELOG_PREPEND_STRING QString( "    " )

void CvsPart::commit( const KURL::List& urlList )
{
    if (!prepareOperation( opCommit ))
        return;

    kdDebug(9000) << "CvsPart::commit() here!" << endl;

    CommitDialog dlg;
    if (dlg.exec() == QDialog::Rejected)
        return;

    QStringList fileList = quoted( URLUtil::toRelativePaths( project()->projectDirectory(), urlList ) );
    // 1. Commit changes to the actual file
    QString command = buildCommitCmd( project()->projectDirectory(), fileList, dlg.logMessage().join( "\n" ) );

    kdDebug( 9000 ) << "I'll run commit with this command: " << command << endl;

    m_widget->startCommand( project()->projectDirectory(), command ); // makeFrontend()->queueCommand(dirName, command);

    // 2. if requested to do so, add an entry to the Changelog too
    if (dlg.mustAddToChangeLog())
    {
        // 2.1 Modify the Changelog
        ChangeLogEntry entry;
        entry.addLines( dlg.logMessage() );
        entry.addToLog( this->project()->projectDirectory() + "/ChangeLog" );

        kdDebug( 9999 ) << " *** ChangeLog entry : " << entry.toString( CHANGELOG_PREPEND_STRING ) << endl;

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

    CvsOptions *options = CvsOptions::instance();
    QStringList fileList = quoted( URLUtil::toRelativePaths( project()->projectDirectory(), urlList ) );

    QString command("cd ");
    command += KShellProcess::quote( project()->projectDirectory() );
    command += " && " + cvs_rsh() + " cvs ";
    command += options->cvs();
    command += " update ";
    command += options->update();
    command += " ";
    command += fileList.join( " " ); // yeah, already quoted!

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( project()->projectDirectory(), command );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::add( const KURL::List& urlList )
{
    if (!prepareOperation( opAdd ))
        return;

    kdDebug(9000) << "CvsPart::add() here" << endl;

    CvsOptions *options = CvsOptions::instance();
    QStringList fileList = quoted( URLUtil::toRelativePaths( project()->projectDirectory(), urlList ) );

    QString command("cd ");
    command += KShellProcess::quote( project()->projectDirectory() );
    command += " && " + cvs_rsh() + " cvs ";
    command += options->cvs();
    command += " add ";
    command += options->add();
    command += " ";
    command += fileList.join( " " );


//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( project()->projectDirectory(), command );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::remove( const KURL::List& urlList )
{
    if (!prepareOperation( opRemove ))
        return;

    kdDebug(9000) << "CvsPart::remove() here" << endl;

    CvsOptions *options = CvsOptions::instance();
    QStringList fileList = quoted( URLUtil::toRelativePaths( project()->projectDirectory(), urlList ) );

    kdDebug(9000) << "These are the KURLs I'm going to remove:" << endl;
    URLUtil::dump( urlList );

    kdDebug(9000) << "Which translate in the following relative (to projectDir) pathnames: " << fileList.join(" ") << endl;

    QString command("cd ");
    command += KShellProcess::quote( project()->projectDirectory() );
    command += " && " + cvs_rsh() + " cvs ";
    command += options->cvs();
    command += " remove ";
    command += options->remove();
    command += " ";
    command += fileList.join( " " );

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( project()->projectDirectory(), command );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::revert( const KURL::List& urlList )
{
    if (!prepareOperation( opRevert ))
        return;

    kdDebug(9000) << "CvsPart::revert() here" << endl;

    CvsOptions *options = CvsOptions::instance();
    QStringList fileList = quoted( URLUtil::toRelativePaths( project()->projectDirectory(), urlList ) );

    QString command("cd ");
    command += KShellProcess::quote( project()->projectDirectory() );
    command += " && " + cvs_rsh() + " cvs ";
    command += options->cvs();
    command += " update ";
    command += options->revert();
    command += " ";
    command += fileList.join( " " );

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( project()->projectDirectory(), command );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::log( const KURL::List& urlList )
{
    if (!prepareOperation( opLog ))
        return;

    kdDebug(9000) << "CvsPart::log() here: " << endl;
    /// \FIXME These should be quoted ...
    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    LogForm* f = new LogForm();
    f->show();
    // Form will do all the work
    f->start( project()->projectDirectory(), fileList );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::diff( const KURL::List& urlList )
{
    if ( proc ) {
        KMessageBox::sorry( 0, i18n("Another diff operation is pending.") );
        return;
    }

    if (!prepareOperation( opDiff ))
        return;

    CvsOptions *options = CvsOptions::instance();
    QStringList fileList = URLUtil::toRelativePaths( project()->projectDirectory(), urlList );

    kdDebug(9000) << "CvsPart::diff() here" << endl;

    proc = new KProcess();
    proc->setWorkingDirectory( project()->projectDirectory() );

    stdOut = QString::null;
    stdErr = QString::null;
    QString str = options->cvs();

    QStringList args;

    args << "cvs";
    if (str.length())
    {
        QStringList list = QStringList::split(' ',str);
        for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            args << *it;
        }
    }

    args << "diff";
    str = options->diff();
    if (str.length())
    {
        QStringList list = QStringList::split(' ',str);
        for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            args << *it;
        }
    }

    QString crsh = options->rsh();
    if ( !crsh.isEmpty() )
        proc->setEnvironment( "CVS_RSH", crsh );

    args += fileList;

    *proc <<  args;

    kdDebug(9000) << "Running cvs diff with these args: " << args.join( " " ) << endl;

    bool ok = proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
    if ( !ok )
    {
        KMessageBox::error( 0, i18n("Could not invoke CVS."), i18n("Error Invoking Command") );
        delete proc; proc = 0;
        return;
    }

    connect( proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotDiffFinished()) );
    connect( proc, SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(receivedStdout(KProcess*,char*,int)) );
    connect( proc, SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(receivedStderr(KProcess*,char*,int)) );

    core()->running( this, true );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::tag( const KURL::List& urlList )
{
    kdDebug(9000) << "CvsPart::tag() here" << endl;

    if (!prepareOperation( opTag ))
        return;

    TagDialog dlg( mainWindow()->main()->centralWidget() );
    if (dlg.exec() != QDialog::Accepted)
        return;

    CvsOptions *options = CvsOptions::instance();
    QStringList fileList = quoted( URLUtil::toRelativePaths( project()->projectDirectory(), urlList ) );
    QString files = fileList.join( " " );

    QString command("cd ");
    command += KShellProcess::quote( project()->projectDirectory() );
    command += " && " + cvs_rsh() + " cvs ";
    command += options->cvs();
    command += " tag ";
    if (dlg.isBranch())
    {
        command += " -b " + dlg.branchName();
    }
    else
    {
        command += dlg.tagName();
    }
    command += " ";
    command += files;

    kdDebug(9000) << "Running: " << command << endl;

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( project()->projectDirectory(), command );

    doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::addToIgnoreList( const KURL::List& urlList )
{
	if ( !project() ) return;

    CvsUtils::addToIgnoreList( project()->projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::removeFromIgnoreList( const KURL::List& urlList )
{
	if ( !project() ) return;

    CvsUtils::removeFromIgnoreList( project()->projectDirectory(), urlList );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAddFilesToProject( const QStringList &filesToAdd )
{
    kdDebug( 9000 ) << "====> CvsPart::slotAddFilesToProject(const QStringList &)" << endl;

    int s = KMessageBox::questionYesNo( 0,
        i18n("Do you want to be added to CVS repository too?"),
        i18n("CVS - New Files Added to Project"),
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
        i18n("Do you want them to be removed from CVS repository too?\nWarning: They will be removed from disk too."),
        i18n("CVS - Files Removed From Project"),
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

    // Avoid bothering the user if this project has no support for CVS
    if (!project() || !isValidDirectory( project()->projectDirectory() ))
    {
        kdDebug(9006) << "Project has no CVS Support: too bad!! :-(" << endl;
        return;
    }

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
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotProjectClosed()
{
    kdDebug(9000) << "CvsPart::slotProjectClosed() here!" << endl;

    // Avoid bothering the user if this project has no support for CVS
    if (!project() || !isValidDirectory( project()->projectDirectory() ))
    {
        kdDebug(9006) << "Project has no CVS Support: too bad!! :-(" << endl;
        return;
    }

    CvsOptions *options = CvsOptions::instance();
    options->save( *projectDom() );
    delete options;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::isValidDirectory( const QString &dirPath ) const
{
    QString cvs = "/CVS/";
    QDir cvsdir( dirPath + cvs );
    QString entriesFileName = dirPath + cvs + "Entries";
    QString rootFileName = dirPath + cvs + "Root";
    QString repoFileName = dirPath + cvs + "Repository";

    return cvsdir.exists() &&
        QFile::exists( entriesFileName ) &&
        QFile::exists( rootFileName ) &&
        QFile::exists( repoFileName );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox = dlg->addVBoxPage( i18n("CVS") );
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
