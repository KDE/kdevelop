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

#include <qtextbrowser.h>
#include <qregexp.h>
#include <qdir.h>
#include <qstringlist.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <dcopref.h>

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "cvsoptions.h"
#include "logform.h"

///////////////////////////////////////////////////////////////////////////////
// class LogForm
///////////////////////////////////////////////////////////////////////////////

LogForm::LogForm( QWidget *parent, const char *name, int flags )
    : LogFormBase( parent, name, flags ), DCOPObject(),
    m_cvsService( 0 ), m_cvsLogJob( 0 ), m_cvsDiffJob( 0 )
{
    kdDebug() << "LogForm::LogForm()" << endl;

    setWFlags( getWFlags() | WDestructiveClose );

    connect( contents, SIGNAL(linkClicked( const QString& )), this, SLOT(slotLinkClicked( const QString& )) );
}

///////////////////////////////////////////////////////////////////////////////

LogForm::~LogForm()
{
    kdDebug() << "LogForm::~LogForm()" << endl;

    delete m_cvsLogJob;
    delete m_cvsDiffJob;
}

///////////////////////////////////////////////////////////////////////////////

void LogForm::start( CvsService_stub *cvsService, const QString &workDir, const QString &pathName )
{
    kdDebug() << "LogForm::start() here! workDir = " << workDir << ", pathName = " << pathName << endl;

    m_cvsService = cvsService;

    CvsOptions *options = CvsOptions::instance();

    // "cvs log" needs to be done on relative-path basis
    m_pathName = pathName;

    DCOPRef job = cvsService->log( pathName );
    m_cvsLogJob = new CvsJob_stub( job.app(), job.obj() );
    if (options->rsh().isEmpty())
    {
        job.call( "setRSH", options->rsh() );
    }

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
//    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
//    connectDCOPSignal( job.app(), job.obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

    kdDebug() << "Running: " << m_cvsLogJob->cvsCommand() << endl;
    m_cvsLogJob->execute();

    setCaption( tr("log %1").arg( pathName ) );
}

///////////////////////////////////////////////////////////////////////////////

void LogForm::setText( const QString& text )
{
    kdDebug() << "LogForm::setText()" << endl;

    contents->clear();

    QStringList l = QStringList::split( "----------------------------", text );
    QString header = l.front();
    l.pop_front();

    for( QStringList::Iterator it=l.begin(); it!=l.end(); ++it ){
        QString s = *it;
        if( !s )
            continue;

        contents->append( s );
        contents->append( "<hr>" );
    }

}

///////////////////////////////////////////////////////////////////////////////

void LogForm::slotLinkClicked( const QString & link )
{
    kdDebug() << "LogForm::slotLinkClicked()" << endl;

    contents->clear();

    QString ver = link.mid( link.findRev( "/" ) + 1 );
    QString v1 = ver.section( '_', 0, 0 );
    QString v2 = ver.section( '_', 1, 1 );
    if ( v1.isEmpty() || v2.isEmpty() )
    {
        contents->append( "invalid link clicked" );
        return;
    }

    /// @todo use the diff frontend
    CvsOptions *options = CvsOptions::instance();

    DCOPRef job = m_cvsService->diff( m_pathName, v1, v2, options->diff(), options->contextLines() );
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

    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotDiffFinished(bool, int)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    job.call( "execute()" );

    setCaption( tr("diff %1").arg(m_pathName) );
}

///////////////////////////////////////////////////////////////////////////////

void LogForm::slotJobExited( bool normalExit, int exitStatus )
{
//    m_part->core()->running( m_part, false );
    if (!normalExit)
    {
        KMessageBox::sorry( this, i18n("Log failed with exitStatus == %1").arg( exitStatus), i18n("Log failed") );
        return;
    }

    static QRegExp rx_sep( "\\-+" );
    static QRegExp rx_sep2( "=+" );
    static QRegExp rx_date( "date: .* author: .* state: .* lines: .*" );
    // "revision" followed by one or more decimals followed by a optional dot
    static QRegExp rx_rev( "revision ((\\d+\\.?)+)" );
    contents->setTextFormat( QTextBrowser::PlainText );

    QStringList lines = m_cvsLogJob->output();
    for (size_t i=0; i<lines.count(); ++i) {
        QString s = lines[i];
        kdDebug() << "Examining line: " << s << endl;
        if ( rx_rev.exactMatch(s) )
        {
            QString ver = rx_rev.cap( 1 );
            QString dstr = "<b>" + s + "</b> ";
            int lastVer = ver.section( '.', -1 ).toInt() - 1;
            if ( lastVer > 0 ) {
                QString lv = ver.left( ver.findRev( "." ) + 1 ) + QString::number( lastVer );
                dstr += " [<a href=\"diff:/" + m_pathName + "/" + lv + "_" + ver + "\">diff to " + lv + "</a>]";
            }
            contents->setTextFormat( QTextBrowser::RichText );
            contents->append( dstr );
            contents->setTextFormat( QTextBrowser::PlainText );
        }
        else if ( rx_date.exactMatch(s) )
        {
            contents->setTextFormat( QTextBrowser::RichText );
            contents->append( "<i>" + s + "</i>" );
            contents->setTextFormat( QTextBrowser::PlainText );
        }
        else if ( rx_sep.exactMatch(s) || rx_sep2.exactMatch(s) )
        {
            contents->append( "\n" );
            contents->setTextFormat( QTextBrowser::RichText );
            contents->append( "<hr>" );
            contents->setTextFormat( QTextBrowser::PlainText );
        } else
        {
            contents->append( s );
        }
    }
//    emit jobFinished( normalExit, exitStatus );
}

///////////////////////////////////////////////////////////////////////////////

void LogForm::slotReceivedOutput( QString someOutput )
{
    kdDebug() << "LogForm::slotReceivedOutput(QString)" << endl;

    kdDebug() << "OUTPUT: " << someOutput << endl;

    m_diffText += someOutput;
}

///////////////////////////////////////////////////////////////////////////////

void LogForm::slotReceivedErrors( QString someErrors )
{
    kdDebug() << "ERRORS: " << someErrors << endl;
}

void LogForm::slotDiffFinished( bool normalExit, int exitStatus )
{
    kdDebug() << "LogForm::slotDiffFinished(bool, int)" << endl;

    if (normalExit)
    {
       contents->setText( m_diffText );
    }
    else
    {
        KMessageBox::error( this, i18n("Error diffing"), i18n( "Error diffing!!") );
    }
}

#include "logform.moc"
