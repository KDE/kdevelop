/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "driver.h"
#include "JavaLexer.hpp"
#include "JavaRecognizer.hpp"

#include <kdebug.h>
#include <stdlib.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <strstream>

class DefaultSourceProvider: public SourceProvider
{
public:
    DefaultSourceProvider() {}

    virtual QString contents( const QString& fileName )
    {
	QString source;

	QFile f( fileName );
	if( f.open(IO_ReadOnly) ){
	    QTextStream s( &f );
	    source = s.read();
	    f.close();
	}
	return source;
    }

    virtual bool isModified( const QString& fileName )
    {
	Q_UNUSED( fileName );
	return true;
    }

private:
    DefaultSourceProvider( const DefaultSourceProvider& source );
    void operator = ( const DefaultSourceProvider& source );
};


Driver::Driver()
    : depresolv( FALSE ), lexer( 0 )
{
    m_sourceProvider = new DefaultSourceProvider();
}

Driver::~Driver()
{
    reset();
}

SourceProvider* Driver::sourceProvider()
{
    return m_sourceProvider;
}

void Driver::setSourceProvider( SourceProvider* sourceProvider )
{
    if( m_sourceProvider )
	delete( m_sourceProvider );
    m_sourceProvider = sourceProvider;
}

void Driver::reset( )
{
    m_dependences.clear();
    m_problems.clear();
    m_includePaths.clear();

    while( m_parsedUnits.size() ){
	RefJavaAST unit = *m_parsedUnits.begin();
	m_parsedUnits.remove( m_parsedUnits.begin() );
        unit = 0;
    }
}

void Driver::remove( const QString & fileName )
{
    m_dependences.remove( fileName );
    m_problems.remove( fileName );

    QMap<QString, RefJavaAST>::Iterator it = m_parsedUnits.find( fileName );
    if( it != m_parsedUnits.end() ){
	RefJavaAST unit = *it;
	m_parsedUnits.remove( it );
	delete( unit );
    }
}

RefJavaAST Driver::takeTranslationUnit( const QString& fileName )
{
    QMap<QString, RefJavaAST>::Iterator it = m_parsedUnits.find( fileName );
    RefJavaAST unit( *it );
    m_parsedUnits.remove( it );
    return unit;
}

RefJavaAST Driver::translationUnit( const QString& fileName ) const
{
    QMap<QString, RefJavaAST>::ConstIterator it = m_parsedUnits.find( fileName );
    return it != m_parsedUnits.end() ? *it : 0;
}

void Driver::addDependence( const QString & fileName, const Dependence & dep )
{
    QFileInfo fileInfo( dep.first );
    QString fn = fileInfo.absFilePath();

    findOrInsertDependenceList( fileName ).insert( fn, dep );

    if ( !depresolv )
	return;

    QString file = findIncludeFile( dep );

    if ( m_parsedUnits.find(file) != m_parsedUnits.end() )
	return;

#if 0
    if ( !QFile::exists( file ) ) {
	Problem p( "Couldn't find include file " + dep.first,
		   lexer ? lexer->currentLine() : -1,
		   lexer ? lexer->currentColumn() : -1 );
	addProblem( fileName, p );
	return;
    }
#endif

    QString cfn = m_currentFileName;
    JavaLexer *l = lexer;
    parseFile( file );
    m_currentFileName = cfn;
    lexer = l;
}

void Driver::addProblem( const QString & fileName, const Problem & problem )
{
    findOrInsertProblemList( fileName ).append( problem );
}

QMap< QString, Dependence >& Driver::findOrInsertDependenceList( const QString & fileName )
{
    QMap<QString, QMap<QString, Dependence> >::Iterator it = m_dependences.find( fileName );
    if( it != m_dependences.end() )
        return it.data();

    QMap<QString, Dependence> l;
    m_dependences.insert( fileName, l );
    return m_dependences[ fileName ];
}

QValueList < Problem >& Driver::findOrInsertProblemList( const QString & fileName )
{
    QMap<QString, QValueList<Problem> >::Iterator it = m_problems.find( fileName );
    if( it != m_problems.end() )
        return it.data();

    QValueList<Problem> l;
    m_problems.insert( fileName, l );
    return m_problems[ fileName ];
}

QMap< QString, Dependence > Driver::dependences( const QString & fileName ) const
{
    QMap<QString, QMap<QString, Dependence> >::ConstIterator it = m_dependences.find( fileName );
    if( it != m_dependences.end() )
	return it.data();
    return QMap<QString, Dependence>();
}

QValueList < Problem > Driver::problems( const QString & fileName ) const
{
    QMap<QString, QValueList<Problem> >::ConstIterator it = m_problems.find( fileName );
    if( it != m_problems.end() )
	return it.data();
    return QValueList<Problem>();
}

void Driver::parseFile( const QString& fileName, bool onlyPreProcess, bool force )
{
    QString absFilePath = QFileInfo( fileName ).absFilePath();

    QMap<QString, RefJavaAST>::Iterator it = m_parsedUnits.find( absFilePath );

    if( force && it != m_parsedUnits.end() ){
	takeTranslationUnit( absFilePath );
    } else if( it != m_parsedUnits.end() && *it != 0 ){
	// file already processed
	return;
    }

    m_dependences.remove( fileName );
    m_problems.remove( fileName );

    m_currentFileName = fileName;

    std::string fn( fileName.utf8() );

    std::string source( sourceProvider()->contents(fileName).utf8() );
    std::istrstream in( source.c_str() );

    JavaLexer lex( in );
    lex.setDriver( this );
    setupLexer( &lex );
    //lexer.setProblemReporter( d->problemReporter );
    lexer = &lex;

    JavaRecognizer parser( lex );
    parser.setDriver( this );
    setupParser( &parser );
    //parser.setProblemReporter( d->problemReporter );

    try{
        // make an ast factory
        antlr::ASTFactory ast_factory;
        // initialize and put it in the parser...
        parser.initializeASTFactory (ast_factory);
        parser.setASTFactory (&ast_factory);
        /* parser.setASTNodeFactory( JavaAST::factory );  (old)  */

        parser.compilationUnit();

        RefJavaAST translationUnit = parser.getAST();
        m_parsedUnits.insert( fileName, translationUnit );

    } catch( antlr::ANTLRException& ex ){
/*        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        d->problemReporter->reportError( ex.getMessage().c_str(),
                                         lexer.getLine(),
                                         lexer.getColumn() );*/
    }

    m_currentFileName = QString::null;
    lexer = 0;
}

void Driver::setupLexer( JavaLexer * lexer )
{
}

void Driver::setupParser( JavaRecognizer * parser )
{
    Q_UNUSED( parser );
}

void Driver::addIncludePath( const QString &path )
{
    if( !path.stripWhiteSpace().isEmpty() )
        m_includePaths << path;
}

QString Driver::findIncludeFile( const Dependence& dep ) const
{
    QString fileName = dep.first;

    if( dep.second == Dep_Local ){
        QString path = QFileInfo( currentFileName() ).dirPath( true );
        QFileInfo fileInfo( QFileInfo(path, fileName) );
	if ( fileInfo.exists() && fileInfo.isFile() )
	    return fileInfo.absFilePath();

    }

    for ( QStringList::ConstIterator it = m_includePaths.begin(); it != m_includePaths.end(); ++it ) {
        QFileInfo fileInfo( *it, fileName );
	if ( fileInfo.exists() && fileInfo.isFile() )
	    return fileInfo.absFilePath();
    }

    return QString::null;
}

void Driver::setResolveDependencesEnabled( bool enabled )
{
    depresolv = enabled;
}

