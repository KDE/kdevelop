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

#include <kdebug.h>

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
}

void LogForm::destroy()
{
    kdDebug() << "LogForm::destroy()" << endl;
    process->kill();
}

void LogForm::start( const QString& fn )
{
    filename = fn;

    process->kill();
    contents->clear();

    QFileInfo info( filename );
    process->setWorkingDirectory( info.dirPath() );
    process->setArguments( QStringList() << "cvs" << "log" << info.fileName() );
    process->start();
    setCaption( tr("log %1").arg(filename) );
}

void LogForm::slotProcessExited()
{
    // setCaption( tr("finished").arg(filename) );
}

void LogForm::slotReadStdout()
{
    QRegExp rx_sep( "\\-+" );
    contents->setTextFormat( QTextBrowser::PlainText );

    while( process->canReadLineStdout() ){
	QString s = process->readLineStdout();
	if( rx_sep.exactMatch(s) ){
	    contents->setTextFormat( QTextBrowser::AutoText );
	    contents->append( "<hr>" );
	    contents->setTextFormat( QTextBrowser::PlainText );
	} else {
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
