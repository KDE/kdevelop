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

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevcore.h"

#include "cvspart.h"

#include "cvsprocesswidget.h"
#include "processwidget.h"

#include <cvsjob_stub.h>

///////////////////////////////////////////////////////////////////////////////
// class CvsProcessWidget
///////////////////////////////////////////////////////////////////////////////

CvsProcessWidget::CvsProcessWidget( QCString appId, CvsPart *part, QWidget *parent, const char *name )
    : QTextEdit( parent, name ), DCOPObject(), m_part( part ), m_job( 0 ),
    m_goodStyle( 0 ), m_errorStyle( 0 )
{
    setReadOnly( true );
    // create a DCOP stub for the non-concurrent cvs job
    m_job = new CvsJob_stub( appId, "NonConcurrentJob" );

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( m_job->app(), m_job->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( m_job->app(), m_job->obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    connectDCOPSignal( m_job->app(), m_job->obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

//    connect( this, SIGNAL(highlighted(int)), this, SLOT(slotLineHighlighted(int)) );
    setTextFormat( Qt::LogText );
    m_goodStyle = new QStyleSheetItem( styleSheet(), "goodtag" );
    m_goodStyle->setColor( "green" );

    m_errorStyle = new QStyleSheetItem( styleSheet(), "errortag" );
    m_errorStyle->setColor( "red" );
    m_errorStyle->setFontWeight( QFont::Bold );
}

///////////////////////////////////////////////////////////////////////////////

CvsProcessWidget::~CvsProcessWidget()
{
    delete m_job;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsProcessWidget::startJob()
{
    kdDebug() << "CvsProcessWidget::startJob() here!" << endl;

    clear();

    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );

    // get command line and add it to output buffer
    QString cmdLine = m_job->cvsCommand();
    m_part->mainWindow()->statusBar()->message( cmdLine );

    kdDebug() << "Running: " << cmdLine << endl;

    // disconnect 3rd party slots from our signals
    disconnect( SIGNAL(jobFinished(bool, int)) );

    return m_job->execute();
}

bool CvsProcessWidget::startJob( const QCString appId, const QCString objId )
{
    kdDebug() << "CvsProcessWidget::startJob(const QCString, const QCString) here!" << endl;

    clear();

    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );

    CvsJob_stub job( appId, objId );
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

    // get command line and add it to output buffer
    QString cmdLine = job.cvsCommand();
    m_part->mainWindow()->statusBar()->message( cmdLine );

    kdDebug() << "Running: " << cmdLine << endl;

    // disconnect 3rd party slots from our signals
    disconnect( SIGNAL(jobFinished(bool, int)) );

    return job.execute();
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::cancelJob()
{
    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotJobExited( bool normalExit, int exitStatus )
{
    m_part->core()->running( m_part, false );
    m_part->mainWindow()->statusBar()->message( "Done CVS command ...", 2000 );

    emit jobFinished( normalExit, exitStatus );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedOutput( QString someOutput )
{
    m_output += "<goodtag>" + someOutput + "</goodtag>";
    append( "<goodtag>" + someOutput + "</goodtag>" );
    scrollToBottom();

    kdDebug() << "OUTPUT: " << someOutput << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedErrors( QString someErrors )
{
    m_errors += "<errortag>" + someErrors + "</errortag>";
    append( "<errortag>" + someErrors + "</errortag>" );
    scrollToBottom();

    kdDebug() << "ERRORS: " << someErrors << endl;
}

#include "cvsprocesswidget.moc"
