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

#include <qfileinfo.h>
#include <qpopupmenu.h>
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

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qguardedptr.h>

#include "kdevcore.h"
#include "kdevmakefrontend.h"
#include "kdevdifffrontend.h"
#include "kdevappfrontend.h"
#include "commitdlg.h"
#include "logform.h"
#include "cvsform.h"
#include "execcommand.h"
#include "cvsoptionswidget.h"
#include "domutil.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"

#include "urlfileutilities.h"
#include "cvsentry.h"
#include "cvspart.h"
#include "cvswidget.h"
#include "cvs_commonoptions.h"

#define zero_delete(p) { delete (p); (p) = 0; }

typedef KGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvs, CvsFactory( "kdevcvs" ) );

using namespace UrlFileUtilities;

///////////////////////////////////////////////////////////////////////////////
// class CvsPart
///////////////////////////////////////////////////////////////////////////////

// Private implementation
struct CvsPart::Private
{
	Private( CvsPart *part ) : invokedFromMenu( true ),
		proc( 0 ),
		actionCommit( 0 ), actionDiff( 0 ),	actionLog( 0 ),	actionAdd( 0 ),	actionRemove( 0 ),
		actionUpdate( 0 ), actionReplace( 0 ),
		owner( part )
	{}

	// Setup actions.
	void setupActions();
	// Updates Url forn the currently focused document
	bool retrieveUrlFocusedDocument();
	// Retrieves the fileName and dirName from the pathUrl
	bool findPaths();
	// Returns true if the file or directory indicated in @p url has been registered in the CVS
	// (if not, returns false since it avoid performing CVS operation)
	bool isRegisteredInRepository();
	// Display "cvs diff" results in the diff part.
	void diffFinished( const QString& diff, const QString& err );
	// Call this every time a slot for cvs operations starts!! (It will setup the
	// state (file/dir URL, ...).
	// It will also display proper error messages so the caller must only exit if
	// it fails (return false); if return true than basic requisites for cvs operation
	// are satisfied.
	bool prepareOperation();
	// Call this every time a slot for cvs operations ends!! (It will restore the state for a new
	// operation).
	void doneOperation();

	// The value for overriding the $CVS_RSH env variable
	QString cvs_rsh() const;
	// Contains the url of the file or direcly for which the service has been invoked
	KURL pathUrl;
	// Contains the directory name
	QString dirName;
	// Contains the fileName (relative to dir)
	QString fileName;
	// Reference to widget integrated in the "bottom tabbar" (IDEAL)
	QGuardedPtr<CvsWidget> m_widget;
	// This is a pointer to the d->form used for collecting data about CVS project creation (used
	// by the ApplicationWizard in example)
	CvsForm *form;
	// True if invoked from menu, false otherwise (i.e. called from context menu)
	// Ok this is a very bad hack but I see no other solution for now.
	bool invokedFromMenu;

	KProcess* proc;
	QString stdOut, stdErr;

	// Actions
	KAction *actionCommit,
		*actionDiff,
		*actionLog,
		*actionAdd,
		*actionRemove,
		*actionUpdate,
		*actionReplace;

private:
	CvsPart *owner;
};

///////////////////////////////////////////////////////////////////////////////

void CvsPart::Private::setupActions()
{
/*
	KAction * action = new KAction( i18n("Import Cvs Repository..."),"wizard", 0,
		this, SLOT(slotImportCvs()), actionCollection(), "cvs_import" );
		action->setStatusText( i18n("Imports an existing CVS repository.") );
*/
	actionCommit = new KAction( i18n("Commit"), 0, owner, SLOT(slotCommit()),
		owner->actionCollection(), "cvs_commit" );
	actionDiff = new KAction( i18n("Diff"), 0, owner, SLOT(slotDiff()),
		owner->actionCollection(), "cvs_diff" );
	actionLog = new KAction( i18n("Log"), 0, owner, SLOT(slotLog()),
		owner->actionCollection(), "cvs_log" );
	actionAdd = new KAction( i18n("Add"), 0, owner, SLOT(slotAdd()),
		owner->actionCollection(), "cvs_add" );
	actionRemove = new KAction( i18n("Remove From Repository"), 0, owner, SLOT(slotRemove()),
		owner->actionCollection(), "cvs_remove" );
	actionUpdate = new KAction( i18n("Update"), 0, owner, SLOT(slotUpdate()),
		owner->actionCollection(), "cvs_update" );
	actionReplace = new KAction( i18n("Replace with Copy From Repository"), 0, owner, SLOT(slotReplace()),
		owner->actionCollection(), "cvs_replace" );
}

///////////////////////////////////////////////////////////////////////////////

QString CvsPart::Private::cvs_rsh() const
{
	QDomDocument &dom = *owner->projectDom();
	QString env = DomUtil::readEntry( dom, "/kdevcvs/rshoptions", default_rsh );
	if ( !env.isEmpty() )
		return QString( "CVS_RSH=" ) + KShellProcess::quote( env );
	return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::Private::retrieveUrlFocusedDocument()
{
	kdDebug(9000) << "CvsPart::retrieveUrlFocusedDocument() here!" << pathUrl.path() << endl;
	KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>( owner->partController()->activePart() );
	if (part)
	{
		pathUrl = part->url();
		kdDebug(9000) << "CvsPart::retrieveUrlFocusedDocument(): Using file " << pathUrl.path() << endl;
		return true;
	}
	else
	{
		kdDebug(9000) << "CvsPart::retrieveUrlFocusedDocument(): No active part found???" << endl;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::Private::findPaths()
{
	if (!owner->project())
	{
		kdDebug(9000) << "CvsPart::findPaths(): No project???" << endl;
		return false;
	}
	// From menu-bar invoked actions we use the currently focused document, otherwise we used the one
	// pointed by the context menu.
	if (invokedFromMenu && !retrieveUrlFocusedDocument())
	{
		kdDebug(9000) << "CvsPart::findPaths(): Unable to retrieve the Url for the active parte (is there any?)." << endl;
		return false;
	}

	kdDebug(9000) << "CvsPart::findPaths(): Project directory is " << owner->project()->projectDirectory() << endl;
	kdDebug(9000) << "CvsPart::findPaths(): Path is " << pathUrl.path() << endl;

	// Ok, this an *ugly* hack: if the operation is requested for the project directory
	// then the path must be "customized" because of the behaviour of KURL ...
	if ( pathUrl.path() != owner->project()->projectDirectory() )
	{
	    dirName = pathUrl.directory();
    	fileName = pathUrl.fileName();
	}
	else
	{
	    dirName = pathUrl.path();
    	fileName = ".";
	}

	kdDebug(9000) << "CvsPart::findPaths(): dirName : " << dirName << endl;
	kdDebug(9000) << "CvsPart::findPaths(): fileName: " << fileName << endl;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::Private::prepareOperation()
{
	if (!findPaths())
	{
		kdDebug(9000) << "  ** slotCommit(): aborting since findPath() == false." << endl;
		return false;
	}
	if (!isRegisteredInRepository())
	{
		kdDebug(9000) << "  ** slotDiff(): aborting since file is not is repository." << endl;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::Private::doneOperation()
{
	invokedFromMenu = true;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsPart::Private::isRegisteredInRepository()
{
	kdDebug(9000) << "===> CvsPart::isRegisteredInRepository() here! " << endl;

	if ( pathUrl.path() == owner->project()->projectDirectory() )
	{
		kdDebug(9000) << "===> Operation requested for projectDir(): true. " << endl;
		return true;
	}

	QString dirName = pathUrl.directory();
	QString entriesFilePath = dirName + "/CVS/Entries";

	kdDebug(9000) << "===> pathUrl.path()      = " << pathUrl.path() << endl;
	kdDebug(9000) << "===> dirName             = " << dirName << endl;
	kdDebug(9000) << "===> entriesFilePath = " << entriesFilePath << endl;
	kdDebug(9000) << "===> whatToSearch    = " << whatToSearch << endl;

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
				kdDebug(9000) << "===> Wow!! *** Found it!!! *** " << endl;
				found = true;
			}
		}
	}
	else
	{
		kdDebug(9000) << "===> Error: could not open CVS/Entries!! " << endl;
	}
	f.close();

	return found;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::Private::diffFinished( const QString& diff, const QString& err )
{
    if (diff.isNull() && err.isNull() )
	{
        kdDebug(9000) << "cvs diff cancelled" << endl;
        return; // user pressed cancel or an error occured
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

    Q_ASSERT( owner->diffFrontend() );
    owner->diffFrontend()->showDiff( diff );
}

///////////////////////////////////////////////////////////////////////////////
// class CvsPart
///////////////////////////////////////////////////////////////////////////////

CvsPart::CvsPart( QObject *parent, const char *name, const QStringList & )
	: KDevVersionControl( "KDevCvsPart", "kdevcvspart", parent, name ? name : "CVS" ),
	d( new Private(this) )
{
	setInstance( CvsFactory::instance() );

	setXMLFile( "kdevcvspart.rc" );

	d->setupActions();

	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
		this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
		this, SLOT(slotStopButtonClicked(KDevPlugin*)) );

	d->m_widget = new CvsWidget( this );
	mainWindow()->embedOutputView( d->m_widget, i18n("CVS"), i18n("cvs output") ); // i18n("CVS") ?!?? ;)
}

///////////////////////////////////////////////////////////////////////////////

CvsPart::~CvsPart()
{
	if (d->m_widget)
	{
		mainWindow()->removeView( d->m_widget ); // Inform toplevel, that the output view is gone
	}
	delete d->m_widget;
	delete d;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotImportCvs()
{
	// TODO
}

///////////////////////////////////////////////////////////////////////////////

QWidget* CvsPart::newProjectWidget( QWidget *parent )
{
    d->form = new CvsForm( parent );
    return d->form;
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::createNewProject( const QString& dirName )
{
	kdDebug( 9000 ) << "====> CvsPart::createNewProject( const QString& )" << endl;

	if (!d->form)
		return;
	QString init("");

	if (d->form->init_check->isChecked())
	{
    	init = "cvs -d " + KShellProcess::quote(d->form->root_edit->text()) + " init && ";
	}
    QString command = init + "cd " + KShellProcess::quote(dirName) +
		" && cvs -d " + KShellProcess::quote(d->form->root_edit->text()) +
		" import -m " + KShellProcess::quote(d->form->message_edit->text()) + " " +
		KShellProcess::quote(d->form->repository_edit->text()) + " " +
		KShellProcess::quote(d->form->vendor_edit->text()) + " " +
		KShellProcess::quote(d->form->release_edit->text()) + " && sh " +
		locate("data","kdevcvs/buildcvs.sh") + " . " +
		KShellProcess::quote(d->form->repository_edit->text()) + " " +
		KShellProcess::quote(d->form->root_edit->text());

	kdDebug( 9000 ) << "  ** Will run the following command: " << endl << command << endl;
	kdDebug( 9000 ) << "  ** on directory: " << dirName << endl;

	makeFrontend()->queueCommand( dirName, command );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::contextMenu( QPopupMenu *popup, const Context *context )
{
	if (context->hasType( "file" ))
	{
		const FileContext *fcontext = static_cast<const FileContext*>( context );
		// Here we must hope that fcontext->fileName() returns an absolute path ;(
		d->pathUrl.setPath( fcontext->fileName() );

		popup->insertSeparator();

		kdDebug(9000) << "contextMenu() for file " << UrlFileUtilities::extractPathNameAbsolute( d->pathUrl ) << endl;

		QString fileName = d->pathUrl.fileName();

		KPopupMenu *subMenu = new KPopupMenu( popup );
		subMenu->insertTitle( i18n("Actions for %1").arg(fileName) );

		d->actionCommit->plug( subMenu );
		d->actionLog->plug( subMenu );
		d->actionUpdate->plug( subMenu );
		d->actionAdd->plug( subMenu );
		d->actionDiff->plug( subMenu );

		subMenu->insertSeparator();

		d->actionRemove->plug( subMenu );
		d->actionReplace->plug( subMenu );

		popup->insertItem( i18n("CVS"), subMenu );

		// If we are invoked from the context menu than the file is has been already specified,
		// otherwise we use the current focused editor/viewer part.
		d->invokedFromMenu = false;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotCommit()
{
	if (!d->prepareOperation())
		return;

    kdDebug(9000) << "slotCommit()" << "Dir = " << d->dirName << ", fileName = " << d->fileName << endl;

    CommitDialog dlg;
	if (dlg.exec() == QDialog::Rejected)
		return;

	QDomDocument &dom = *this->projectDom();

	QString command( "cd " );
	command += KShellProcess::quote( d->dirName );
	command += " && ";
	command += d->cvs_rsh(); // yes, it is already quoted
	command += " cvs ";
	command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
	command += " commit ";
	command += DomUtil::readEntry( dom, "/kdevcvs/commitoptions", default_commit );
	command += " -m " + KShellProcess::quote( dlg.logMessage() );
	command += " ";
	command += KShellProcess::quote( d->fileName );

//	makeFrontend()->queueCommand(dirName, command);
	d->m_widget->startCommand( d->dirName, command );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotUpdate()
{
	if (!d->prepareOperation())
		return;

    kdDebug(9000) << "slotUpdate()" << "Dir = " << d->dirName << ", fileName = " << d->fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote( d->dirName );
    command += " && " + d->cvs_rsh() + " cvs ";
    command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
    command += " update ";
    command += DomUtil::readEntry( dom, "/kdevcvs/updateoptions", default_update );
    command += " ";
    command += KShellProcess::quote( d->fileName );

//    makeFrontend()->queueCommand(dirName, command);
    d->m_widget->startCommand( d->dirName, command );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotAdd()
{
	// cvs add requires only the file to exist (FIX: we don't check for "already
	// present" condition and have faith in cvs command to report error).
	if (!d->findPaths())
	{
		kdDebug(9000) << "  ** slotCommit(): aborting since findPath() == false." << endl;
		return;
	}

    kdDebug(9000) << "slotAdd()" << "Dir = " << d->dirName << ", fileName = " << d->fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote( d->dirName );
    command += " && " + d->cvs_rsh() + " cvs ";
    command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
    command += " add ";
    command += DomUtil::readEntry( dom, "/kdevcvs/addoptions", default_add );
    command += " ";
    command += KShellProcess::quote( d->fileName );

//    makeFrontend()->queueCommand(dirName, command);
    d->m_widget->startCommand( d->dirName, command );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotRemove()
{
	if (!d->prepareOperation())
		return;

    kdDebug(9000) << "slotRemove()" << "Dir = " << d->dirName << ", fileName = " << d->fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote( d->dirName );
    command += " && " + d->cvs_rsh() + " cvs ";
    command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
    command += " remove ";
    command += DomUtil::readEntry( dom, "/kdevcvs/removeoptions", default_remove );
    command += " ";
    command += KShellProcess::quote( d->fileName );

//    makeFrontend()->queueCommand(dirName, command);
    d->m_widget->startCommand( d->dirName, command );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotReplace()
{
	if (!d->prepareOperation())
		return;

    kdDebug(9000) << "slotReplace()" << "Dir = " << d->dirName << ", fileName = " << d->fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote( d->dirName );
    command += " && " + d->cvs_rsh() + " cvs ";
    command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
    command += " update ";
    command += default_replace;
    command += " ";
    command += KShellProcess::quote( d->fileName );

//    makeFrontend()->queueCommand(dirName, command);
    d->m_widget->startCommand( d->dirName, command );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotLog()
{
	if (!d->prepareOperation())
		return;

	kdDebug(9000) << "slotLog(), request for : " << d->pathUrl.path() << endl;

	LogForm* f = new LogForm();
	f->show();
	// Form will do all the work
	QString projectDir = project()->projectDirectory();
	QString relPath = UrlFileUtilities::extractPathNameRelative( projectDir, d->pathUrl );

	f->start( projectDir, relPath );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotDiff()
{
	if ( d->proc ) {
		KMessageBox::sorry( 0, i18n("Another diff operation is pending.") );
		return;
	}

	if (!d->prepareOperation())
		return;

	kdDebug(9000) << "slotDiff()" << "Dir = " << d->dirName << ", fileName = " << d->fileName << endl;

	QStringList args;
	QString str;
	d->proc = new KProcess();
	d->stdOut = QString::null;
	d->stdErr = QString::null;

	QDomDocument &dom = *this->projectDom();

	str = DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );

	if (str.length())
	{
		QStringList list = QStringList::split(' ',str);
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
			args << *it;
		}
	}
	args << "diff";
	str = DomUtil::readEntry( dom, "/kdevcvs/diffoptions", default_diff );
	if (str.length())
	{
		QStringList list = QStringList::split(' ',str);
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
			args << *it;
		}
	}
	args << d->fileName;

	QString crsh = DomUtil::readEntry( dom, "/kdevcvs/rshoptions", default_rsh );
	if ( !crsh.isEmpty() )
		d->proc->setEnvironment( "CVS_RSH", crsh );
	d->proc->setWorkingDirectory( d->dirName );
	*d->proc << "cvs";
	*d->proc << args;

	bool ok = d->proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
	if ( !ok ) {
		KMessageBox::error( 0, i18n("Could not invoke CVS"), i18n("Error Invoking Command") );
		zero_delete( d->proc );
		return;
	}

	connect( d->proc, SIGNAL(processExited(KProcess*)),this, SLOT(processExited()) );
	connect( d->proc, SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(receivedStdout(KProcess*,char*,int)) );
	connect( d->proc, SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(receivedStderr(KProcess*,char*,int)) );

	core()->running( this, true );

	d->doneOperation();
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::processExited()
{
	core()->running( this, false );

	if ( d->proc->normalExit() )
		d->diffFinished( d->stdOut, d->stdErr );

	d->stdOut = QString::null;
	d->stdErr = QString::null;

	zero_delete( d->proc );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::receivedStdout( KProcess*, char* buffer, int buflen )
{
	d->stdOut += QString::fromUtf8( buffer, buflen );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::receivedStderr( KProcess*, char* buffer, int buflen )
{
	d->stdErr += QString::fromUtf8( buffer, buflen );
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::slotStopButtonClicked( KDevPlugin* which )
{
	if ( which != 0 && which != this )
		return;
	if ( !d->proc )
		return;
	if ( !d->proc->kill() ) {
		KMessageBox::sorry( 0, i18n("Unable to kill process, you might want to kill it by hand.") );
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CvsPart::projectConfigWidget( KDialogBase *dlg )
{
	QVBox *vbox = dlg->addVBoxPage( i18n("CVS") );
	CvsOptionsWidget *w = new CvsOptionsWidget( this, (QWidget *)vbox, "cvs config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

#include "cvspart.moc"
