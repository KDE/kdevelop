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
#include <kgenericfactory.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kaction.h>

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

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
#include "cvspart.h"
#include "cvswidget.h"
#include "cvs_commonoptions.h"

typedef KGenericFactory<CvsPart> CvsFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcvs, CvsFactory( "kdevcvs" ) );

using namespace UrlFileUtilities;

CvsPart::CvsPart( QObject *parent, const char *fileName, const QStringList & )
	: KDevVersionControl( "CVS", "cvs", parent, fileName ? fileName : "CvsPart" ),
	invokedFromMenu( true )
{
	proc = 0;

	setInstance( CvsFactory::instance() );

	setXMLFile( "kdevcvspart.rc" );

	setupActions();

	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
		this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
		this, SLOT(slotStopButtonClicked(KDevPlugin*)) );

	m_widget = new CvsWidget( this );
	mainWindow()->embedOutputView( m_widget, i18n("CVS"), i18n("cvs output") );
}

void CvsPart::setupActions()
{
/*
	KAction * action = new KAction( i18n("Import Cvs Repository..."),"wizard", 0,
		this, SLOT(slotImportCvs()), actionCollection(), "cvs_import" );
		action->setStatusText( i18n("Imports an existing CVS repository.") );
*/
	actionCommit = new KAction( i18n("Commit"), 0, this, SLOT(slotCommit()),
		actionCollection(), "cvs_commit" );
	actionDiff = new KAction( i18n("Diff"), 0, this, SLOT(slotDiff()),
		actionCollection(), "cvs_diff" );
	actionLog = new KAction( i18n("Log"), 0, this, SLOT(slotLog()),
		actionCollection(), "cvs_log" );
	actionAdd = new KAction( i18n("Add"), 0, this, SLOT(slotAdd()),
		actionCollection(), "cvs_add" );
	actionRemove = new KAction( i18n("Remove From Repository"), 0, this, SLOT(slotRemove()),
		actionCollection(), "cvs_remove" );
	actionUpdate = new KAction( i18n("Update"), 0, this, SLOT(slotUpdate()),
		actionCollection(), "cvs_update" );
	actionReplace = new KAction( i18n("Replace with Copy From Repository"), 0, this, SLOT(slotReplace()),
		actionCollection(), "cvs_replace" );
}

CvsPart::~CvsPart()
{
	if (m_widget)
	{
		mainWindow()->removeView( m_widget ); // Inform toplevel, that the output view is gone
	}
	delete m_widget;
}

void CvsPart::slotImportCvs()
{
	// TODO
}

QWidget* CvsPart::newProjectWidget( QWidget *parent )
{
    form = new CvsForm( parent );
    return form;
}

void CvsPart::createNewProject( const QString& dirName )
{
	kdDebug( 9000 ) << "====> CvsPart::createNewProject( const QString& )" << endl;

	if (!form)
		return;
	QString init("");

	if (form->init_check->isChecked())
	{
    	init = "cvs -d " + KShellProcess::quote(form->root_edit->text()) + " init && ";
	}
    QString command = init + "cd " + KShellProcess::quote(dirName) +
		" && cvs -d " + KShellProcess::quote(form->root_edit->text()) +
		" import -m " + KShellProcess::quote(form->message_edit->text()) + " " +
		KShellProcess::quote(form->repository_edit->text()) + " " +
		KShellProcess::quote(form->vendor_edit->text()) + " " +
		KShellProcess::quote(form->release_edit->text()) + " && sh " +
		locate("data","kdevcvs/buildcvs.sh") + " . " +
		KShellProcess::quote(form->repository_edit->text()) + " " +
		KShellProcess::quote(form->root_edit->text());

	kdDebug( 9000 ) << "  ** Will run the following command: " << endl << command << endl;
	kdDebug( 9000 ) << "  ** on directory: " << dirName << endl;

	makeFrontend()->queueCommand(dirName,command);
}

void CvsPart::contextMenu( QPopupMenu *popup, const Context *context )
{
	if (context->hasType( "file" ))
	{
		const FileContext *fcontext = static_cast<const FileContext*>( context );
		// Here we must hope that fcontext->fileName() returns an absolute path ;(
		pathUrl.setPath( fcontext->fileName() );

		popup->insertSeparator();

		kdDebug(9000) << "contextMenu() for file " << UrlFileUtilities::extractPathNameAbsolute( pathUrl ) << endl;

		QString fileName = pathUrl.fileName();

		KPopupMenu *subMenu = new KPopupMenu( popup );
		subMenu->insertTitle( i18n("Actions for %1").arg(fileName) );

		actionCommit->plug( subMenu );
		actionLog->plug( subMenu );
		actionUpdate->plug( subMenu );
		actionAdd->plug( subMenu );
		actionDiff->plug( subMenu );

		subMenu->insertSeparator();

		actionRemove->plug( subMenu );
		actionReplace->plug( subMenu );

		popup->insertItem( i18n("CVS"), subMenu );

		// If we are invoked from the context menu than the file is has been already specified,
		// otherwise we use the current focused editor/viewer part.
		invokedFromMenu = false;
	}
}

QString CvsPart::cvs_rsh() const
{
	QDomDocument &dom = *this->projectDom();
	QString env = DomUtil::readEntry( dom, "/kdevcvs/rshoptions", default_rsh );
	if ( !env.isEmpty() )
		return QString( "CVS_RSH=" ) + KShellProcess::quote( env );
	return QString::null;
}

void CvsPart::slotCommit()
{
	if (!prepareOperation())
		return;

    kdDebug(9000) << "slotCommit()" << "Dir = " << dirName << ", fileName = " << fileName << endl;

    CommitDialog d;
	if (d.exec() == QDialog::Rejected)
		return;

	QDomDocument &dom = *this->projectDom();

	QString command( "cd " );
	command += KShellProcess::quote( dirName );
	command += " && ";
	command += cvs_rsh(); // yes, it is already quoted
	command += " cvs ";
	command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
	command += " commit ";
	command += DomUtil::readEntry( dom, "/kdevcvs/commitoptions", default_commit );
	command += " -m " + KShellProcess::quote( d.logMessage() );
	command += " ";
	command += KShellProcess::quote( fileName );

//	makeFrontend()->queueCommand(dirName, command);
	m_widget->startCommand( dirName, command );

	doneOperation();
}


void CvsPart::slotUpdate()
{
	if (!prepareOperation())
		return;

    kdDebug(9000) << "slotUpdate()" << "Dir = " << dirName << ", fileName = " << fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dirName);
    command += " && " + cvs_rsh() + " cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " update ";
    command += DomUtil::readEntry(dom,"/kdevcvs/updateoptions",default_update);
    command += " ";
    command += KShellProcess::quote(fileName);

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand(dirName,command);

	doneOperation();
}


void CvsPart::slotAdd()
{
	// cvs add requires only the file to exist (FIX: we don't check for "already
	// present" condition and have faith in cvs command to report error).
	if (!findPaths())
	{
		kdDebug(9000) << "  ** slotCommit(): aborting since findPath() == false." << endl;
		return;
	}

    kdDebug(9000) << "slotAdd()" << "Dir = " << dirName << ", fileName = " << fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dirName);
    command += " && " + cvs_rsh() + " cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " add ";
    command += DomUtil::readEntry(dom,"/kdevcvs/addoptions",default_add);
    command += " ";
    command += KShellProcess::quote(fileName);

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand(dirName,command);

	doneOperation();
}


void CvsPart::slotRemove()
{
	if (!prepareOperation())
		return;

    kdDebug(9000) << "slotRemove()" << "Dir = " << dirName << ", fileName = " << fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote( dirName );
    command += " && " + cvs_rsh() + " cvs ";
    command += DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
    command += " remove ";
    command += DomUtil::readEntry( dom, "/kdevcvs/removeoptions", default_remove );
    command += " ";
    command += KShellProcess::quote( fileName );

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( dirName, command );

	doneOperation();
}


void CvsPart::slotReplace()
{
	if (!prepareOperation())
		return;

    kdDebug(9000) << "slotReplace()" << "Dir = " << dirName << ", fileName = " << fileName << endl;

    QDomDocument &dom = *this->projectDom();

    QString command("cd ");
    command += KShellProcess::quote(dirName);
    command += " && " + cvs_rsh() + " cvs ";
    command += DomUtil::readEntry(dom,"/kdevcvs/cvsoptions",default_cvs);
    command += " update ";
    command += default_replace;
    command += " ";
    command += KShellProcess::quote(fileName);

//    makeFrontend()->queueCommand(dirName, command);
    m_widget->startCommand( dirName, command );

	doneOperation();
}

void CvsPart::slotLog()
{
	if (!prepareOperation())
		return;

	kdDebug(9000) << "slotLog(), request for : " << pathUrl.path() << endl;

	LogForm* f = new LogForm();
	f->show();
	// Form will do all the work
	QString projectDir = project()->projectDirectory();
	QString relPath = UrlFileUtilities::extractPathNameRelative( projectDir, pathUrl );

	f->start( projectDir, relPath );

	doneOperation();
}

void CvsPart::slotDiff()
{
	if ( proc ) {
		KMessageBox::sorry( 0, i18n("Another diff operation is pending.") );
		return;
	}

	if (!prepareOperation())
		return;

	kdDebug(9000) << "slotDiff()" << "Dir = " << dirName << ", fileName = " << fileName << endl;

	QStringList args;
	QString str;
	proc = new KProcess();
	stdOut = QString::null;
	stdErr = QString::null;

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
	args << fileName;

	QString crsh = DomUtil::readEntry(dom,"/kdevcvs/rshoptions",default_rsh);
	if ( !crsh.isEmpty() )
		proc->setEnvironment( "CVS_RSH", crsh );
	proc->setWorkingDirectory( dirName );
	*proc << "cvs";
	*proc << args;

	bool ok = proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
	if ( !ok ) {
		KMessageBox::error( 0, 
			i18n("Could not invoke CVS"),
			i18n("Error Invoking Command") );
		delete proc;
		proc = 0;
		return;
	}

	connect( proc, SIGNAL(processExited(KProcess*)),
		this, SLOT(processExited()) );
	connect( proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
		this, SLOT(receivedStdout(KProcess*,char*,int)) );
	connect( proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
		this, SLOT(receivedStderr(KProcess*,char*,int)) );

	core()->running( this, true );

	doneOperation();
}

void CvsPart::processExited()
{
	core()->running( this, false );

	if ( proc->normalExit() )
		diffFinished( stdOut, stdErr );

	stdOut = QString::null;
	stdErr = QString::null;

	delete proc;
	proc = 0;
}

void CvsPart::receivedStdout( KProcess*, char* buffer, int buflen )
{
	stdOut += QString::fromUtf8( buffer, buflen );
}

void CvsPart::receivedStderr( KProcess*, char* buffer, int buflen )
{
	stdErr += QString::fromUtf8( buffer, buflen );
}

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

void CvsPart::diffFinished( const QString& diff, const QString& err )
{
    if ( diff.isNull() && err.isNull() )
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

    Q_ASSERT( diffFrontend() );
    diffFrontend()->showDiff( diff );
}

void CvsPart::projectConfigWidget( KDialogBase *dlg )
{
	QVBox *vbox = dlg->addVBoxPage( i18n("CVS") );
	CvsOptionsWidget *w = new CvsOptionsWidget( this, (QWidget *)vbox, "cvs config widget" );
	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

#include <kparts/part.h>
#include <kdevpartcontroller.h>

bool CvsPart::retrieveUrlFocusedDocument()
{
	kdDebug(9000) << "CvsPart::retrieveUrlFocusedDocument() here!" << pathUrl.path() << endl;
	KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
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

bool CvsPart::findPaths()
{
	if (!project())
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

	kdDebug(9000) << "CvsPart::findPaths(): Project directory is " << project()->projectDirectory() << endl;
	kdDebug(9000) << "CvsPart::findPaths(): Path is " << pathUrl.path() << endl;

	// Ok, this an *ugly* hack: if the operation is requested for the project directory
	// then the path must be "customized" because of the behaviour of KURL ...
	if ( pathUrl.path() != project()->projectDirectory() )
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

bool CvsPart::prepareOperation()
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


void CvsPart::doneOperation()
{
	invokedFromMenu = true;
}

bool CvsPart::isRegisteredInRepository()
{
	kdDebug(9000) << "===> CvsPart::isRegisteredInRepository() here! " << endl;

	if ( pathUrl.path() == project()->projectDirectory() )
	{
		kdDebug(9000) << "===> Operation requested for projectDir(): true. " << endl;
		return true;
	}

	QString dirName = pathUrl.directory();
	QString entriesFilePath = dirName + "/CVS/Entries";
	// Lines in CVS/Entries are like:
	//   /.cvsignore/1.3/Fri Aug 30 21:21:04 2002//
	//   /Makefile.am/1.9/Sun Apr  6 16:52:14 2003//
	// ...
	// That is: <slash><filename><slash>...
	// So we just properly "escape" the filename so that "Makefile" does not
	// match with "Makefile.am" ;-)
	QString whatToSearch = "/" + pathUrl.fileName() + "/";

	kdDebug(9000) << "===> pathUrl.path()      = " << pathUrl.path() << endl;
	kdDebug(9000) << "===> dirName             = " << dirName << endl;
	kdDebug(9000) << "===> entriesFilePath = " << entriesFilePath << endl;
	kdDebug(9000) << "===> whatToSearch    = " << whatToSearch << endl;

	bool found = false;
	QFile f( entriesFilePath );
	if (f.open( IO_ReadOnly ))
	{
		QTextStream t( &f );
		QString s;
		while (!t.eof() && !found)
		{
			s = t.readLine();
			kdDebug(9000) << "===> Analyzing line: " << s << endl;
			if (s.find( whatToSearch ) > -1)
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




#include "cvspart.moc"
