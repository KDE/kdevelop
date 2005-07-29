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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DRIVER_H
#define DRIVER_H

#include "JavaAST.hpp"

#include <qpair.h>
#include <qvaluestack.h>
#include <qstringlist.h>
#include <qmap.h>

class JavaLexer;
class JavaRecognizer;

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

    virtual void addProblem( const QString& fileName, const Problem& problem );

    QString currentFileName() const { return m_currentFileName; }
    RefJavaAST takeTranslationUnit( const QString& fileName );
    RefJavaAST translationUnit( const QString& fileName ) const;
    QValueList<Problem> problems( const QString& fileName ) const;

    QStringList includePaths() const { return m_includePaths; }
    virtual void addIncludePath( const QString &path );

    const QMap<QString, RefJavaAST> &parsedUnits() const { return m_parsedUnits; }

protected:
    virtual void setupLexer( JavaLexer* lexer );
    virtual void setupParser( JavaRecognizer* parser );

private:
    QValueList<Problem>& findOrInsertProblemList( const QString& fileName );

private:
    QString m_currentFileName;
    QMap< QString, QValueList<Problem> > m_problems;
    QMap< QString, RefJavaAST > m_parsedUnits;
    QStringList m_includePaths;
    JavaLexer *lexer;
    SourceProvider* m_sourceProvider;

private:
    Driver( const Driver& source );
    void operator = ( const Driver& source );
};

#endif
