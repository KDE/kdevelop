#include <lexer.h>

#include "kdevdriver.h"
#include <unistd.h>

KDevDriver::KDevDriver( CppSupportPart* cppSupport )
: m_cppSupport( cppSupport )
{
	//setupProject();
	//setup();
	
	addMacro( Macro( "__cplusplus", "1" ) );
}

CppSupportPart* KDevDriver::cppSupport() { return m_cppSupport; }

void KDevDriver::setupProject()
{
	QMap<QString, bool> map;
	
	QStringList fileList = m_cppSupport->project() ->allFiles();
	QStringList::ConstIterator it = fileList.begin();
	while ( it != fileList.end() )
	{
		QFileInfo info( *it );
		++it;
		
		map.insert( info.dirPath( true ), true );
	}
	QMap<QString, bool>::Iterator mapit = map.begin();
	while ( mapit != map.end() )
	{
		addIncludePath( mapit.key() );
		++mapit;
	}
	
}

void KDevDriver::setupLexer( Lexer* lexer )
{
	Driver::setupLexer( lexer );
	lexer->setReportMessages( true );
	lexer->setReportWarnings( true );
}

//! setup the preprocessor
//! code provided by Reginald Stadlbauer <reggie@trolltech.com>
void KDevDriver::setup()
{
	QString kdedir = getenv( "KDEDIR" );
	if( !kdedir.isNull() )
	addIncludePath( kdedir + "/include" );

	QString qtdir = getenv( "QTDIR" );
	if( !qtdir.isNull() )
	addIncludePath( qtdir + "/include" );

	QString qmakespec = getenv( "QMAKESPEC" );
	if ( qmakespec.isNull() )
	qmakespec = "linux-g++";
	// #### implement other mkspecs and find a better way to find the
	// #### proper mkspec (althoigh this will be no fun :-)

	addIncludePath( qtdir + "/mkspecs/" + qmakespec );
	if ( qmakespec == "linux-g++" ) 
	{
		addIncludePath( "/include" );
		addIncludePath( "/usr/include" );
		addIncludePath( "/usr/local/include" );
#if KDE_VERSION <= 305
		return; /// \FIXME Roberto, please review! ;-)
	// If the QProcess from below is executed,
	// it somehow breaks the gcc call in parts/outputviews/makewidget.cpp. :-(
	// It then has the effect that KProcess will never exit, at least on KDE-3.0
#endif // KDE_VERSION
		Q3Process proc;
		proc.addArgument( "gcc" );
		proc.addArgument( "-print-file-name=include" );
		if ( !proc.start() ) {
			qWarning( "Couldn't start gcc" );
			return;
		}
		while ( proc.isRunning() )
			usleep( 1 );
		
		addIncludePath( proc.readStdout() );
		addIncludePath( "/usr/include/g++-3" );
		addIncludePath( "/usr/include/g++" );
		proc.clearArguments();
		proc.addArgument( "gcc" );
		proc.addArgument( "-E" );
		proc.addArgument( "-dM" );
		proc.addArgument( "-ansi" );
		proc.addArgument( "-" );
		if ( !proc.start() ) {
			qWarning( "Couldn't start gcc" );
			return;
		}
		while ( !proc.isRunning() )
			usleep( 1 );
		proc.closeStdin();
		while ( proc.isRunning() )
			usleep( 1 );
		while ( proc.canReadLineStdout() ) {
			QString l = proc.readLineStdout();
			QStringList lst = QStringList::split( ' ', l );
			if ( lst.count() != 3 )
				continue;
			addMacro( Macro( lst[1], lst[2] ) );
		}
		addMacro( Macro( "__cplusplus", "1" ) );
	} else if ( qmakespec == "win32-borland" ) {
		QString incl = getenv( "INCLUDE" );
		QStringList includePaths = QStringList::split( ';', incl );
		QStringList::Iterator it = includePaths.begin();
		while( it != includePaths.end() ){
			addIncludePath( *it );
			++it;
		}
	// ### I am sure there are more standard include paths on
	// ### windows. I will fix that soon
	// ### Also do the compiler specific defines on windows
	}
}

//kate: indent-mode csands; tab-width 4; space-indent off;

