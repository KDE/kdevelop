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

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kcursor.h>
#include <kdebug.h>

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
        const QString &moduleName, const QString &moduleComment )
        : KListViewItem( listview )
    {
        setName( moduleName );
        setComment( moduleComment );
    }

    void setName( const QString &aName ) { setText( 0, aName); }
    QString name() const { return text(0); }
    void setComment( const QString &aComment ) { setText(1, aComment); }
    QString comment() const { return text(1); }

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
    connect( m_base->chooseWorkDirButton, SIGNAL(clicked()),
        this, SLOT(slotSelectWorkDirList()) );
    connect( m_base->modulesListView, SIGNAL(executed(QListViewItem*)),
        this, SLOT(slotModuleSelected(QListViewItem*)) );
    // DEBUG-LAZINESS here ;-)
    setServerPath( ":pserver:marios@cvs.kde.org:/home/kde" );
    setWorkDir( "/home/mario/src/test/prova_checkout" );
}

///////////////////////////////////////////////////////////////////////////////

CheckoutDialog::~CheckoutDialog()
{
    delete m_job;
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::cvsRsh() const
{
    return m_base->cvsRshEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::serverPath() const
{
    return m_base->serverPathLineEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::setServerPath( const QString &aPath )
{
    m_base->serverPathLineEdit->setText( aPath );
}

///////////////////////////////////////////////////////////////////////////////

QString CheckoutDialog::workDir() const
{
    return m_base->workDirEdit->text();
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::setWorkDir( const QString &aDir )
{
    m_base->workDirEdit->setText( aDir );
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
    kdDebug(9000) << "CheckoutDialog::slotModulesListFetched() here!" << endl;

    kdDebug(9000) << "Received: " << m_job->output().join( "\n" ) << endl;

//    m_base->modulesListView->insertStringList( m_job->output() );
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotReceivedOutput( QString someOutput )
{
    kdDebug( 9000 ) << " Received output: " << someOutput << endl;

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
    kdDebug( 9000 ) << " Received errors: " << someErrors << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotSelectWorkDirList()
{
    kdDebug(9000) << "CheckoutDialog::slotSelectWorkDirList() here!" << endl;

    QString workDir = KFileDialog::getExistingDirectory(
        QString::null, this, "filedialog"
    );
    if (workDir.isEmpty())
        return;
    setWorkDir( workDir );
}

///////////////////////////////////////////////////////////////////////////////

void CheckoutDialog::slotModuleSelected( QListViewItem * )
{
    ModuleListViewItem *aModuleItem = static_cast<ModuleListViewItem*>(
        m_base->modulesListView->selectedItem()
    );
    if (!aModuleItem)
        return;

    m_base->moduleEdit->setText( aModuleItem->name() );
}


#include "checkoutdialog.moc"
