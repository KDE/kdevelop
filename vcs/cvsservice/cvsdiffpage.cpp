/***************************************************************************
 *   Copyright (C) 200?-2003 by KDevelop Authors                           *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtextedit.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qdir.h>
#include <qstringlist.h>

#include <kmessagebox.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "cvsoptions.h"
#include "cvsdiffpage.h"

#include "diffwidget.h"

///////////////////////////////////////////////////////////////////////////////
// class CVSDiffPage
///////////////////////////////////////////////////////////////////////////////

CVSDiffPage::CVSDiffPage( CvsService_stub *cvsService,
    QWidget *parent, const char *name, int )
	// Leaving it anonymous let us to have multiple objects at the same time!
    : DCOPObject(), //  "CVSDiffPageDCOPIface"
	QWidget( parent, name? name : "logformdialog" ),
    m_diffText( 0 ), m_cvsService( cvsService ), m_cvsDiffJob( 0 )
{
    QLayout *thisLayout = new QVBoxLayout( this );
    // This should be replaced by the diff part
//    m_diffText = new QTextEdit( this, "difftextedit" );
//    m_diffText->setReadOnly( true );
    m_diffText = new DiffWidget( this, "difftextedit" );

    thisLayout->add( m_diffText );
}

///////////////////////////////////////////////////////////////////////////////

CVSDiffPage::~CVSDiffPage()
{
    kdDebug(9006) << "CVSDiffPage::~CVSDiffPage()" << endl;
    cancel();
    delete m_cvsDiffJob;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::startDiff( const QString &fileName, const QString &v1, const QString &v2 )
{
    kdDebug(9006) << "CVSDiffPage::startDiff()" << endl;

    if ( v1.isEmpty() || v2.isEmpty() )
    {
        KMessageBox::error( this, i18n("Error: passed revisions are empty!"), i18n( "Error During Diff") );
        return;
    }

    CvsOptions *options = CvsOptions::instance();
    DCOPRef job = m_cvsService->diff( fileName, v1, v2, options->diffOptions(), options->contextLines() );
    m_cvsDiffJob = new CvsJob_stub( job.app(), job.obj() );

    kdDebug(9006) << "Running command : " << m_cvsDiffJob->cvsCommand() << endl;
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
//    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    bool success = m_cvsDiffJob->execute();
    if (!success)
    {
        kdDebug(9006) << "Argh ... cannot start the diff job!" << endl;
    }
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::slotJobExited( bool normalExit, int /*exitStatus*/ )
{
    kdDebug(9006) << "CVSDiffPage::slotJobExited(bool, int)" << endl;

    if (normalExit)
    {
        QString diffText = m_cvsDiffJob->output().join( "\n" );
        kdDebug(9006) << "*** Received: " << diffText << endl;
//        m_diffText->setText( diffText );
        m_diffText->setDiff( diffText );
    }
    else
    {
        KMessageBox::error( this, i18n("An error occurred during diffing."), i18n( "Error During Diff"));
    }
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::slotReceivedOutput( QString someOutput )
{
    kdDebug(9006) << "CVSDiffPage::slotReceivedOutput(QString)" << endl;
    kdDebug(9006) << "OUTPUT: " << someOutput << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::slotReceivedErrors( QString someErrors )
{
    kdDebug(9006) << "CVSDiffPage::slotReceivedErrors(QString)" << endl;
    kdDebug(9006) << "ERRORS: " << someErrors << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::cancel()
{
    if (m_cvsDiffJob && m_cvsDiffJob->isRunning())
        m_cvsDiffJob->cancel();
}

#include "cvsdiffpage.moc"


