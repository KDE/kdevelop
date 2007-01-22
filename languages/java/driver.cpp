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

#include "JavaAST.hpp"
#include "JavaLexer.hpp"
#include "JavaRecognizer.hpp"

#include <kdebug.h>
#include <stdlib.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <string>
#include <sstream>

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
    : lexer( 0 )
{
    m_sourceProvider = new DefaultSourceProvider();
}

Driver::~Driver()
{
    reset();
    delete( m_sourceProvider );
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
    m_problems.clear();
    m_includePaths.clear();

    while( m_parsedUnits.size() ){
	RefJavaAST unit = *m_parsedUnits.begin();
	m_parsedUnits.remove( m_parsedUnits.begin() );
	delete( unit );
    }
}

void Driver::remove( const QString & fileName )
{
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
    //m_parsedUnits.remove( it );
    m_parsedUnits[ fileName] = 0;
    return unit;
}

RefJavaAST Driver::translationUnit( const QString& fileName ) const
{
    QMap<QString, RefJavaAST>::ConstIterator it = m_parsedUnits.find( fileName );
    return it != m_parsedUnits.end() ? *it : RefJavaAST();
}

void Driver::addProblem( const QString & fileName, const Problem & problem )
{
    findOrInsertProblemList( fileName ).append( problem );
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

QValueList < Problem > Driver::problems( const QString & fileName ) const
{
    QMap<QString, QValueList<Problem> >::ConstIterator it = m_problems.find( fileName );
    if( it != m_problems.end() )
	return it.data();
    return QValueList<Problem>();
}

void Driver::parseFile( const QString& fileName, bool onlyPreProcess, bool force )
{
    QFileInfo fileInfo( fileName );
    QString absFilePath = fileInfo.absFilePath();

    QMap<QString, RefJavaAST>::Iterator it = m_parsedUnits.find( absFilePath );

    if( force && it != m_parsedUnits.end() ){
	takeTranslationUnit( absFilePath );
    } else if( it != m_parsedUnits.end() && *it != 0 ){
	// file already processed
	return;
    }

    m_problems.remove( fileName );

    m_currentFileName = fileName;

    std::string source( sourceProvider()->contents(fileName).utf8() );
    std::istringstream in( source.c_str() );

    JavaLexer lex( in );
    lex.setDriver( this );
    lexer = &lex;
    setupLexer( &lex );


    /// @todo lex.setSource( sourceProvider()->contents(fileName) );

    RefJavaAST translationUnit;
    if( !onlyPreProcess ){
	JavaRecognizer parser( lex );
	parser.setDriver( this );
	setupParser( &parser );


    try{
        // make an ast factory
        ANTLR_USE_NAMESPACE(antlr)JavaASTFactory ast_factory;
        // initialize and put it in the parser...
        parser.initializeASTFactory (ast_factory);
        parser.setASTFactory (&ast_factory);

        parser.compilationUnit();

        RefJavaAST translationUnit = RefJavaAST( parser.getAST() );
        m_parsedUnits.insert( fileName, translationUnit );

    } catch( ANTLR_USE_NAMESPACE(antlr)ANTLRException& ex ){}

    }

    m_currentFileName = QString::null;
    lexer = 0;

    fileParsed( fileName );
}

void Driver::setupLexer( JavaLexer * // lexer
                         )
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

void Driver::fileParsed( const QString & fileName )
{
    Q_UNUSED( fileName );
}
