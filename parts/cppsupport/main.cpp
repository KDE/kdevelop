

#include "driver.h"
#include "ast.h"

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
    MyDriver(): count(0) {}


    TranslationUnitAST::Node parseFile( const QString & fileName, const QString& source )
    {
       ++count;
       return Driver::parseFile( fileName, source );
    }

    // setup the preprocessor
    // code provided by Reginald Stadlbauer <reggie@trolltech.com>
    void setupPreProcessor()
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

    int count;
};

int main( int argc, char* argv[] )
{
    MyDriver driver;
    driver.setResolveDependencesEnabled( true );
    
    bool showMacros = false;
    
    for( int i=1; i<argc; ++i ){
	if( QString(argv[i]) == "-m" ){
	    showMacros = true;
	    continue;
	}
	
	QFile f( argv[i] );
	
	if( !f.open(IO_ReadOnly) ){
	    std::cout << "cannot open file " << argv[i] << std::endl;
	    continue;
	}
	
	std::cout << "parsing " << argv[ i ];
	
	QTextStream s( &f );
	QString contents = s.read();
	f.close();
	
	QFileInfo info( argv[i] );
	
	TranslationUnitAST::Node translationUnit = driver.parseFile( info.absFilePath(), contents  );
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
    
    std::cout << "parsed " << driver.count << " files" << std::endl;
    
    
    if( showMacros ){
        std::cout << std::endl << "Macro Table" << std::endl;
	std::cout << "-----------------------------------------------------------------" << std::endl;
	QValueList<Macro> macros = driver.macros().values();
	QValueListIterator<Macro> it = macros.begin();
	while( it != macros.end() ){
		Macro m = *it++;
		std::cout << "found macro " << m.name() << " in file " << m.fileName() << std::endl;
	}
	std::cout << "-----------------------------------------------------------------" << std::endl << std::endl;
    }

    return -1;
}
