/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DRIVER_H
#define DRIVER_H

#include "ast.h"

#include <qpair.h>
#include <qvaluestack.h>
#include <qstringlist.h>
#include <qasciidict.h>

class Lexer;
class Parser;

class Problem
{
public:
    Problem() {}
    Problem( const Problem& source )
	: m_text( source.m_text ), m_line( source.m_line ), m_column( source.m_column ) {}
    Problem( const QString& text, int line, int column )
	: m_text( text ), m_line( line ), m_column( column ) {}
    
    Problem& operator = ( const Problem& source )
    {
	m_text = source.m_text;
	m_line = source.m_line;
	m_column = source.m_column;
	return( *this );
    }
    
    bool operator == ( const Problem& p ) const
    {
	return m_text == p.m_text && m_line == p.m_line && m_column == p.m_column;
    }

    QString text() const { return m_text; }
    int line() const { return m_line; }
    int column() const { return m_column; }
    
private:
    QString m_text;
    int m_line;
    int m_column;
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
    typedef QPair<QString, QString> Argument;
    
public:
    Macro(): m_hasArguments( false ) {}
    
    Macro( const Macro& source )
	: m_name( source.m_name),
	  m_body( source.m_body ),
	  m_hasArguments( source.m_hasArguments ),
	  m_argumentList( source.m_argumentList ) {}
    
    Macro& operator = ( const Macro& source )
    {
	m_name = source.m_name;
	m_body = source.m_body;
	m_hasArguments = source.m_hasArguments;
	m_argumentList = source.m_argumentList;
	return *this;
    }
    
    bool operator == ( const Macro& source )
    {
	return
	    m_name == source.m_name &&
	    m_body == source.m_body &&
	    m_hasArguments == source.m_hasArguments &&
	    m_argumentList == source.m_argumentList;
    }
    
    QString name() const { return m_name; }
    void setName( const QString& name ) { m_name = name; }
    
    QString body() const { return m_body; }
    void setBody( const QString& body ) { m_body = body; }
    
    bool hasArguments() const { return m_hasArguments; }
    QValueList<Argument> argumentList() const { return m_argumentList; }
    
    void clearArgumentList() { m_argumentList.clear(); m_hasArguments = false; }
    void addArgument( const Argument& argument ) { m_hasArguments = true; m_argumentList << argument; }
    void addArgumentList( const QValueList<Argument>& arguments ) 
    { 
	m_hasArguments = true; 
	m_argumentList += arguments; 
    }
    
private:
    QString m_name;
    QString m_body;
    bool m_hasArguments;
    QValueList<Argument> m_argumentList;
};
    
class Driver
{
public:
    Driver();
    virtual ~Driver();
    
    virtual void reset();
    virtual void clear( const QString& fileName );
    
    virtual void addDependence( const QString& fileName, const Dependence& dep );
    virtual void addMacro( const QString& fileName, const Macro& macro );
    virtual void addProblem( const QString& fileName, const Problem& problem );
    
    virtual QString currentFileName() const { return m_currentFileName; }
    virtual TranslationUnitAST::Node parseFile( const QString& fileName, const QString& contents );
    
    QValueList<Dependence> dependences( const QString& fileName ) const;
    QValueList<Macro> macros( const QString& fileName ) const;
    QValueList<Problem> problems( const QString& fileName ) const;
    
protected:
    virtual void setupLexer( Lexer* lexer );
    virtual void setupParser( Parser* parser );
    
private:
    QValueList<Dependence>* findOrInsertDependenceList( const QString& fileName );
    QValueList<Macro>* findOrInsertMacroList( const QString& fileName );
    QValueList<Problem>* findOrInsertProblemList( const QString& fileName );

private:
    QString m_currentFileName;
    QAsciiDict< QValueList<Dependence> > m_dependences;
    QAsciiDict< QValueList<Macro> > m_macros;
    QAsciiDict< QValueList<Problem> > m_problems;
    
private:
    Driver( const Driver& source );
    void operator = ( const Driver& source );
};

#endif
