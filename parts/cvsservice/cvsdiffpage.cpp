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

///////////////////////////////////////////////////////////////////////////////
// class CVSDiffPage
///////////////////////////////////////////////////////////////////////////////

CVSDiffPage::CVSDiffPage( CvsService_stub *cvsService,
    QWidget *parent, const char *name, int )
    : QWidget( parent, name? name : "logformdialog" ),
    m_diffText( 0 ), m_cvsService( cvsService ), m_cvsDiffJob( 0 )
{
    QLayout *thisLayout = new QVBoxLayout( this );
    // This should be replaced by the diff part
    m_diffText = new QTextEdit( this, "difftextedit" );
    m_diffText->setReadOnly( true );

    thisLayout->add( m_diffText );
}

///////////////////////////////////////////////////////////////////////////////

CVSDiffPage::~CVSDiffPage()
{
    kdDebug() << "CVSDiffPage::~CVSDiffPage()" << endl;
    cancel();
    delete m_cvsDiffJob;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::startDiff( const QString &fileName, const QString &v1, const QString &v2 )
{
    kdDebug() << "CVSDiffPage::startDiff()" << endl;

    if ( v1.isEmpty() || v2.isEmpty() )
    {
        KMessageBox::error( this, i18n("Error: passed revisions are empty!"), i18n( "Error diffing!!") );
        return;
    }

    CvsOptions *options = CvsOptions::instance();
    DCOPRef job = m_cvsService->diff( fileName, v1, v2, options->diffOptions(), options->contextLines() );
    m_cvsDiffJob = new CvsJob_stub( job.app(), job.obj() );
    if (job.isNull())
    {
        kdDebug() << "Null job???" << endl;
        return;
    }

    QString cmd;
    DCOPReply reply = job.call( "cvsCommand()" );
    if (!reply.isValid())
    {
        kdDebug() << "Reply not valid" << endl;
        return;
    }
    reply.get<QString>( cmd );

    kdDebug() << "Running command : " << cmd << endl;
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotDiffJobExited(bool, int)", true );
//    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    bool success = m_cvsDiffJob->execute();
    if (!success)
    {
        kdDebug() << "Argh ... cannot start the diff job!" << endl;
    }
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::slotDiffJobExited( bool normalExit, int /*exitStatus*/ )
{
    kdDebug() << "CVSDiffPage::slotDiffJobExited(bool, int)" << endl;

    if (normalExit)
    {
        QString diffText = m_cvsDiffJob->output().join( "\n" );
        kdDebug() << "*** Received: " << diffText << endl;
        m_diffText->setText( diffText );
    }
    else
    {
        KMessageBox::error( this, i18n("Error diffing"), i18n( "Error diffing!!") );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::slotReceivedOutput( QString someOutput )
{
    kdDebug() << "CVSDiffPage::slotReceivedOutput(QString)" << endl;

    kdDebug() << "OUTPUT: " << someOutput << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::slotReceivedErrors( QString someErrors )
{
    kdDebug() << "ERRORS: " << someErrors << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDiffPage::cancel()
{
    if (m_cvsDiffJob && m_cvsDiffJob->isRunning())
        m_cvsDiffJob->cancel();
}

#include "cvsdiffpage.moc"


