/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>
#include <qregexp.h>

#include <dcopref.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevcore.h"

#include "cvspart.h"
#include "cvsprocesswidget.h"
#include "processwidget.h"

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

// Undef
#define MYDCOPDEBUG

///////////////////////////////////////////////////////////////////////////////
// class CvsProcessWidget
///////////////////////////////////////////////////////////////////////////////

#ifdef MYDCOPDEBUG
int g_dcopExitCounter = 0;
int g_dcopOutCounter = 0;
int g_dcopErrCounter = 0;
#endif


CvsProcessWidget::CvsProcessWidget( CvsService_stub *service, CvsPart *part, QWidget *parent, const char *name )
    : QTextEdit( parent, name ), DCOPObject(), m_part( part ), m_service( service ), m_job( 0 ),
    m_goodStyle( 0 ), m_errorStyle( 0 ), m_infoStyle( 0 )
{
    setReadOnly( true );
    setTextFormat( Qt::LogText );

    m_goodStyle = new QStyleSheetItem( styleSheet(), "goodtag" );
    m_goodStyle->setColor( "darkgreen" );

    m_errorStyle = new QStyleSheetItem( styleSheet(), "errortag" );
    m_errorStyle->setColor( "red" );
    m_errorStyle->setFontWeight( QFont::Bold );

    m_infoStyle = new QStyleSheetItem( styleSheet(), "infotag" );
    m_infoStyle->setColor( "blue" );
}

///////////////////////////////////////////////////////////////////////////////

CvsProcessWidget::~CvsProcessWidget()
{
    if (m_job)
    {
        delete m_job;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CvsProcessWidget::isAlreadyWorking() const
{
    if (m_job)
        return m_job->isRunning();
    else
       return false;
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::clear()
{
    QTextEdit::clear();
    this->m_errors = QString::null;
    this->m_output = QString::null;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsProcessWidget::startJob( const DCOPRef &aJob )
{
    kdDebug() << "CvsProcessWidget::startJob(const DCOPRef &) here!" << endl;

    clear();
    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );

    // create a DCOP stub for the non-concurrent cvs job
    if (m_job)
    {
        delete m_job;
        m_job = 0;
    }
    m_job = new CvsJob_stub( aJob.app(), aJob.obj() );

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( m_job->app(), m_job->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( m_job->app(), m_job->obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    connectDCOPSignal( m_job->app(), m_job->obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

    // get command line and add it to output buffer
    QString cmdLine = m_job->cvsCommand();
    m_part->mainWindow()->statusBar()->message( cmdLine );

    kdDebug() << "Running: " << cmdLine << endl;

    // disconnect 3rd party slots from our signals
    disconnect( SIGNAL(jobFinished(bool, int)) );

    showInfo( i18n("Started job: ") + cmdLine );

#ifdef MYDCOPDEBUG
    g_dcopExitCounter = 0;
    g_dcopOutCounter = 0;
    g_dcopErrCounter = 0;
#endif

    return m_job->execute();
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::cancelJob()
{
    kdDebug() << "CvsProcessWidget::cancelJob() here!" << endl;

    if (!m_job)
        return;
    m_job->cancel();
    disconnectDCOPSignal( m_job->app(), m_job->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)" );
    disconnectDCOPSignal( m_job->app(), m_job->obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)" );
    disconnectDCOPSignal( m_job->app(), m_job->obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)" );
    delete m_job; m_job = 0;

    showInfo( i18n("*** Job canceled by user request ***") );

    m_part->mainWindow()->statusBar()->message( i18n("CVS Job canceled") );
    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotJobExited( bool normalExit, int exitStatus )
{
    kdDebug() << "CvsProcessWidget::slotJobExited(bool, int) here!" << endl;
#ifdef MYDCOPDEBUG
    g_dcopExitCounter++;
    kdDebug() << "MYDCOPDEBUG: dcopExitCounter == " << g_dcopExitCounter << endl;
#endif
    disconnectDCOPSignal( m_job->app(), m_job->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)" );
    disconnectDCOPSignal( m_job->app(), m_job->obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)" );
    disconnectDCOPSignal( m_job->app(), m_job->obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)" );

    QString exitMsg = i18n("Job finished with exitCode == %1");
    showInfo( exitMsg.arg( exitStatus) );

    m_part->core()->running( m_part, false );
    m_part->mainWindow()->statusBar()->message( i18n("Done CVS command ..."), 2000 );

    emit jobFinished( normalExit, exitStatus );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedOutput( QString someOutput )
{
    kdDebug() << "CvsProcessWidget::slotReceivedOutput(QString)  here!" << endl;
#ifdef MYDCOPDEBUG
    g_dcopOutCounter++;
    kdDebug() << "MYDCOPDEBUG: dcopOutCounter == " << g_dcopOutCounter << endl;
#endif

    m_output += someOutput;
    showOutput( someOutput );
    scrollToBottom();
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedErrors( QString someErrors )
{
    kdDebug() << "CvsProcessWidget::slotReceivedErrors(QString)  here!" << endl;
#ifdef MYDCOPDEBUG
    g_dcopErrCounter++;
    kdDebug() << "MYDCOPDEBUG: dcopErrCounter == " << g_dcopErrCounter << endl;
#endif

    m_errors += someErrors;
    showError( someErrors );
    scrollToBottom();
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::showInfo( const QString &msg )
{
    append( "<infotag>" + msg + "</infotag>" );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::showError( const QString &msg )
{
    append( "<errortag>" + msg + "</errortag>" );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::showOutput( const QString &msg )
{
    append( "<goodtag>" + msg + "</goodtag>" );
}


#include "cvsprocesswidget.moc"
