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

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qtextstream.h>

#include <klistview.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kapplication.h>

#include <dcopref.h>
#include <cvsjob_stub.h>
#include <repository_stub.h>
#include <cvsservice_stub.h>

#include "checkoutdialogbase.h"

#include "checkoutdialog.h"

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

const QString SSS( ":" );  // Server String Separator :)

///////////////////////////////////////////////////////////////////////////////
// class ModuleListViewItem
///////////////////////////////////////////////////////////////////////////////

class ModuleListViewItem : public KListViewItem
{
public:
    ModuleListViewItem( KListView *listview,
        const QString &moduleAlias, const QString &moduleRealPath )
        : KListViewItem( listview )
    {
        setAlias( moduleAlias );
        setRealPath( moduleRealPath );
    }

    void setAlias( const QString &aName ) { setText( 0, aName); }
    QString alias() const { return text(0); }
    void setRealPath( const QString &aRealPath ) { setText(1, aRealPath); }
    QString realPath() const { return text(1); }

//    virtual QString text() const { return name(); }
};

///////////////////////////////////////////////////////////////////////////////
// class CheckoutDialog
///////////////////////////////////////////////////////////////////////////////

CheckoutDialog::CheckoutDialog( CvsService_stub *cvsService,
    QWidget *parent, const char *name, WFlags ) :
    DCOPObject( "CheckoutDialogDCOPIface" ),
    KDialogBase( parent, name? name : "checkoutdialog", true, i18n("CVS Checkout"),
        Ok | Cancel, Ok, true ),
    m_service( cvsService ), m_job( 0 )
{
    m_base = new CheckoutDialogBase( this, "checkoutdialogbase" );
    setMainWidget( m_base );

    connect( m_base->fetchModulesButton, SIGNAL(clicked()),
        this, SLOT(slotFetchModulesList()) );
    connect( m_base->modulesListView, SIGNAL(executed(QListViewItem*)),
        this, SLOT(slotModuleSelected(QListViewItem*)) );

    // Avoid displaying 'file:/' when displaying the file
    m_base->workURLRequester->setShowLocalProtocol( false );
    m_base->workURLRequester->setMode( KFile::Directory );

	// Grab the entries from $HOME/.cvspass
	fetchUserCvsRepositories();
	// And suggest to use the default projects dir set in KDevelop's preferences
    KConfig *config = kapp->config();
	config->setGroup("General Options");
    QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");
	setWorkDir( defaultProjectsDir );
}

///////////////////////////////////////////////////////////////////////////////

CheckoutDialog::~CheckoutDialog()
{
    delete m_job;
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::serverPath() const
{
	return m_base->serverPaths->currentText();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::fillServerPaths( const QStringList &serverPaths )
{
    m_base->serverPaths->insertStringList( serverPaths );
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::workDir() const
{
    return m_base->workURLRequester->url();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::setWorkDir( const QString &aDir )
{
    m_base->workURLRequester->setURL( aDir );
}

///////////////////////////////////////////////////////////////////////////////

bool CheckoutDialog::pruneDirs() const
{
    return m_base->pruneDirsCheck->isChecked();
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::tag() const
{
    return m_base->tagEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::module() const
{
    return m_base->moduleEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotFetchModulesList()
{
    setCursor( KCursor::waitCursor() );

    if (serverPath().isEmpty() || workDir().isEmpty())
        return;

    DCOPRef job = m_service->moduleList( serverPath() );
    if (!m_service->ok())
        return;

    m_job = new CvsJob_stub( job.app(), job.obj() );
    // We only need to know when it finishes and then will grab the output
    // by using m_job->output() :-)
    connectDCOPSignal( job.app(), job.obj(), "jobFinished(bool,int)", "slotJobExited(bool,int)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "receivedOutput(QString)", true );

    kdDebug() << "Running: " << m_job->cvsCommand() << endl;
    m_job->execute();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotJobExited( bool /*normalExit*/, int /*exitStatus*/ )
{
    kdDebug(9006) << "CheckoutDialog::slotModulesListFetched() here!" << endl;

    kdDebug(9006) << "Received: " << m_job->output().join( "\n" ) << endl;

//    m_base->modulesListView->insertStringList( m_job->output() );
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotReceivedOutput( QString someOutput )
{
    kdDebug( 9006 ) << " Received output: " << someOutput << endl;

    setCursor( KCursor::arrowCursor() );

    // Fill the modules KListView if the list obtained is not empty
    // QStringList modules = m_job->output();
    QStringList modules = QStringList::split( "\n", someOutput );
    if (modules.count() <= 0)
        return;

    QStringList::iterator it = modules.begin();
    for ( ; it != modules.end(); ++it )
    {
        QStringList l = QStringList::split( " ", (*it) );
        // Now, l[0] is the module name, l[1] is ... another string ;-)
        new ModuleListViewItem( m_base->modulesListView, l[0], l[1] );
    }
}

void CheckoutDialog::slotReceivedErrors( QString someErrors )
{
	kdDebug( 9006 ) << " Received errors: " << someErrors << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotModuleSelected( QListViewItem * )
{
	ModuleListViewItem *aModuleItem = static_cast<ModuleListViewItem*>(
		m_base->modulesListView->selectedItem()
	);
	if (!aModuleItem)
		return;

	m_base->moduleEdit->setText( aModuleItem->alias() );
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::fetchUserCvsRepositories()
{
	QStringList repositories;

	QFile cvspass( QDir::homeDirPath() + QDir::separator() + ".cvspass" );
	if (!cvspass.open( IO_ReadOnly ))
		return;
	QByteArray data = cvspass.readAll();
	cvspass.close();

	QTextIStream istream( data );
	// Entries are like:
	// /1 :pserver:marios@cvs.kde.org:2401/home/kde Ahz:UIK?=d ?
	// /1 :pserver:mario@xamel:2401/home/cvsroot aJT_d'K?=d ?
	while (!istream.eof()) {
		QString line = istream.readLine();
		QStringList lineElements = QStringList::split( " ", line );
		if (lineElements.count() > 1) {
			repositories << lineElements[ 1 ];
		}
	}

	fillServerPaths( repositories );
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotOk()
{
	QString errorMessage = QString::null;

	if (!(workDir().length() > 0) && QFile::exists( workDir() ))
		errorMessage = i18n( "Please, choose a valid working directory" );
	else if (!(serverPath().length() > 0))
		errorMessage = i18n( "Please, choose a CVS server." );
	else if (!(module().length() > 0))
		errorMessage = i18n( "Please, fill the CVS module field." );

	if (errorMessage.isNull())
		KDialogBase::slotOk();
	else
		KMessageBox::error( this, errorMessage );
}


#include "checkoutdialog.moc"
