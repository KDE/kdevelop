
#include <iostream>

#include "driver.h"
#include "ast.h"
#include "lexer.h"
#include "tag_creator.h"
#include "setuphelper.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <catalog.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <stdlib.h>
#include <unistd.h>

class RppDriver: public Driver
{
public:
	RppDriver( Catalog* c )
		: catalog( c ), m_generateTags( true )
	{
		setup();
	}
	
	virtual ~RppDriver()
	{
		TagCreator::destroyDocumentation();
	}
	
	void setGenerateTags( bool b )
	{
		m_generateTags = b;
	}
	
	void addDocDirectory( const QString& dir )
	{
		m_docDirectoryList.append( dir );
		TagCreator::setDocumentationDirectories( m_docDirectoryList );
	}
	
	void fileParsed( const ParsedFile& fileName )
	{
		std::cout << ( m_generateTags ? "generate tags for " : "checking " ) 
			<< QFile::encodeName( fileName.fileName() ).data() << std::endl;
		
		QValueList<Problem> l = problems( fileName.fileName() );
		QValueList<Problem>::Iterator it = l.begin();
		while ( it != l.end() )
		{
			const Problem & p = *it;
			++it;
		std::cout << QFile::encodeName( fileName.fileName() ).data() << ":" << p.line() << ":"
				<< p.column() << ": " << p.text().latin1() << std::endl;
		}
		
		takeTranslationUnit( fileName );
		
		if ( m_generateTags )
		{
			TagCreator w( fileName.fileName(), catalog );
			w.parseTranslationUnit( fileName );
		}
		
		if ( !isResolveDependencesEnabled() )
			removeAllMacrosInFile( fileName.fileName() );
	}
	
	void setupLexer( Lexer* lex )
	{
		//lex->disableSkipWords();
		Driver::setupLexer( lex );
	}
	
	// setup the preprocessor
	// code provided by Reginald Stadlbauer <reggie@trolltech.com>
	void setup()
	{
		QString kdedir = getenv( "KDEDIR" );
		if ( !kdedir.isNull() )
			addIncludePath( kdedir + "/include" );
		
		QString qtdir = getenv( "QTDIR" );
		if ( !qtdir.isNull() )
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
			addIncludePath( "/ust/local/include" );
            bool ok;    
            QString gccLibPath = SetupHelper::getGccIncludePath(&ok);
            if (!ok) 
                return;  			
			gccLibPath = gccLibPath.replace( QRegExp( "[\r\n]" ), "" );
			addIncludePath( gccLibPath );
			addIncludePath( "/usr/include/g++-3" );
			addIncludePath( "/usr/include/g++" );
            QStringList lines = SetupHelper::getGccMacros(&ok);
            if (!ok) 
                return;  
            for (QStringList::ConstIterator it = lines.constBegin(); it != lines.constEnd(); ++it) {
                QStringList lst = QStringList::split( ' ', *it );
                if ( lst.count() != 3 )
                    continue;
                addMacro( Macro( lst[1], lst[2] ) );
            }
			addMacro( Macro( "__cplusplus", "1" ) );
			
			QString incl = getenv( "INCLUDE" );
		    QStringList includePaths = QStringList::split( ':', incl );
			QStringList::Iterator it = includePaths.begin();
			while ( it != includePaths.end() )
			{
				addIncludePath( ( *it ).stripWhiteSpace() );
				++it;
			}
			
		}
		else if ( qmakespec == "win32-borland" )
		{
			QString incl = getenv( "INCLUDE" );
			QStringList includePaths = QStringList::split( ';', incl );
			QStringList::Iterator it = includePaths.begin();
			while ( it != includePaths.end() )
			{
				addIncludePath( ( *it ).stripWhiteSpace() );
				++it;
			}
			// ### I am sure there are more standard include paths on
			// ### windows. I will fix that soon
			// ### Also do the compiler specific defines on windows
		}
	}

private:
	Catalog* catalog;
	bool m_generateTags;
	QStringList m_docDirectoryList;
};

void parseDirectory( Driver& driver, QDir& dir, bool rec, bool parseAllFiles )
{
	
	QStringList fileList;
	if ( parseAllFiles )
		fileList = dir.entryList( QDir::Files );
	else
		fileList = dir.entryList( "*.h;*.H;*.hh;*.hxx;*.hpp;*.tlh" );
	
	QStringList::Iterator it = fileList.begin();
	while ( it != fileList.end() )
	{
		QString fn = dir.path() + "/" + ( *it );
		++it;
		
		driver.parseFile( fn );
	}
	
	if ( rec )
	{
		QStringList fileList = dir.entryList( QDir::Dirs );
		QStringList::Iterator it = fileList.begin();
		while ( it != fileList.end() )
		{
			if ( ( *it ).startsWith( "." ) )
			{
				++it;
				continue;
			}
			
			QDir subdir( dir.path() + "/" + ( *it ) );
			++it;
			
			parseDirectory( driver, subdir, rec, parseAllFiles );
		}
	}
}

int main( int argc, char* argv[] )
{
	KStandardDirs stddir;
	
	if ( argc < 3 )
	{
		std::cerr << "usage: r++ dbname directories..." << std::endl << std::endl;
		return -1;
	}
	
	bool rec = false;
	bool parseAllFiles = false;
	
	QString datadir = stddir.localkdedir() + "/" + KStandardDirs::kde_default( "data" );
	if ( ! KStandardDirs::makeDir( datadir + "/kdevcppsupport/pcs/" ) )
	{
		kdWarning() << "*error* " << "could not create " << datadir + "/kdevcppsupport/pcs/" << endl << endl;
		return -1;
	}
	
	
	if ( !QFile::exists( datadir + "/kdevcppsupport/pcs/" ) )
	{
		kdWarning() << "*error* " << datadir + "/kdevcppsupport/pcs/" << " doesn't exists!!" << endl << endl;
		return -1;
	}
	
	QString dbFileName = datadir + "/kdevcppsupport/pcs/" + argv[ 1 ] + ".db";
	// std::cout << "dbFileName = " << dbFileName << std::endl;
	if ( QFile::exists( dbFileName ) )
	{
		kdWarning() << "*error* " << "database " << dbFileName << " already exists!" << endl << endl;
		return -1;
	}
	
	
	Catalog catalog;
	catalog.open( dbFileName );
	catalog.addIndex( "kind" );
	catalog.addIndex( "name" );
	catalog.addIndex( "scope" );
	catalog.addIndex( "fileName" );
	
	RppDriver driver( &catalog );
	driver.setResolveDependencesEnabled( true );
	
	for ( int i = 2; i < argc; ++i )
	{
		QString s( argv[ i ] );
		if ( s == "-r" || s == "--recursive" )
		{
			rec = true;
			continue;
		}
		else if ( s == "-a" || s == "--all" )
		{
			parseAllFiles = true;
			continue;
		}
		else if ( s == "-f" || s == "--fast" )
		{
			driver.setResolveDependencesEnabled( false );
			continue;
		}
		else if ( s == "-c" || s == "--check-only" )
		{
			driver.setGenerateTags( false );
			continue;
		}
		else if ( s.startsWith( "-d" ) )
		{
			driver.addDocDirectory( s.mid( 2 ) );
			continue;
		}
		
		QDir dir( s );
		if ( !dir.exists() )
		{
			kdWarning() << "*error* " << "the directory " << dir.path() << " doesn't exists!" << endl << endl;
			continue;
		}
		
		parseDirectory( driver, dir, rec, parseAllFiles );
	}
	
	return 0;
}

//kate: indent-mode csands; tab-width 4; space-indent off;
