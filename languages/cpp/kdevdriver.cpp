#include <lexer.h>

#include "kdevdriver.h"
#include "cppcodecompletionconfig.h"
#include <unistd.h>
#include <blockingkprocess.h>


KDevDriver::KDevDriver( CppSupportPart* cppSupport )
: m_cppSupport( cppSupport )
{
	//setupProject();
	setup();
	
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
	clearMacros();
	clearIncludePaths();

	addMacro( Macro( "__cplusplus", "1" ) );
	
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
        
    BlockingKProcess proc;
		proc << "gcc" ;
		proc << "-print-file-name=include" ;
        QString processStdout; 
        if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
			qWarning( "Couldn't start gcc" );
			return;
		}
        processStdout = proc.stdOut();    

		QStringList ls = QStringList::split( "\n", processStdout );
		for( QStringList::const_iterator it = ls.begin(); it != ls.end(); ++it ) {
			if( !(*it).isEmpty() )
				addIncludePath( *it );
		}
		
		addIncludePath( processStdout );
		addIncludePath( "/usr/include/g++-3" );
		addIncludePath( "/usr/include/g++" );
		proc.clearArguments();
        proc.clearStdOut(); 
		proc << "gcc";
		proc << "-E";
		proc << "-dM";
		proc << "-ansi" ;
		proc << "-";
        if ( !proc.start(KProcess::NotifyOnExit, KProcess::Stdout) ) {
			qWarning( "Couldn't start gcc" );
			return;
		}
		proc.closeStdin();
        processStdout = proc.stdOut(); 
        QStringList lines = QStringList::split('\n', processStdout);
        for (QStringList::ConstIterator it = lines.constBegin(); it != lines.constEnd(); ++it) {
			QStringList lst = QStringList::split( ' ', *it );
			if ( lst.count() != 3 )
				continue;
			addMacro( Macro( lst[1], lst[2] ) );
		}
		addMacro( Macro( "__cplusplus", "1" ) );
		addMacro( Macro( "Q_SIGNALS", "signals" ) );
		addMacro( Macro( "Q_SLOTS", "slots" ) );
		addMacro( Macro( "Q_SCRIPTABLE", "" ) );
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

	CppCodeCompletionConfig* cfg = m_cppSupport->codeCompletionConfig();
	QString str = cfg->customIncludePaths();
	int pos = 0;
	while( pos < str.length() ) {
		int end = str.find( ';', pos );
		if( end == -1 ) {
			end = str.length();
		}
		
		QString s = str.mid( pos, end-pos ).stripWhiteSpace();
		if( !s.isEmpty() ) {
			if( !s.startsWith( "/" ) && m_cppSupport->project() ) {
				s = m_cppSupport->project()->projectDirectory() + "/" + s;
			}
			addIncludePath( s );
		}
		
		pos = end+1;
	}

	setResolveDependencesEnabled( cfg->preProcessAllHeaders() /*|| cfg->parseMissingHeaders()*/ );
	
}

//kate: indent-mode csands; tab-width 4; space-indent off;

