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

#include <qstringlist.h>
#include <qprocess.h>
#include <qtextbrowser.h>
#include <qregexp.h>
#include <qdir.h>

#include <kdebug.h>

#include "logform.h"

LogForm::LogForm( QWidget *parent, const char *name, int flags )
    : LogFormBase( parent, name, flags )
{
    kdDebug() << "LogForm::LogForm()" << endl;

    setWFlags( getWFlags() | WDestructiveClose );

    process = new QProcess( this );
    process->setCommunication( QProcess::Stdout | QProcess::Stderr );

    connect( process, SIGNAL(readyReadStdout()), this, SLOT(slotReadStdout()) );
    connect( process, SIGNAL(readyReadStderr()), this, SLOT(slotReadStderr()) );
    connect( process, SIGNAL(processExited()), this, SLOT(slotProcessExited()) );
    connect( contents, SIGNAL(linkClicked( const QString& )), this, SLOT(slotLinkClicked( const QString& )) );
}


LogForm::~LogForm()
{
    kdDebug() << "LogForm::~LogForm()" << endl;
    process->kill();
    delete process;
}

void LogForm::start( const QString &workDir, const QStringList &pathsList )
{
    kdDebug() << "LogForm::start() here! workDir = " << workDir << ", pathsList = " << pathsList.join( " " ) << endl;

    // "cvs log" needs to be done on relative-path basis
    pathName = pathsList.join( " " );

//    process->kill();
//    contents->clear();

    process->setWorkingDirectory( workDir );

    QStringList args;
    args << "cvs" << "log";
    args += pathsList;

    process->setArguments( args );

    kdDebug() << "Running cvs log with args: " + process->arguments().join( " " )  << endl;
    kdDebug() << "Running with working dir : " + process->workingDirectory().path()  << endl;

    process->start();

    if (!process->isRunning())
    {
        kdDebug() << " **** Process has exited with status : "<< process->exitStatus() << endl;
    }

    setCaption( tr("log %1").arg( pathName ) );
}

void LogForm::slotProcessExited()
{
    // setCaption( tr("finished").arg(pathName) );
}

void LogForm::slotReadStdout()
{
    kdDebug() << "LogForm::slotReadStdout()" << endl;

    static QRegExp rx_sep( "\\-+" );
    static QRegExp rx_sep2( "=+" );
    static QRegExp rx_date( "date: .* author: .* state: .* lines: .*" );
    // "revision" followed by one or more decimals followed by a optional dot
    static QRegExp rx_rev( "revision ((\\d+\\.?)+)" );
    contents->setTextFormat( QTextBrowser::PlainText );

    while( process->canReadLineStdout() ){
    QString s = process->readLineStdout();
    if( rx_rev.exactMatch(s) )
    {
        QString ver = rx_rev.cap( 1 );
        QString dstr = "<b>" + s + "</b> ";
        int lastVer = ver.section( '.', -1 ).toInt() - 1;
        if ( lastVer > 0 ) {
            QString lv = ver.left( ver.findRev( "." ) + 1 ) + QString::number( lastVer );
            dstr += " [<a href=\"diff:/" + pathName + "/" + lv + "_" + ver + "\">diff to " + lv + "</a>]";
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
        else if( rx_sep.exactMatch(s) || rx_sep2.exactMatch(s) )
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
}

void LogForm::slotReadStderr()
{
    kdDebug() << "LogForm::slotReadStderr()" << endl;

    QString s = "";
    while( process->canReadLineStderr() ){
        s.append( process->readLineStderr() + "\n" );
    }
    kdDebug() << "*error*" << s.latin1();
}

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

void LogForm::slotLinkClicked( const QString & link )
{
    kdDebug() << "LogForm::slotLinkClicked()" << endl;

    QString ver = link.mid( link.findRev( "/" ) + 1 );
    QString v1 = ver.section( '_', 0, 0 );
    QString v2 = ver.section( '_', 1, 1 );
    contents->clear();
    if ( v1.isEmpty() || v2.isEmpty() )
    {
        contents->append( "invalid link clicked" );
        return;
    }
    // TODO: use the diff frontend
    process->setArguments( QStringList() << "cvs" << "diff" << "-r" + v1 << "-r" + v2 << pathName );
    process->start();
    setCaption( tr("diff %1").arg(pathName) );
}

#include "logform.moc"
