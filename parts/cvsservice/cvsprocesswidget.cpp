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
#include <kdebug.h>

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevcore.h"

#include "cvspart.h"

#include "cvsprocesswidget.h"
#include "processwidget.h"

#include <cvsjob_stub.h>

///////////////////////////////////////////////////////////////////////////////
// class CvsListBoxItem
///////////////////////////////////////////////////////////////////////////////

class CvsListBoxItem : public ProcessListBoxItem
{
public:
    CvsListBoxItem( const QString &s1 );
    QString fileName() { return filename; }
    bool containsFileName() { return !filename.isEmpty(); }
    virtual bool isCustomItem();

private:
    virtual void paint( QPainter *p );
    QString str1;
    QString filename;
};

///////////////////////////////////////////////////////////////////////////////

CvsListBoxItem::CvsListBoxItem( const QString &s1 )
        : ProcessListBoxItem( s1, Normal ) {
    str1 = s1;
    QRegExp re("[ACMPRU?] (.*)");
    if (re.exactMatch(s1)) filename = re.cap(1);
}

///////////////////////////////////////////////////////////////////////////////

bool CvsListBoxItem::isCustomItem()
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsListBoxItem::paint(QPainter *p)
{
    QFontMetrics fm = p->fontMetrics();
    int y = fm.ascent()+fm.leading()/2;
    int x = 3;

    p->setPen(Qt::darkGreen);
    p->drawText(x, y, str1);
}

///////////////////////////////////////////////////////////////////////////////
// class CvsProcessWidget
///////////////////////////////////////////////////////////////////////////////

CvsProcessWidget::CvsProcessWidget( QCString appId, CvsPart *part, QWidget *parent, const char *name )
    : KListBox( parent, name ), DCOPObject(), m_part( part ), m_job( 0 )
{
    // create a DCOP stub for the non-concurrent cvs job
    m_job = new CvsJob_stub( appId, "NonConcurrentJob" );

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( m_job->app(), m_job->obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( m_job->app(), m_job->obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    connectDCOPSignal( m_job->app(), m_job->obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

    connect( this, SIGNAL(highlighted(int)), this, SLOT(slotLineHighlighted(int)) );
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

    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );

    // get command line and add it to output buffer
    QString cmdLine = m_job->cvsCommand();
    kdDebug() << "Running: " << cmdLine << endl;

    // disconnect 3rd party slots from our signals
    disconnect( SIGNAL(jobFinished(bool, int)) );

    return m_job->execute();
}

bool CvsProcessWidget::startJob( const QCString appId, const QCString objId )
{
    kdDebug() << "CvsProcessWidget::startJob(const QCString, const QCString) here!" << endl;

    m_part->mainWindow()->raiseView( this );
    m_part->core()->running( m_part, true );

    CvsJob_stub job( appId, objId );
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

    // get command line and add it to output buffer
    QString cmdLine = job.cvsCommand();
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

void CvsProcessWidget::slotLineHighlighted( int line )
{
    ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item(line));
    if (i->isCustomItem())
    {
        CvsListBoxItem *ci = static_cast<CvsListBoxItem*>( i );
        if (ci->containsFileName())
        {
//            m_part->partController()->editDocument( dir + "/" + ci->fileName() );
            m_part->mainWindow()->lowerView( this );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotJobExited( bool normalExit, int exitStatus )
{
    m_part->core()->running( m_part, false );

    emit jobFinished( normalExit, exitStatus );
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedOutput( QString someOutput )
{
    insertItem( new CvsListBoxItem( someOutput ) );

    m_output += someOutput;

    kdDebug() << "OUTPUT: " << someOutput << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CvsProcessWidget::slotReceivedErrors( QString someErrors )
{
    insertItem( new CvsListBoxItem( someErrors ) );

    m_errors += someErrors;

    kdDebug() << "ERRORS: " << someErrors << endl;
}

#include "cvsprocesswidget.moc"
