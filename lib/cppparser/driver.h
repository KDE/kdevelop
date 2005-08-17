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
#include <q3valuestack.h>
#include <qstringlist.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3ValueList>

class Lexer;
class Parser;
/**
 * Encapsulates a problem in a piece of source code.
 */
class Problem
{
public:
    enum
    {
	Level_Error = 0, ///< Indicates an error that will prevent the code from compiling
	Level_Warning, ///< Indicates a warning
	Level_Todo, ///< Indicates there is still something left to do
	Level_Fixme ///< Indicates that something needs to be fixed
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

    /** Get the text for the problem */
    QString text() const { return m_text; }
    /** Get the line number of the problem */
    int line() const { return m_line; }
    /** Get the column of the problem */
    int column() const { return m_column; }
    /**
     * Get the seriousness of the problem. There are four possibilities:
     * \li Error
     * \li Warning
     * \li Todo
     * \li Fixme
     */
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

/**
 * A datatype that represents a preprocessor macro.
 */
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

    /** Get the name for this macro */
    QString name() const { return m_name; }
    /** Set the name for this macro */
    void setName( const QString& name ) { m_name = name; }

    /** Get the file name that contains this macro */
    QString fileName() const { return m_fileName; }
    /** Set the file name that contains this macro */
    void setFileName( const QString& fileName ) { m_fileName = fileName; }

    /** Get the body of the macro */
    QString body() const { return m_body; }
    /** Set the body of the macro */
    void setBody( const QString& body ) { m_body = body; }

    /** Check whether the macro has arguments that are passed to it */
    bool hasArguments() const { return m_hasArguments; }
    void setHasArguments( bool hasArguments ) { m_hasArguments = hasArguments; }
    /** Get a list of arguments passed to this macro */
    Q3ValueList<Argument> argumentList() const { return m_argumentList; }

    /** Clear the list of arguments this macro has */
    void clearArgumentList() { m_argumentList.clear(); m_hasArguments = false; }
    /** Add an argument to this macro */
    void addArgument( const Argument& argument ) { m_argumentList << argument; }
    /** Add a list of arguments to this macro */
    void addArgumentList( const Q3ValueList<Argument>& arguments ) { m_argumentList += arguments; }

private:
    QString m_name;
    QString m_fileName;
    QString m_body;
    bool m_hasArguments;
    Q3ValueList<Argument> m_argumentList;
};

/**
 * An interface that provides source code to the Driver
 */
class SourceProvider
{
public:
    SourceProvider() {}
    virtual ~SourceProvider() {}

    /**
     * Get the contents of a file
     * \param fileName The name of the file to get the contents for. An absolute
     *                 path should be used.
     * \return A QString that contains the contents of the file
     */
    virtual QString contents( const QString& fileName ) = 0;

    /**
     * Check to see if a file has been modified
     * \param fileName The name of hte file to get the modification state of. An
     *                 absolute path should be used.
     * \return true if the file has been modified
     * \return false if the file has not been modified
     */
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

    /**
     * Get the source provider for this driver. This would be useful for
     * getting the text the driver is working with.
     */
    SourceProvider* sourceProvider();
    /**
     * Sets the source provider the driver will use
     * @param sourceProvider the SourceProvider the driver will use
     */
    void setSourceProvider( SourceProvider* sourceProvider );

    /**
     * @brief Resets the driver
     *
     * Clears the driver of all problems, dependencies, macros, and include paths and
     * removes any translation units that have been parsed
     */
    virtual void reset();

    /**
     * Tells the driver to start parsing a file
     * @param fileName The name of the file to parse
     * @param onlyPreProcesss Tells the driver to only run the file through the preprocessor. Defaults to false
     * @param force Force the parsing of the file. Defaults to false
     */
    virtual void parseFile( const QString& fileName, bool onlyPreProcesss=false, bool force=false );

    /**
     * Indicates that the file has been parsed
     * @param fileName The name of the file parsed
     */
    virtual void fileParsed( const QString& fileName );

    /**
     * Removes the file specified by @p fileName from the driver
     * @param fileName The name of the file to remove
     */
    virtual void remove( const QString& fileName );

    /**
     * Add a dependency on another header file for @p fileName
     * @param fileName The file name to add the dependency for
     * @param dep The dependency to add
     */
    virtual void addDependence( const QString& fileName, const Dependence& dep );

    /**
     * Add a macro to the driver
     * @param macro The macro to add to the driver
     */
    virtual void addMacro( const Macro& macro );

    /**
     * Add a problem to the driver
     * @param fileName The file name to add the problem for
     * @param problem The problem to add
     */
    virtual void addProblem( const QString& fileName, const Problem& problem );


    /**
     * The current file name the driver is working with
     */
    QString currentFileName() const { return m_currentFileName; }
    TranslationUnitAST::Node takeTranslationUnit( const QString& fileName );
    /**
     * Get the translation unit contained in the driver for @p fileName.
     * @param fileName The name of the file to get the translation unit for
     * @return The TranslationUnitAST pointer that represents the translation unit
     * @return 0 if no translation unit exists for the file
     */
    TranslationUnitAST* translationUnit( const QString& fileName ) const;
    /**
     * Get the dependencies for a file
     * @param fileName The file name to get dependencies for
     * @return The dependencies for the file
     */
    QMap<QString, Dependence> dependences( const QString& fileName ) const;
    /**
     * Get all the macros the driver contains
     * @return The macros
     */
    QMap<QString, Macro> macros() const;
    /**
     * Get the list of problem areas the driver contains
     * @param fileName The filename to get problems for
     * @return The list of problems for @p fileName
     */
    Q3ValueList<Problem> problems( const QString& fileName ) const;

    /**
     * Check if we have a macro in the driver
     * @param name The name of the macro to check for
     * @return true if we have the macro in the driver
     * @return false if we don't have the macro in the driver
     */
    bool hasMacro( const QString& name ) const { return m_macros.contains( name ); }
    /**
     * Get the macro identified by @p name
     * @param name The name of the macro to get
     * @return A const reference of the macro object represented by @p name
     */
    const Macro& macro( const QString& name ) const { return m_macros[ name ]; }
    /**
     * Get the macro identified by @p name
     * @override
     * @param name The name of the macro to get
     * @return A non-const reference of the macro object represented by @p name
     * 
     */
    Macro& macro( const QString& name ) { return m_macros[ name ]; }

    /**
     * Remove a macro from the driver
     * @param macroName The name of the macro to remove
     */
    virtual void removeMacro( const QString& macroName );
    /**
     * Remove all macros from the driver for a certain file
     * @param fileName The file name 
     */
    virtual void removeAllMacrosInFile( const QString& fileName );

    QStringList includePaths() const { return m_includePaths; }
    virtual void addIncludePath( const QString &path );

    /// @todo remove
    const QMap<QString, TranslationUnitAST*> &parsedUnits() const { return m_parsedUnits; }

    /**
     * Set whether or not to enable dependency resolving for files added to the driver
     */
    virtual void setResolveDependencesEnabled( bool enabled );
    /**
     * Check if dependency resolving is enabled
     * \return true if dependency resolving is enabled
     * \return false if dependency resolving is disabled
     */
    bool isResolveDependencesEnabled() const { return depresolv; }

protected:
    /**
     * Set up the lexer.
     */
    virtual void setupLexer( Lexer* lexer );
    /**
     * Setup the parser
     */
    virtual void setupParser( Parser* parser );
    /**
     * Set up the preprocessor
     */
    virtual void setupPreProcessor();

private:
    QMap<QString, Dependence>& findOrInsertDependenceList( const QString& fileName );
    Q3ValueList<Problem>& findOrInsertProblemList( const QString& fileName );
    QString findIncludeFile( const Dependence& dep ) const;

private:
    QString m_currentFileName;
    QMap< QString, QMap<QString, Dependence> > m_dependences;
    QMap<QString, Macro> m_macros;
    QMap< QString, Q3ValueList<Problem> > m_problems;
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
