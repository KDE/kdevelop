/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qregexp.h>
#include <qprocess.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <kurl.h>
#include <kdebug.h>

#include "urlfileutilities.h"

using namespace std;

void LogForm::init()
{
	kdDebug() << "LogForm::init()" << endl;

	setWFlags( getWFlags() | WDestructiveClose );
	process = new QProcess( this );
	process->setCommunication( QProcess::Stdout | QProcess::Stderr );
	connect( process, SIGNAL(readyReadStdout()), this, SLOT(slotReadStdout()) );
	connect( process, SIGNAL(readyReadStderr()), this, SLOT(slotReadStderr()) );
	connect( process, SIGNAL(processExited()), this, SLOT(slotProcessExited()) );
	connect( contents, SIGNAL(linkClicked( const QString& )), this, SLOT(linkClicked( const QString& )) );
}

void LogForm::destroy()
{
	kdDebug() << "LogForm::destroy()" << endl;
	process->kill();
}

void LogForm::start( const QString &workDir, const KURL &pathUrl )
{
	// "cvs log" needs to be done on relative-path basis
	pathName = UrlFileUtilities::extractPathNameRelative( workDir, pathUrl );

	process->kill();
	contents->clear();

	process->setWorkingDirectory( workDir );
	process->setArguments( QStringList() << "cvs" << "log" << pathName );

	process->start();
	setCaption( tr("log %1").arg( pathName ) );
}

void LogForm::slotProcessExited()
{
    // setCaption( tr("finished").arg(pathName) );
}

void LogForm::slotReadStdout()
{
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
	QString s = "";
	while( process->canReadLineStderr() ){
		s.append( process->readLineStderr() + "\n" );
	}
	kdDebug() << "*error*" << s.latin1();
}

void LogForm::setText( const QString& text )
{
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

void LogForm::linkClicked( const QString & link )
{
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
