

#include "driver.h"
#include "ast.h"
#include "lexer.h"

#ifdef __WALKER__
#include "my_walker.h"
#endif

#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qprocess.h>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

class MyDriver: public Driver
{
public:
    MyDriver() { setup(); }

    void setupLexer( Lexer* lex )
    {
        lex->disableSkipWords();
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

};

int main( int argc, char* argv[] )
{
    MyDriver driver;
    driver.setResolveDependencesEnabled( true );

    bool showMacros = false;

    for( int i=1; i<argc; ++i ){
	QString a = argv[ i ];
	if( a == "-m" ){
	    showMacros = true;
	    continue;
	} else if( a == "-n" || a == "--nodep" ){
	    driver.setResolveDependencesEnabled( false );
            continue;
	}

	QFile f( argv[i] );

	std::cout << "parsing " << argv[ i ];

	QFileInfo info( argv[i] );

	driver.parseFile( info.absFilePath()  );
	TranslationUnitAST* translationUnit = driver.translationUnit( info.absFilePath() );
	Q_UNUSED( translationUnit );
	QValueList<Problem> problems = driver.problems( info.absFilePath() );

	if( info.extension() == "cpp" )
	    driver.removeAllMacrosInFile( info.absFilePath() );

	if( problems.count() == 0 ){
	    std::cout << " OK" << std::endl;
#ifdef __WALKER__
	    MyWalker w;
	    w.parseTranslationUnit( translationUnit.get() );
#endif
	    continue;
	}

	QString contents = driver.sourceProvider()->contents(info.absFilePath());
	QStringList lines = QStringList::split( "\n", contents, true );
	QValueList<Problem>::Iterator it = problems.begin();
	std::cout << " found " << problems.count() << " problems" << std::endl;

	while( it != problems.end() ) {
	    Problem p = *it++;
	    QString textLine = lines[ p.line() ];

	    std::cerr << p.line()+1 << ": " << p.text() << std::endl;
	    std::cerr << p.line()+1 << ": " << textLine << std::endl;

	    QString s = textLine.left( p.column() );
	    s.replace( QRegExp("[^\t]"), " " );
	    s += "^";

	    std::cerr << p.line()+1 << ": " << s << std::endl;
	}

	std::cerr << std::endl;
    }

    std::cout << std::endl << "parsed " << driver.parsedUnits().size() << " files" << std::endl;

    if( showMacros ){
	QMap<QString, Macro> macros = driver.macros();
        std::cout << std::endl << "Macro Table #" << macros.size() << std::endl;
	std::cout << "-----------------------------------------------------------------" << std::endl;
	QMap<QString, Macro>::Iterator it = macros.begin();
	while( it != macros.end() ){
	    Macro m = it.data();
            QString fileName = m.fileName();
            QString name = m.name();
            QString body = m.body();


            if( fileName.isEmpty() )
               fileName = "<nofile>";
            if( name.isEmpty() )
               name = "<noname>";
            if( body.isEmpty() )
               body = "<nobody>";

	    std::cout << "found macro " << name << " in file " << fileName << " with body " << body;

            std::cout << std::endl;
            ++it;
	}
	std::cout << "-----------------------------------------------------------------" << std::endl << std::endl;
    }

    return -1;
}
