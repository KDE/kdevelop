/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
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

#include <kparts/part.h>
#include <kdevpartcontroller.h>
#include <kgenericfactory.h>

#include <qfileinfo.h>
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

#include "cvsentry.h"
#include "changelog.h"
#include "cvspart.h"
#include "cvswidget.h"
#include "cvsoptions.h"
#include "commitdlg.h"
#include "logform.h"
#include "cvsform.h"
#include "execcommand.h"
#include "cvsoptionswidget.h"

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
K_EXPORT_COMPONENT_FACTORY( libkdevcvs, CvsFactory( "kdevcvs" ) );

///////////////////////////////////////////////////////////////////////////////
// class CvsPart
///////////////////////////////////////////////////////////////////////////////

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & )
	: KDevVersionControl( "KDevCvsPart", "kdevcvspart", parent, name ? name : "CVS" ),
	proc( 0 ),
	actionCommit( 0 ), actionDiff( 0 ),	actionLog( 0 ),	actionAdd( 0 ),	actionRemove( 0 ),
	actionUpdate( 0 ), actionRevert( 0 )
{
	setInstance( CvsFactory::instance() );

	setXMLFile( "kdevcvspart.rc" );

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
	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
		this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
		this, SLOT(slotStopButtonClicked(KDevPlugin*)) );

	m_widget = new CvsWidget( this );

	mainWindow()->embedOutputView( m_widget, i18n("CVS"), i18n("cvs output") ); // i18n("CVS") ?!?? ;)
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::setupActions()
{
/*
	KAction * action = new KAction( i18n("Import Cvs Repository..."),"wizard", 0,
		this, SLOT(slotImportCvs()), actionCollection(), "cvs_import" );
		action->setStatusText( i18n("Imports an existing CVS repository.") );
*/
	actionCommit = new KAction( i18n("Commit"), 0, this, SLOT(slotActionCommit()),
		actionCollection(), "cvs_commit" );
	actionDiff = new KAction( i18n("Diff"), 0, this, SLOT(slotActionDiff()),
		actionCollection(), "cvs_diff" );
	actionLog = new KAction( i18n("Log"), 0, this, SLOT(slotActionLog()),
		actionCollection(), "cvs_log" );
	actionAdd = new KAction( i18n("Add"), 0, this, SLOT(slotActionAdd()),
		actionCollection(), "cvs_add" );
	actionRemove = new KAction( i18n("Remove From Repository"), 0, this, SLOT(slotActionRemove()),
		actionCollection(), "cvs_remove" );
	actionUpdate = new KAction( i18n("Update"), 0, this, SLOT(slotActionUpdate()),
		actionCollection(), "cvs_update" );
	actionRevert = new KAction( i18n("Revert"), 0, this, SLOT(slotActionRevert()),
		actionCollection(), "cvs_revert" );
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

QString CvsPart::currentDocument()
{
    KParts::ReadOnlyPart *part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
    if ( part ) {
	if ( part->url().isLocalFile() ) {
	    kdDebug(9027) << "cannot handle non-local files!" << endl;
	}

	return part->url().path();
    }

    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

#if 0
bool CvsPart::isRegisteredInRepository()
{
	kdDebug(9027) << "===> CvsPart::isRegisteredInRepository() here! " << endl;

	if ( pathUrl.path() == project()->projectDirectory() )
	{
		kdDebug(9000) << "===> Operation requested for projectDir(): true. " << endl;
		return true;
	}

	QString dirName = pathUrl.directory();
	QString entriesFilePath = dirName + "/CVS/Entries";

	kdDebug(9027) << "===> pathUrl.path()      = " << pathUrl.path() << endl;
	kdDebug(9027) << "===> dirName             = " << dirName << endl;
	kdDebug(9027) << "===> entriesFilePath = " << entriesFilePath << endl;

	bool found = false;
	QFile f( entriesFilePath );
	if (f.open( IO_ReadOnly ))
	{
		QTextStream t( &f );
		CvsEntry cvsEntry;
		while (cvsEntry.read( t ) && !found)
		{
			if (cvsEntry.fileName == pathUrl.fileName())
			{
				kdDebug(9027) << "===> Wow!! *** Found it!!! *** " << endl;
				found = true;
			}
		}
	}
	else
	{
		kdDebug(9027) << "===> Error: could not open CVS/Entries!! " << endl;
	}
	f.close();

	return found;
}
#endif

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotImportCvs()
{
	// TODO
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::createNewProject( const QString& dirName )
{
	kdDebug( 9027 ) << "====> CvsPart::createNewProject( const QString& )" << endl;

	if (!m_cvsConfigurationForm)
		return;
	QString init("");

	// FIXME: Store rsh setting. Here doesn't store it in CvsOptions because:
	// createNewProject() is called _before_ projectOpened() signal is emitted: the latter
	// Is it possible to postpone it? Or have we to fall back to some kind of temporary variable/
	// flag approach?
	// For now, one must re-set CVS_RSH in Project->Options->Cvs
	g_tempEnvRsh = m_cvsConfigurationForm->cvs_rsh->text();

	if (m_cvsConfigurationForm->init_check->isChecked())
	{
    	init = "cvs -d " + KShellProcess::quote(m_cvsConfigurationForm->root_edit->text()) + " init && ";
	}
	QString cvs_rsh;
	if ( !g_tempEnvRsh.isEmpty() )
		cvs_rsh = "CVS_RSH=" + KShellProcess::quote( g_tempEnvRsh );
	QString command = init + "cd " + KShellProcess::quote(dirName) +
		" && " + cvs_rsh +
		" cvs -d " + KShellProcess::quote(m_cvsConfigurationForm->root_edit->text()) +
		" import -m " + KShellProcess::quote(m_cvsConfigurationForm->message_edit->text()) + " " +
		KShellProcess::quote(m_cvsConfigurationForm->repository_edit->text()) + " " +
		KShellProcess::quote(m_cvsConfigurationForm->vendor_edit->text()) + " " +
		KShellProcess::quote(m_cvsConfigurationForm->release_edit->text()) + " && sh " +
		locate("data","kdevcvs/buildcvs.sh") + " . " +
		KShellProcess::quote(m_cvsConfigurationForm->repository_edit->text()) + " " +
		KShellProcess::quote(m_cvsConfigurationForm->root_edit->text());

	kdDebug( 9027 ) << "  ** Will run the following command: " << endl << command << endl;
	kdDebug( 9027 ) << "  ** on directory: " << dirName << endl;

	makeFrontend()->queueCommand( dirName, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::contextMenu( QPopupMenu *popup, const Context *context )
{
	if (context->hasType( "file" ))
	{
		const FileContext *fcontext = static_cast<const FileContext*>( context );
		// FIXME: Here we must hope that fcontext->fileName() returns an absolute path ;(
		popupfile = fcontext->fileName();
		QFileInfo fi( popupfile );
		QString fileName = fi.fileName();

		popup->insertSeparator();

		KPopupMenu *subMenu = new KPopupMenu( popup );
		subMenu->insertTitle( i18n("Actions for %1").arg(fileName) );

		subMenu->insertItem( actionCommit->text(), this, SLOT(slotCommit()) );
		subMenu->insertItem( actionLog->text(), this, SLOT(slotLog()) );
		subMenu->insertItem( actionUpdate->text(), this, SLOT(slotUpdate()) );
		subMenu->insertItem( actionAdd->text(), this, SLOT(slotAdd()) );
		subMenu->insertItem( actionDiff->text(), this, SLOT(slotDiff()) );

		subMenu->insertSeparator();

		subMenu->insertItem( actionRemove->text(), this, SLOT(slotRemove()) );
		subMenu->insertItem( actionRevert->text(), this, SLOT(slotRevert()) );

		popup->insertItem( i18n("CVS"), subMenu );
	}
}

///////////////////////////////////////////////////////////////////////////////

QString CvsPart::buildCvsCommand( const QString& fileName, const QString& cmd, const QString& options ) const
{
	CvsOptions *cvsOptions = CvsOptions::instance();

	QString dir;
	QString file;
	QFileInfo fi( fileName );
	
	if (fi.isDir()) {
		dir = fi.absFilePath();
	} else {
		dir = fi.dirPath();
		file = fi.fileName();
	}

	QString command( "cd " );
	command += KShellProcess::quote( dir );
	command += " && ";
	command += cvs_rsh(); // yes, it is already quoted
	command += " cvs ";
	command += cvsOptions->cvs();
	command += cmd;
	command += options;
	if ( !file.isEmpty() )
		command += " " + KShellProcess::quote( file );

	return command;
}

void CvsPart::commit( const QString& fileName )
{
	if ( fileName.isEmpty() )
		return;

	CommitDialog dlg;
	if (dlg.exec() == QDialog::Rejected)
		return;

	// 1. Commit changes to the actual file
	QString command = buildCvsCommand( fileName, "commit",
					  CvsOptions::instance()->commit() + " -m " +
					  KShellProcess::quote( dlg.logMessage().join( "" ) ) );
	m_widget->startCommand( QString::null, command );

	// if requested to do so, add an entry to the Changelog too
	if (dlg.mustAddToChangeLog())
	{
		// modify the Changelog
		ChangeLogEntry entry;
		entry.addLines( dlg.logMessage() );
		entry.addToLog( this->project()->projectDirectory() + "/ChangeLog" );
	}
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::update( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    CvsOptions *options = CvsOptions::instance();

    QString command = buildCvsCommand( fileName, "update", options->update() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::add( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    CvsOptions *options = CvsOptions::instance();

    QString command = buildCvsCommand( fileName, "add", options->add() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::remove( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    CvsOptions *options = CvsOptions::instance();

    QString command = buildCvsCommand( fileName, "remove", options->remove() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::revert( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    CvsOptions *options = CvsOptions::instance();

    QString command = buildCvsCommand( fileName, "update", options->revert() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::log( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    LogForm* f = new LogForm();
    f->show();
    // Form will do all the work
    QString projectDir = project()->projectDirectory();
    QString relPath = URLUtil::extractPathNameRelative( projectDir, fileName );

    f->start( projectDir, relPath );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::diff( const QString& fileName )
{
	if ( fileName.isEmpty() )
		return;

	if ( proc ) {
		KMessageBox::sorry( 0, i18n("Another diff operation is pending.") );
		return;
	}

	CvsOptions *options = CvsOptions::instance();

        QFileInfo fi( fileName );
	QString file, dir;
        if (fi.isDir()) {
                dir = fi.absFilePath();
        } else {
                dir = fi.dirPath();
                file = fi.fileName();
        }

	QStringList args;
	proc = new KProcess();
	stdOut = QString::null;
	stdErr = QString::null;
	QString str = options->cvs();

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
	if ( !file.isEmpty() )
		args << file;

	QString crsh = options->rsh();
	if ( !crsh.isEmpty() )
		proc->setEnvironment( "CVS_RSH", crsh );
	proc->setWorkingDirectory( dir );
	*proc << "cvs";
	*proc << args;

	bool ok = proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
	if ( !ok ) {
		KMessageBox::error( 0, i18n("Could not invoke CVS"), i18n("Error Invoking Command") );
		delete proc; proc = 0;
		return;
	}

	connect( proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotDiffFinished()) );
	connect( proc, SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(receivedStdout(KProcess*,char*,int)) );
	connect( proc, SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(receivedStderr(KProcess*,char*,int)) );

	core()->running( this, true );
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

	kdDebug( 9027 ) << "diff = " << diff << endl;
	kdDebug( 9027 ) << "err = " << err << endl;

	if (normalExit)
		kdDebug( 9027 ) << " *** Process " << proc->name() << " died nicely with exit status = " << exitStatus << endl;
	else
		kdDebug( 9027 ) << " *** Process " << proc->name() << " was killed with exit status = " << exitStatus << endl;

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

void CvsPart::slotProjectOpened()
{
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

}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotProjectClosed()
{
	CvsOptions *options = CvsOptions::instance();
	options->save( *projectDom() );
	delete options;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::projectConfigWidget( KDialogBase *dlg )
{
	QVBox *vbox = dlg->addVBoxPage( i18n("CVS") );
	CvsOptionsWidget *w = new CvsOptionsWidget( this, (QWidget *)vbox, "cvs config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

///////////////////////////////////////////////////////////////////////////////

QWidget* CvsPart::newProjectWidget( QWidget *parent )
{
	m_cvsConfigurationForm = new CvsForm( parent, "cvsform" );
	return m_cvsConfigurationForm;
}

void CvsPart::slotCommit()
{ commit( popupfile ); }
void CvsPart::slotUpdate()
{ update( popupfile ); }
void CvsPart::slotAdd()
{ add( popupfile ); }
void CvsPart::slotRemove()
{ remove( popupfile ); }
void CvsPart::slotRevert()
{ revert( popupfile ); }
void CvsPart::slotLog()
{ log( popupfile ); }
void CvsPart::slotDiff()
{ diff( popupfile ); }

void CvsPart::slotActionCommit()
{ commit( currentDocument() ); }
void CvsPart::slotActionUpdate()
{ update( currentDocument() ); }
void CvsPart::slotActionAdd()
{ add( currentDocument() ); }
void CvsPart::slotActionRemove()
{ remove( currentDocument() ); }
void CvsPart::slotActionRevert()
{ revert( currentDocument() ); }
void CvsPart::slotActionLog()
{ log( currentDocument() ); }
void CvsPart::slotActionDiff()
{ diff( currentDocument() ); }

#include "cvspart.moc"
