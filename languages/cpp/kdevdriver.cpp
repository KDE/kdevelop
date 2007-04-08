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

	setResolveDependencesEnabled( cfg->preProcessAllHeaders() | cfg->parseMissingHeaders() );

	if( cfg->resolveIncludePaths() ) {
		delete m_includePathResolver;
		m_includePathResolver = new CppTools::IncludePathResolver( m_foreground );
	}
	
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
	return m_shouldParseIncludedFiles && !m_cppSupport->safeFileSet().contains(file->fileName()) && !m_cppSupport->safeFileSet().contains( file->fileName() + "||" + QString("%1").arg(file->usedMacros().valueHash()) + "||" + QString("%1").arg(file->usedMacros().idHash()) );
}

//kate: indent-mode csands; tab-width 4; space-indent off;
