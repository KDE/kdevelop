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

#include "urlfileutilities.h"
#include "svnentry.h"
#include "changelog.h"
#include "svnpart.h"
#include "svnwidget.h"
#include "svnoptions.h"
#include "commitdlg.h"
#include "logform.h"
#include "svnform.h"
#include "execcommand.h"
#include "svnoptionswidget.h"

using namespace UrlFileUtilities;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

const QString changeLogFileName = "ChangeLog";

///////////////////////////////////////////////////////////////////////////////
// Global vars
///////////////////////////////////////////////////////////////////////////////

// This is an ugly hack for being able to pass SVN_RSH from SvnPart::create
QString g_tempEnvRsh( "" );

///////////////////////////////////////////////////////////////////////////////
// Plugin factory
///////////////////////////////////////////////////////////////////////////////

typedef KGenericFactory<SvnPart> SvnFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevsvn, SvnFactory( "kdevsvn" ) );

///////////////////////////////////////////////////////////////////////////////
// class SvnPart
///////////////////////////////////////////////////////////////////////////////

SvnPart::SvnPart( QObject *parent, const char *name, const QStringList & )
	: KDevVersionControl( "KDevSvnPart", "kdevsvnpart", parent, name ? name : "SVN" ),
	proc( 0 ),
	actionCommit( 0 ), actionDiff( 0 ),	actionLog( 0 ),	actionAdd( 0 ),	actionRemove( 0 ),
	actionUpdate( 0 ), actionRevert( 0 )
{
	setInstance( SvnFactory::instance() );

	setXMLFile( "kdevsvnpart.rc" );

	init();
}

///////////////////////////////////////////////////////////////////////////////

SvnPart::~SvnPart()
{
	if (m_widget)
	{
		mainWindow()->removeView( m_widget ); // Inform toplevel, that the output view is gone
	}
	delete m_widget;
	delete m_svnConfigurationForm;
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::init()
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

	m_widget = new SvnWidget( this );

	mainWindow()->embedOutputView( m_widget, i18n("SVN"), i18n("svn output") ); // i18n("SVN") ?!?? ;)
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::setupActions()
{
/*
	KAction * action = new KAction( i18n("Import Svn Repository..."),"wizard", 0,
		this, SLOT(slotImportSvn()), actionCollection(), "svn_import" );
		action->setStatusText( i18n("Imports an existing SVN repository.") );
*/
	actionCommit = new KAction( i18n("Commit"), 0, this, SLOT(slotActionCommit()),
		actionCollection(), "svn_commit" );
	actionDiff = new KAction( i18n("Diff"), 0, this, SLOT(slotActionDiff()),
		actionCollection(), "svn_diff" );
	actionLog = new KAction( i18n("Log"), 0, this, SLOT(slotActionLog()),
		actionCollection(), "svn_log" );
	actionAdd = new KAction( i18n("Add"), 0, this, SLOT(slotActionAdd()),
		actionCollection(), "svn_add" );
	actionRemove = new KAction( i18n("Remove From Repository"), 0, this, SLOT(slotActionRemove()),
		actionCollection(), "svn_remove" );
	actionUpdate = new KAction( i18n("Update"), 0, this, SLOT(slotActionUpdate()),
		actionCollection(), "svn_update" );
	actionRevert = new KAction( i18n("Revert"), 0, this, SLOT(slotActionRevert()),
		actionCollection(), "svn_revert" );
}

///////////////////////////////////////////////////////////////////////////////

QString SvnPart::svn_rsh() const
{
	SvnOptions *options = SvnOptions::instance();
	QString env = options->rsh();
	if ( !env.isEmpty() )
		return QString( "SVN_RSH=" ) + KShellProcess::quote( env );
	return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString SvnPart::currentDocument()
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
bool SvnPart::isRegisteredInRepository()
{
	kdDebug(9027) << "===> SvnPart::isRegisteredInRepository() here! " << endl;

	if ( pathUrl.path() == project()->projectDirectory() )
	{
		kdDebug(9000) << "===> Operation requested for projectDir(): true. " << endl;
		return true;
	}

	QString dirName = pathUrl.directory();
	QString entriesFilePath = dirName + "/SVN/Entries";

	kdDebug(9027) << "===> pathUrl.path()      = " << pathUrl.path() << endl;
	kdDebug(9027) << "===> dirName             = " << dirName << endl;
	kdDebug(9027) << "===> entriesFilePath = " << entriesFilePath << endl;

	bool found = false;
	QFile f( entriesFilePath );
	if (f.open( IO_ReadOnly ))
	{
		QTextStream t( &f );
		SvnEntry svnEntry;
		while (svnEntry.read( t ) && !found)
		{
			if (svnEntry.fileName == pathUrl.fileName())
			{
				kdDebug(9027) << "===> Wow!! *** Found it!!! *** " << endl;
				found = true;
			}
		}
	}
	else
	{
		kdDebug(9027) << "===> Error: could not open SVN/Entries!! " << endl;
	}
	f.close();

	return found;
}
#endif

///////////////////////////////////////////////////////////////////////////////

void SvnPart::slotImportSvn()
{
	// TODO
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::createNewProject( const QString& dirName )
{
	kdDebug( 9027 ) << "====> SvnPart::createNewProject( const QString& )" << endl;

	if (!m_svnConfigurationForm)
		return;
        QString cmd;
        QTextStream out( &cmd, IO_WriteOnly );

        if( m_svnConfigurationForm->init_check->isChecked() )
        {
            out << "svnadmin create " << KShellProcess::quote(m_svnConfigurationForm->repository_edit->text()) << " && ";
        }

        out << "cd " << KShellProcess::quote( dirName ) << " "
            << "svn import " << KShellProcess::quote(m_svnConfigurationForm->repository_edit->text()) << " "
            << "." << " "
            << "-m " << KShellProcess::quote(m_svnConfigurationForm->message_edit->text()) << " "
	    << "&& "
	    << "rm -rf " << KShellProcess::quote( dirName ) << " "
	    << "&& "
	    << "svn checkout " << KShellProcess::quote(m_svnConfigurationForm->repository_edit->text()) << " "
	    << KShellProcess::quote(dirName);

/*
	if (m_svnConfigurationForm->init_check->isChecked())
	{
            init = "svnadmin create " + KShellProcess::quote(m_svnConfigurationForm->repository_edit->text()) + " && ";
	}
	QString command = init + "cd " + KShellProcess::quote(dirName) + " && " +
		"svn import " +
                "-m " + KShellProcess::quote(m_svnConfigurationForm->message_edit->text()) + " "
                + KShellProcess::quote(m_svnConfigurationForm->repository_edit->text()) + " "
                KShellProcess::quote(m_svnConfigurationForm->root_edit->text()) +
		 +
		KShellProcess::quote(m_svnConfigurationForm->release_edit->text())

		KShellProcess::quote(m_svnConfigurationForm->repository_edit->text()) + " " +
		KShellProcess::quote(m_svnConfigurationForm->root_edit->text());

	kdDebug( 9027 ) << "  ** Will run the following command: " << endl << command << endl;
	kdDebug( 9027 ) << "  ** on directory: " << dirName << endl;
*/
	// makeFrontend()->queueCommand( dirName, cmd );
        kdDebug(9027) << cmd << endl;
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::contextMenu( QPopupMenu *popup, const Context *context )
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

		popup->insertItem( i18n("SVN"), subMenu );
	}
}

///////////////////////////////////////////////////////////////////////////////

QString SvnPart::buildSvnCommand( const QString& fileName, const QString& cmd, const QString& options ) const
{
	SvnOptions *svnOptions = SvnOptions::instance();

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
	command += svn_rsh(); // yes, it is already quoted
	command += " svn ";
	command += svnOptions->svn();
	command += cmd;
	command += options;
	if ( !file.isEmpty() )
		command += " " + KShellProcess::quote( file );

	return command;
}

void SvnPart::commit( const QString& fileName )
{
	if ( fileName.isEmpty() )
		return;

	CommitDialog dlg;
	if (dlg.exec() == QDialog::Rejected)
		return;

	// 1. Commit changes to the actual file
	QString command = buildSvnCommand( fileName, "commit",
					  SvnOptions::instance()->commit() + " -m " +
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

void SvnPart::update( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    SvnOptions *options = SvnOptions::instance();

    QString command = buildSvnCommand( fileName, "update", options->update() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::add( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    SvnOptions *options = SvnOptions::instance();

    QString command = buildSvnCommand( fileName, "add", options->add() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::remove( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    SvnOptions *options = SvnOptions::instance();

    QString command = buildSvnCommand( fileName, "remove", options->remove() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::revert( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    SvnOptions *options = SvnOptions::instance();

    QString command = buildSvnCommand( fileName, "revert", options->revert() );

    m_widget->startCommand( QString::null, command );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::log( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    LogForm* f = new LogForm();
    f->show();
    // Form will do all the work
    QString projectDir = project()->projectDirectory();
    QString relPath = UrlFileUtilities::extractPathNameRelative( projectDir, fileName );

    f->start( projectDir, relPath );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::diff( const QString& fileName )
{
	if ( fileName.isEmpty() )
		return;

	if ( proc ) {
		KMessageBox::sorry( 0, i18n("Another diff operation is pending.") );
		return;
	}

	SvnOptions *options = SvnOptions::instance();

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
	QString str = options->svn();

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
		proc->setEnvironment( "SVN_RSH", crsh );
	proc->setWorkingDirectory( dir );
	*proc << "svn";
	*proc << args;

	bool ok = proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
	if ( !ok ) {
		KMessageBox::error( 0, i18n("Could not invoke SVN"), i18n("Error Invoking Command") );
		delete proc; proc = 0;
		return;
	}

	connect( proc, SIGNAL(processExited(KProcess*)),this, SLOT(slotDiffFinished()) );
	connect( proc, SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(receivedStdout(KProcess*,char*,int)) );
	connect( proc, SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(receivedStderr(KProcess*,char*,int)) );

	core()->running( this, true );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::slotDiffFinished()
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
        KMessageBox::information( 0, i18n("Operation aborted (process killed)"), i18n("SVN Diff") );
		return;
	}
    if ( diff.isEmpty() && !err.isEmpty() )
	{
        KMessageBox::detailedError( 0, i18n("SVN outputted errors during diff."), err, i18n("Errors During Diff") );
        return;
    }

    if ( !err.isEmpty() )
	{
        int s = KMessageBox::warningContinueCancelList( 0, i18n("SVN outputted errors during diff. Do you still want to continue?"),
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

void SvnPart::receivedStdout( KProcess*, char* buffer, int buflen )
{
	stdOut += QString::fromUtf8( buffer, buflen );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::receivedStderr( KProcess*, char* buffer, int buflen )
{
	stdErr += QString::fromUtf8( buffer, buflen );
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::slotStopButtonClicked( KDevPlugin* which )
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

void SvnPart::slotProjectOpened()
{
	SvnOptions *options = SvnOptions::instance();
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

void SvnPart::slotProjectClosed()
{
	SvnOptions *options = SvnOptions::instance();
	options->save( *projectDom() );
	delete options;
}

///////////////////////////////////////////////////////////////////////////////

void SvnPart::projectConfigWidget( KDialogBase *dlg )
{
	QVBox *vbox = dlg->addVBoxPage( i18n("SVN") );
	SvnOptionsWidget *w = new SvnOptionsWidget( this, (QWidget *)vbox, "svn config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

///////////////////////////////////////////////////////////////////////////////

QWidget* SvnPart::newProjectWidget( QWidget *parent )
{
	m_svnConfigurationForm = new SvnForm( parent, "svnform" );
	return m_svnConfigurationForm;
}

void SvnPart::slotCommit()
{ commit( popupfile ); }
void SvnPart::slotUpdate()
{ update( popupfile ); }
void SvnPart::slotAdd()
{ add( popupfile ); }
void SvnPart::slotRemove()
{ remove( popupfile ); }
void SvnPart::slotRevert()
{ revert( popupfile ); }
void SvnPart::slotLog()
{ log( popupfile ); }
void SvnPart::slotDiff()
{ diff( popupfile ); }

void SvnPart::slotActionCommit()
{ commit( currentDocument() ); }
void SvnPart::slotActionUpdate()
{ update( currentDocument() ); }
void SvnPart::slotActionAdd()
{ add( currentDocument() ); }
void SvnPart::slotActionRemove()
{ remove( currentDocument() ); }
void SvnPart::slotActionRevert()
{ revert( currentDocument() ); }
void SvnPart::slotActionLog()
{ log( currentDocument() ); }
void SvnPart::slotActionDiff()
{ diff( currentDocument() ); }

#include "svnpart.moc"
