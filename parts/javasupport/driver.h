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
    virtual void remove( const QString& fileName );

    virtual void addDependence( const QString& fileName, const Dependence& dep );
    virtual void addProblem( const QString& fileName, const Problem& problem );

    QString currentFileName() const { return m_currentFileName; }
    RefJavaAST takeTranslationUnit( const QString& fileName );
    RefJavaAST translationUnit( const QString& fileName ) const;
    QMap<QString, Dependence> dependences( const QString& fileName ) const;
    QValueList<Problem> problems( const QString& fileName ) const;

    QStringList includePaths() const { return m_includePaths; }
    virtual void addIncludePath( const QString &path );

    // TODO: remove
    const QMap<QString, RefJavaAST> &parsedUnits() const { return m_parsedUnits; }

    virtual void setResolveDependencesEnabled( bool enabled );
    bool isResolveDependencesEnabled() const { return depresolv; }

protected:
    virtual void setupLexer( JavaLexer* lexer );
    virtual void setupParser( JavaRecognizer* parser );

private:
    QMap<QString, Dependence>& findOrInsertDependenceList( const QString& fileName );
    QValueList<Problem>& findOrInsertProblemList( const QString& fileName );
    QString findIncludeFile( const Dependence& dep ) const;

private:
    QString m_currentFileName;
    QMap< QString, QMap<QString, Dependence> > m_dependences;
    QMap< QString, QValueList<Problem> > m_problems;
    QMap<QString, RefJavaAST> m_parsedUnits;
    QStringList m_includePaths;
    uint depresolv : 1;
    JavaLexer *lexer;
    SourceProvider* m_sourceProvider;

private:
    Driver( const Driver& source );
    void operator = ( const Driver& source );
};

#endif
