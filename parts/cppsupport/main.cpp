

#include "driver.h"
#include "ast.h"
#include "lexer.h"
#include "tag_creator.h"

#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qprocess.h>

#include <catalog.h>
#include <kstandarddirs.h>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

class MyDriver: public Driver
{
public:
    MyDriver() { setup(); }

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

	if ( qmakespec == "linux-g++" ) {
	    addIncludePath( "/include" );
	    addIncludePath( "/usr/include" );
	    addIncludePath( "/ust/local/include" );
	    QProcess proc;
	    proc.addArgument( "gcc" );
	    proc.addArgument( "-print-file-name=include" );
	    if ( !proc.start() ) {
		qWarning( "Couldn't start gcc" );
		return;
	    }
	    while ( proc.isRunning() )
		usleep( 1 );

            QString gccLibPath = proc.readStdout();
            gccLibPath = gccLibPath.replace( QRegExp("[\r\n]"), "" );
	    addIncludePath( gccLibPath );
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
		addIncludePath( (*it).stripWhiteSpace() );
		++it;
	    }
	    // ### I am sure there are more standard include paths on
	    // ### windows. I will fix that soon
	    // ### Also do the compiler specific defines on windows
	}
    }

};

void parseDirectory( Driver& driver, QDir& dir, bool rec )
{
  {
    QStringList fileList = dir.entryList( "*.h;*.H;*.hh;*.hxx;*.hpp;*.tlh" );
    QStringList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
        QString fn = dir.path() + "/" + (*it);
        ++it;

        std::cout << "parsing file " << fn << std::endl; 
        driver.parseFile( fn );
    }
  }

  if( rec ) {
    QStringList fileList = dir.entryList( QDir::Dirs );
    QStringList::Iterator it = fileList.begin();
    while( it != fileList.end() ){
        if( (*it).startsWith(".") ){
            ++it;
            continue;
        }

        QDir subdir( dir.path() + "/" + (*it) );
        ++it; 

        parseDirectory( driver, subdir, rec );
    }
  }
}

int main( int argc, char* argv[] )
{
    MyDriver driver;
    driver.setResolveDependencesEnabled( true );
    KStandardDirs stddir;   

    if( argc < 2 ){
        std::cerr << "usage: r++ dbname directories..." << std::endl << std::endl;     
        return -1;
    }

    bool rec = false;

    QString dbFileName = stddir.localkdedir() + "/" + KStandardDirs::kde_default( "data" ) + "/kdevcppsupport/pcs/" + argv[ 1 ] + ".db";
    std::cout << "dbFileName = " << dbFileName << std::endl;
    if( QFile::exists(dbFileName) ){
        std::cerr << "database " << dbFileName << " already exists!" << std::endl << std::endl;     
        return -1;
    }

    for( int i=2; i<argc; ++i ){
        QString s( argv[i] );
        if( s == "-r" || s == "--recursive" ){
           rec = true;
           continue; 
        }

        QDir dir( s );
        if( !dir.exists() ){
            std::cerr << "the directory " << dir.path() << " doesn't exists!" << std::endl << std::endl;     
            continue;
        }
        parseDirectory( driver, dir, rec ); 
    }

    Catalog catalog;
    catalog.open( dbFileName );
    catalog.addIndex( "kind" );
    catalog.addIndex( "name" );
    catalog.addIndex( "scope" );
    catalog.addIndex( "fileName" );

    QMap<QString, TranslationUnitAST*> units = driver.parsedUnits();
    QMap<QString, TranslationUnitAST*>::Iterator unitIt = units.begin();
    while( unitIt != units.end() ){
        TagCreator w( unitIt.key(), &catalog );
        w.parseTranslationUnit( unitIt.data() );

        TranslationUnitAST::Node node = driver.takeTranslationUnit( unitIt.key() );
        node.reset();

        ++unitIt;
    }

    return 0;
}
