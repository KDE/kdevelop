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

#include <q3textbrowser.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qdir.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kmessagebox.h>
#include <kcursor.h>
#include <klocale.h>
#include <kdebug.h>
#include <dcopref.h>

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "cvsoptions.h"
#include "cvslogpage.h"
#include "cvsdiffpage.h"

///////////////////////////////////////////////////////////////////////////////
// class CVSLogPage
///////////////////////////////////////////////////////////////////////////////

CVSLogPage::CVSLogPage( CvsService_stub *cvsService, QWidget *parent, const char *name, int )
    : DCOPObject( "CvsLogPageDCOPIface" ),
	QWidget( parent, name? name : "logformpage" ),
    m_cvsService( cvsService ), m_cvsLogJob( 0 )
{
    QLayout *thisLayout = new QVBoxLayout( this );

    m_textBrowser = new Q3TextBrowser( this, "logbrowser" );
    thisLayout->add( m_textBrowser );

    /// \FIXME a better way?
    m_textBrowser->setMinimumWidth(fontMetrics().width('X')*50);
    m_textBrowser->setMinimumHeight(fontMetrics().width('X')*43);

    connect( m_textBrowser, SIGNAL(linkClicked( const QString& )), this, SLOT(slotLinkClicked( const QString& )) );
}

///////////////////////////////////////////////////////////////////////////////

CVSLogPage::~CVSLogPage()
{
    kdDebug(9006) << "CVSLogPage::~CVSLogPage()" << endl;
    cancel();
    delete m_cvsLogJob;
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogPage::startLog( const QString &workDir, const QString &pathName )
{
    kdDebug(9006) << "CVSLogPage::start() here! workDir = " << workDir <<
        ", pathName = " << pathName << endl;

//    CvsOptions *options = CvsOptions::instance();
    // "cvs log" needs to be done on relative-path basis
    m_pathName = pathName;
	m_diffStrings.clear();

    DCOPRef job = m_cvsService->log( pathName );
    m_cvsLogJob = new CvsJob_stub( job.app(), job.obj() );

    // establish connections to the signals of the cvs m_job
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    // We'll read the ouput directly from the job ...
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
//    connectDCOPSignal( job.app(), job.obj(), "receivedStderr(QString)", "slotReceivedErrors(QString)", true );

    kdDebug(9006) << "Running: " << m_cvsLogJob->cvsCommand() << endl;
    m_cvsLogJob->execute();
}

///////////////////////////////////////////////////////////////////////////////
/*
void CVSLogPage::parseLogContent( const QString& text )
{
    kdDebug(9006) << "CVSLogPage::parseLogContent()" << endl;

    m_base->contents->clear();

    QStringList l = QStringList::split( "----------------------------", text );
    QString header = l.front();
    l.pop_front();

    for( QStringList::Iterator it=l.begin(); it!=l.end(); ++it )
    {
        const QString &s = *it;
        if (s)
        {
            m_base->contents->append( s );
            m_base->contents->append( "<hr>" );
        }
    }
}
*/
///////////////////////////////////////////////////////////////////////////////

void CVSLogPage::slotJobExited( bool normalExit, int exitStatus )
{
//    m_part->core()->running( m_part, false );
    if (!normalExit)
    {
        KMessageBox::sorry( this, i18n("Log failed with exitStatus == %1").arg( exitStatus), i18n("Log Failed") );
        return;
    }

    static QRegExp rx_sep( "\\-+" );
    static QRegExp rx_sep2( "=+" );
    static QRegExp rx_date( "date: .* author: .* state: .* lines: .*" );
    // "revision" followed by one or more decimals followed by a optional dot
    static QRegExp rx_rev( "revision ((\\d+\\.?)+)" );
    m_textBrowser->setTextFormat( Qt::PlainText );

    for (size_t i=0; i<m_diffStrings.count(); ++i) {
        QString s = m_diffStrings[i];
        kdDebug(9006) << "Examining line: " << s << endl;
        if ( rx_rev.exactMatch(s) )
        {
            QString ver = rx_rev.cap( 1 );
            QString dstr = "<b>" + s + "</b> ";
            int lastVer = ver.section( '.', -1 ).toInt() - 1;
            if ( lastVer > 0 ) {
                QString lv = ver.left( ver.findRev( "." ) + 1 ) + QString::number( lastVer );
                dstr += " [<a href=\"diff:/" + m_pathName + "/" + lv + "_" + ver + "\">diff to " + lv + "</a>]";
            }
            m_textBrowser->setTextFormat( Qt::RichText );
            m_textBrowser->append( dstr );
            m_textBrowser->setTextFormat( Qt::PlainText );
        }
        else if ( rx_date.exactMatch(s) )
        {
            m_textBrowser->setTextFormat( Qt::RichText );
            m_textBrowser->append( "<i>" + s + "</i>" );
            m_textBrowser->setTextFormat( Qt::PlainText );
        }
        else if ( rx_sep.exactMatch(s) || rx_sep2.exactMatch(s) )
        {
            m_textBrowser->append( "\n" );
            m_textBrowser->setTextFormat( Qt::RichText );
            m_textBrowser->append( "<hr>" );
            m_textBrowser->setTextFormat( Qt::PlainText );
        } else
        {
            m_textBrowser->append( s );
        }
    }
    m_logTextBackup = m_textBrowser->source();

//    emit jobFinished( normalExit, exitStatus );
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogPage::slotLinkClicked( const QString &link )
{
    kdDebug(9006) << "CVSLogPage::slotLinkClicked()" << endl;

    // The text browser clears the page so we go back to our old one
    /// \FIXME in this way I lose the source
    m_textBrowser->setSource( m_logTextBackup );

    QString ver = link.mid( link.findRev( "/" ) + 1 );
    QString v1 = ver.section( '_', 0, 0 );
    QString v2 = ver.section( '_', 1, 1 );
    if ( v1.isEmpty() || v2.isEmpty() )
    {
        m_textBrowser->append( i18n( "invalid link clicked" ) );
        return;
    }

    emit diffRequested( m_pathName, v1, v2 );
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogPage::slotReceivedOutput( QString someOutput )
{
    kdDebug(9006) << "CVSLogPage::slotReceivedOutput(QString)" << endl;

    kdDebug(9006) << "OUTPUT: " << someOutput << endl;
	m_diffStrings += m_outputBuffer.process(someOutput);
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogPage::slotReceivedErrors( QString someErrors )
{
    kdDebug(9006) << "ERRORS: " << someErrors << endl;
}

///////////////////////////////////////////////////////////////////////////////

void CVSLogPage::cancel()
{
    if (m_cvsLogJob && m_cvsLogJob->isRunning())
        m_cvsLogJob->cancel();
}

#include "cvslogpage.moc"
