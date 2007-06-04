#include "klocale.h" /* defines [function] i18n */
#include <lexer.h>

#include "kdevdriver.h"
#include "cppcodecompletionconfig.h"
#include "setuphelper.h"
#include <unistd.h>
#include "includepathresolver.h"


KDevDriver::KDevDriver( CppSupportPart* cppSupport, bool foreground )
: m_cppSupport( cppSupport ), m_includePathResolver(0), m_foreground(foreground), m_shouldParseIncludedFiles(true)
{
	//setupProject();
	setup();
	
}

KDevDriver::~KDevDriver() {
	delete m_includePathResolver;
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
	if( lexerCache() ) lexerCache()->clear(); ///Clear the lexer-cache so missing headers get a chance to be parsed
	clearMacros();
	clearIncludePaths();

	addMacro( Macro("KDEVELOP_PARSER", "3.4") );
	
	bool ok;
	QString verboseGccOutput = SetupHelper::getVerboseGccIncludePath( &ok );
	QStringList verboseGccLines = QStringList::split( '\n', verboseGccOutput );
	if( verboseGccLines.count() > 3 ) {
		///Parse the output of gcc. It includes gcc's final include-path when parsing an empty c++-file(including dirs like /usr/include/c++/4.xx/...)
		//what about order?
		for( QStringList::iterator it = verboseGccLines.begin(); it != verboseGccLines.end(); ++it ) {
			if( (*it).startsWith(" ") && (*it).length() > 2 && (*it)[1] != ' ' ) {
				//it is a potential include-file
				QString path = (*it).stripWhiteSpace();
				QFileInfo info( path );
				if( info.exists() ) {
					kdDebug( 9007 ) << "Adding include-path from gcc-output: \"" << path << "\" absolute: \"" << info.absFilePath() << "\"" <<  endl;
					addIncludePath(info.absFilePath());
				}
			}
		}
	} else {
		///Do some of the old stuff
		addIncludePath( "/include" );
		addIncludePath( "/usr/include" );
		addIncludePath( "/usr/local/include" );
        
		bool ok;
        QString includePath = SetupHelper::getGccIncludePath(&ok);
		if (ok) {
			QStringList ls = QStringList::split( "\n", includePath );
			for( QStringList::const_iterator it = ls.begin(); it != ls.end(); ++it ) {
			if( !(*it).isEmpty() )
				addIncludePath( *it );
			}
		}
    
        addIncludePath( includePath );
		addIncludePath( "/usr/include/g++-3" );
		addIncludePath( "/usr/include/g++" );
	}
	
	addMacro( Macro( "__cplusplus", "1" ) );

	///@todo maybe remove the following? Is there any normal user who has his environment set up correctly so this is of any use?
	QString kdedir = getenv( "KDEDIR" );
	if( !kdedir.isNull() )
		addIncludePath( kdedir + "/include" );

	QString qmakespec = getenv( "QMAKESPEC" );
	if ( qmakespec.isNull() )
	qmakespec = "linux-g++";
	
	QString qtdir = getenv( "QTDIR" );
	if( !qtdir.isNull() ) {
		addIncludePath( qtdir + "/include" );

		// #### implement other mkspecs and find a better way to find the
		// #### proper mkspec (althoigh this will be no fun :-)

		addIncludePath( qtdir + "/mkspecs/" + qmakespec );
	}

	QStringList lines = SetupHelper::getGccMacros(&ok);
	if (!ok) {
		for (QStringList::ConstIterator it = lines.constBegin(); it != lines.constEnd(); ++it) {
			QStringList lst = QStringList::split( ' ', *it );
			if ( lst.count() != 3 )
				continue;
			addMacro( Macro( lst[1], lst[2] ) );
		}
	}
		
	addMacro( Macro( "__cplusplus", "1" ) );
	addMacro( Macro( "Q_SIGNALS", "signals" ) );
	addMacro( Macro( "Q_SLOTS", "slots" ) );
	addMacro( Macro( "Q_SCRIPTABLE", "" ) );

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

	setResolveDependencesEnabled( cfg->preProcessAllHeaders() | cfg->parseMissingHeaders() );

	delete m_includePathResolver;
	if( cfg->resolveIncludePaths() ) {
		m_includePathResolver = new CppTools::IncludePathResolver( m_foreground );
		if( m_cppSupport && m_cppSupport->project() )
			m_includePathResolver->setOutOfSourceBuildSystem( m_cppSupport->project()->projectDirectory(), m_cppSupport->project()->buildDirectory() );
	} else
		m_includePathResolver = 0;
	
	m_shouldParseIncludedFiles = cfg->parseMissingHeaders();
}

QStringList KDevDriver::getCustomIncludePath( const QString& file ) {
	if( !file.startsWith("/") )
		kdDebug( 9007 ) << "KDevDriver::getCustomIncludePath(..): given file \"" << file << "\" is not absolute" << endl;
	if( !m_includePathResolver )
		return includePaths();
	CppTools::PathResolutionResult res = m_includePathResolver->resolveIncludePath( file );
	
	if( !res.success ) {
		Problem p( i18n( "%1. Message: %2" ).arg( res.errorMessage ).arg( res.longErrorMessage ), 0, 0, Problem::Level_Warning );
		p.setFileName( file );
		addProblem( file, p );
	}

	return res.path + includePaths();
}

bool KDevDriver::shouldParseIncludedFile( const ParsedFilePointer& file ) {
	QString compoundString = file->fileName() + "||" + QString("%1").arg(file->usedMacros().valueHash()) + "||" + QString("%1").arg(file->usedMacros().idHash());

	if( !m_shouldParseIncludedFiles )
		return false;
	m_cppSupport->safeFileSet().contains( compoundString );

	if( m_cppSupport->safeFileSet().contains( file->fileName()) ){
		return false;
	} else if( m_cppSupport->safeFileSet().contains( compoundString ) ) {
		//kdDebug( 9007 ) << "ALREADY IN FILE-SET: " << compoundString << endl;
		return false;
	} else {
		m_cppSupport->safeFileSet().insert( compoundString ); //This is needed so the same instance of a file is not queued many times
		//kdDebug( 9007 ) << "NOT IN FILE-SET, PARSING: " << compoundString << endl;
		return true;
	}

}

//kate: indent-mode csands; tab-width 4; space-indent off;
