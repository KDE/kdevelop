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

#ifndef DRIVER_H
#define DRIVER_H

#include "ast.h"

#include <qpair.h>
#include <qvaluestack.h>
#include <qstringlist.h>
#include <qmap.h>

class Lexer;
class Parser;

class Problem
{
public:
    enum
    {
	Level_Error = 0,
	Level_Warning,
	Level_Todo,
	Level_Fixme
    };

public:
    Problem() {}
    Problem( const Problem& source )
	: m_text( source.m_text ), m_line( source.m_line ),
	  m_column( source.m_column ), m_level( source.m_level ) {}
    Problem( const QString& text, int line, int column, int level=Level_Error )
	: m_text( text ), m_line( line ), m_column( column ), m_level(level) {}

    Problem& operator = ( const Problem& source )
    {
	m_text = source.m_text;
	m_line = source.m_line;
	m_column = source.m_column;
	m_level = source.m_level;
	return( *this );
    }

    bool operator == ( const Problem& p ) const
    {
	return m_text == p.m_text && m_line == p.m_line && m_column == p.m_column && m_level == p.m_level;
    }

    QString text() const { return m_text; }
    int line() const { return m_line; }
    int column() const { return m_column; }
    int level() const { return m_level; }

private:
    QString m_text;
    int m_line;
    int m_column;
    int m_level;
};

enum
{
    Dep_Global,
    Dep_Local
};

typedef QPair<QString, int> Dependence;

class Macro
{
public:
    typedef QString Argument;

public:
    Macro( bool hasArguments = false ): m_hasArguments( hasArguments ) {}
    Macro( const QString &n, const QString &b ) : m_name( n ), m_body( b ), m_hasArguments( false ) {}

    Macro( const Macro& source )
	: m_name( source.m_name),
          m_fileName( source.m_fileName ),
	  m_body( source.m_body ),
	  m_hasArguments( source.m_hasArguments ),
	  m_argumentList( source.m_argumentList ) {}

    Macro& operator = ( const Macro& source )
    {
	m_name = source.m_name;
	m_body = source.m_body;
        m_fileName = source.m_fileName;
	m_hasArguments = source.m_hasArguments;
	m_argumentList = source.m_argumentList;
	return *this;
    }

    bool operator == ( const Macro& source ) const
    {
	return
	    m_name == source.m_name &&
            m_fileName == source.m_fileName &&
	    m_body == source.m_body &&
	    m_hasArguments == source.m_hasArguments &&
	    m_argumentList == source.m_argumentList;
    }

    QString name() const { return m_name; }
    void setName( const QString& name ) { m_name = name; }

    QString fileName() const { return m_fileName; }
    void setFileName( const QString& fileName ) { m_fileName = fileName; }

    QString body() const { return m_body; }
    void setBody( const QString& body ) { m_body = body; }

    bool hasArguments() const { return m_hasArguments; }
    void setHasArguments( bool hasArguments ) { m_hasArguments = hasArguments; }
    QValueList<Argument> argumentList() const { return m_argumentList; }

    void clearArgumentList() { m_argumentList.clear(); m_hasArguments = false; }
    void addArgument( const Argument& argument ) { m_argumentList << argument; }
    void addArgumentList( const QValueList<Argument>& arguments ) { m_argumentList += arguments; }

private:
    QString m_name;
    QString m_fileName;
    QString m_body;
    bool m_hasArguments;
    QValueList<Argument> m_argumentList;
};

class SourceProvider
{
public:
    SourceProvider() {}
    virtual ~SourceProvider() {}

    virtual QString contents( const QString& fileName ) = 0;
    virtual bool isModified( const QString& fileName ) = 0;

private:
    SourceProvider( const SourceProvider& source );
    void operator = ( const SourceProvider& source );
};

class Driver
{
public:
    Driver();
    virtual ~Driver();

    SourceProvider* sourceProvider();
    void setSourceProvider( SourceProvider* sourceProvider );

    virtual void reset();

    virtual void parseFile( const QString& fileName, bool onlyPreProcesss=false, bool force=false );
    virtual void fileParsed( const QString& fileName );
    virtual void remove( const QString& fileName );

    virtual void addDependence( const QString& fileName, const Dependence& dep );
    virtual void addMacro( const Macro& macro );
    virtual void addProblem( const QString& fileName, const Problem& problem );


    QString currentFileName() const { return m_currentFileName; }
    TranslationUnitAST::Node takeTranslationUnit( const QString& fileName );
    TranslationUnitAST* translationUnit( const QString& fileName ) const;
    QMap<QString, Dependence> dependences( const QString& fileName ) const;
    QMap<QString, Macro> macros() const;
    QValueList<Problem> problems( const QString& fileName ) const;

    bool hasMacro( const QString& name ) const { return m_macros.contains( name ); }
    const Macro& macro( const QString& name ) const { return m_macros[ name ]; }
    Macro& macro( const QString& name ) { return m_macros[ name ]; }

    virtual void removeMacro( const QString& macroName );
    virtual void removeAllMacrosInFile( const QString& fileName );

    QStringList includePaths() const { return m_includePaths; }
    virtual void addIncludePath( const QString &path );

    /// @todo remove
    const QMap<QString, TranslationUnitAST*> &parsedUnits() const { return m_parsedUnits; }

    virtual void setResolveDependencesEnabled( bool enabled );
    bool isResolveDependencesEnabled() const { return depresolv; }

protected:
    virtual void setupLexer( Lexer* lexer );
    virtual void setupParser( Parser* parser );
    virtual void setupPreProcessor();

private:
    QMap<QString, Dependence>& findOrInsertDependenceList( const QString& fileName );
    QValueList<Problem>& findOrInsertProblemList( const QString& fileName );
    QString findIncludeFile( const Dependence& dep ) const;

private:
    QString m_currentFileName;
    QMap< QString, QMap<QString, Dependence> > m_dependences;
    QMap<QString, Macro> m_macros;
    QMap< QString, QValueList<Problem> > m_problems;
    QMap<QString, TranslationUnitAST*> m_parsedUnits;
    QStringList m_includePaths;
    uint depresolv : 1;
    Lexer *lexer;
    SourceProvider* m_sourceProvider;

private:
    Driver( const Driver& source );
    void operator = ( const Driver& source );
};

#endif
