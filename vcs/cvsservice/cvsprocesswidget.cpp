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
//#define MYDCOPDEBUG

///////////////////////////////////////////////////////////////////////////////
// class CvsProcessWidget
///////////////////////////////////////////////////////////////////////////////

#ifdef MYDCOPDEBUG
int g_dcopExitCounter = 0;
int g_dcopOutCounter = 0;
int g_dcopErrCounter = 0;
#endif


CvsProcessWidget::CvsProcessWidget( CvsService_stub *service, CvsServicePart *part, QWidget *parent, const char *name )
    : DCOPObject( "CvsProcessWidgetDCOPIface" ),
    QTextEdit( parent, name ),
	m_part( part ), m_service( service ), m_job( 0 )
{
    setReadOnly( true );
    setTextFormat( Qt::LogText );

    QStyleSheetItem *style = 0;
    style = new QStyleSheetItem( styleSheet(), "goodtag" );
    style->setColor( "black" );

    style = new QStyleSheetItem( styleSheet(), "errortag" );
    style->setColor( "red" );
    style->setFontWeight( QFont::Bold );

    style = new QStyleSheetItem( styleSheet(), "infotag" );
    style->setColor( "blue" );

    style = new QStyleSheetItem( styleSheet(), "cvs_conflict" );
    style->setColor( "red" );

    style = new QStyleSheetItem( styleSheet(), "cvs_added" );
    style->setColor( "green" );

    style = new QStyleSheetItem( styleSheet(), "cvs_removed" );
    style->setColor( "yellow" );

    style = new QStyleSheetItem( styleSheet(), "cvs_updated" );
    style->setColor( "lightblue" );

    style = new QStyleSheetItem( styleSheet(), "cvs_modified" );
    style->setColor( "darkgreen" );

    style = new QStyleSheetItem( styleSheet(), "cvs_unknown" );
    style->setColor( "gray" );
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
    kdDebug(9006) << "CvsProcessWidget::startJob(const DCOPRef &) here!" << endl;

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

    kdDebug(9006) << "Running: " << cmdLine << endl;

    // disconnect 3rd party slots from our signals
    disconnect( SIGNAL(jobFinished(bool, int)) );

    showInfo( i18n("Started job: %1").arg( cmdLine ) );

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
    kdDebug(9006) << "CvsProcessWidget::cancelJob() here!" << endl;

    if (!m_job || !m_job->isRunning())
        return;
    m_job->cancel();
    delete m_job; m_job = 0;

    showInfo( i18n("*** Job canceled by user request ***") );

    m_part->core()->running( m_part, false );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotJobExited( bool normalExit, int exitStatus )
{
    kdDebug(9006) << "CvsProcessWidget::slotJobExited(bool, int) here!" << endl;
#ifdef MYDCOPDEBUG
    g_dcopExitCounter++;
    kdDebug(9006) << "MYDCOPDEBUG: dcopExitCounter == " << g_dcopExitCounter << endl;
#endif
    if (m_job)
    {
        disconnectDCOPSignal( m_job->app(), m_job->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)" );
        disconnectDCOPSignal( m_job->app(), m_job->obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)" );
        disconnectDCOPSignal( m_job->app(), m_job->obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)" );
        delete m_job;
        m_job = 0;
    }
    QString exitMsg = i18n("Job finished with exitCode == %1");
    showInfo( exitMsg.arg( exitStatus) );

    m_part->core()->running( m_part, false );
    m_part->mainWindow()->statusBar()->message( i18n("Done CVS command ..."), 2000 );

    emit jobFinished( normalExit, exitStatus );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedOutput( QString someOutput )
{
    kdDebug(9006) << "CvsProcessWidget::slotReceivedOutput(QString)  here!" << endl;
#ifdef MYDCOPDEBUG
    g_dcopOutCounter++;
    kdDebug(9006) << "MYDCOPDEBUG: dcopOutCounter == " << g_dcopOutCounter << endl;
#endif

    QStringList strings = m_outputBuffer.process( someOutput );
    if (strings.count() > 0)
    {
        m_output += strings;
        showOutput( strings );
        scrollToBottom();
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedErrors( QString someErrors )
{
    kdDebug(9006) << "CvsProcessWidget::slotReceivedErrors(QString)  here!" << endl;
#ifdef MYDCOPDEBUG
    g_dcopErrCounter++;
    kdDebug(9006) << "MYDCOPDEBUG: dcopErrCounter == " << g_dcopErrCounter << endl;
#endif

    QStringList strings = m_errorBuffer.process( someErrors );
    if (strings.count() > 0)
    {
        m_errors += strings;
        showError( strings );
        scrollToBottom();
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::showInfo( const QStringList &msg )
{
    for (QStringList::const_iterator it = msg.begin(); it != msg.end(); ++it)
        append( "<infotag>" + (*it) + "</infotag>" );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::showError( const QStringList &msg )
{
    for (QStringList::const_iterator it = msg.begin(); it != msg.end(); ++it)
        append( "<errortag>" + (*it) + "</errortag>" );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::showOutput( const QStringList &msg )
{
    for (QStringList::const_iterator it = msg.begin(); it != msg.end(); ++it)
    {
        // @todo here we can interpret lines as [C], [M], ...
        const QString &line = (*it);

        if (line.startsWith( "C " ))
            append( "<cvs_conflict>" + line + "</cvs_conflict>" );
        else if (line.startsWith( "M " ))
            append( "<cvs_modified>" + line + "</cvs_modified>" );
        else if (line.startsWith( "A " ))
            append( "<cvs_added>" + line + "</cvs_added>" );
        else if (line.startsWith( "R " ))
            append( "<cvs_removed>" + line + "</cvs_removed>" );
        else if (line.startsWith( "U " ))
            append( "<cvs_updated>" + line + "</cvs_updated>" );
        else if (line.startsWith( "? " ))
            append( "<cvs_unknown>" + line + "</cvs_unknown>" );
        else // default
            append( "<goodtag>" + (*it) + "</goodtag>" );
    }
}

#include "cvsprocesswidget.moc"
