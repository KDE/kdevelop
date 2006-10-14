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
#include <qcstring.h>
#include <qdatastream.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <map>
#include <set>
#include <lib/interfaces/hashedstring.h>
#include <ksharedptr.h>
#include <lib/interfaces/codemodel.h>
#include <ext/hash_map>


class Lexer;
class Parser;

/**
 * Encapsulates a problem in a piece of source code.
 */
class Problem {
  public:
    enum
    {
      Level_Error = 0,   ///< Indicates an error that will prevent the code from compiling
      Level_Warning,   ///< Indicates a warning
      Level_Todo,   ///< Indicates there is still something left to do
      Level_Fixme ///< Indicates that something needs to be fixed
  };

  public:
    Problem() {}
    Problem( const Problem& source )
        : m_text( source.m_text ), m_line( source.m_line ),
    m_column( source.m_column ), m_level( source.m_level ), m_file( source.m_file ) {}
    Problem( const QString& text, int line, int column, int level = Level_Error )
        : m_text( text ), m_line( line ), m_column( column ), m_level( level ) {}

    Problem& operator = ( const Problem& source ) {
      m_text = source.m_text;
      m_line = source.m_line;
      m_column = source.m_column;
      m_level = source.m_level;
      m_file = source.m_file;
      return ( *this );
    }
    
    void setFileName( const QString& fileName ) {
        m_file = fileName;
    }

    bool operator == ( const Problem& p ) const {
      return m_text == p.m_text && m_line == p.m_line && m_column == p.m_column && m_level == p.m_level && m_file == p.m_file;
    }
    
    /** Get the filename in which the problem was encountered */
    QString fileName() const {
        return m_file;
    }

    /** Get the text for the problem */
    QString text() const {
      return m_text;
    }
    /** Get the line number of the problem */
    int line() const {
      return m_line;
    }
    /** Get the column of the problem */
    int column() const {
      return m_column;
    }
    /**
     * Get the seriousness of the problem. There are four possibilities:
     * \li Error
     * \li Warning
     * \li Todo
     * \li Fixme
     */
    int level() const {
      return m_level;
    }

  private:
    QString m_text;
    int m_line;
    int m_column;
    int m_level;
    QString m_file;
};

enum
{
  Dep_Global,
  Dep_Local
};

typedef QPair<QString, int> Dependence;

/**
 * A datatype that represents a preprocessor macro.
 * Most of the functions in this class need to be inline, so we do not have to import cppparser to many modules. The other solution would be moving macro into interfaces.
 */
class Macro {
  public:
    typedef QString Argument;

  public:
      Macro( bool hasArguments = false ) : m_idHashValid( false ), m_valueHashValid( false ), m_hasArguments( hasArguments ), m_isUndefMacro( false ) {}
      Macro( const QString &n, const QString &b ) : m_idHashValid( false ), m_valueHashValid( false ), m_name( n ), m_body( b ), m_hasArguments( false ), m_isUndefMacro( false ) {}

      //Sorts the macros by their hash-value, then by their name.
    struct NameArgCompare {
      bool operator () ( const Macro& lhs, const Macro& rhs ) const {
          size_t lhash = lhs.idHash();
          size_t rhash = rhs.idHash();
          if( lhash < rhash ) return true;
          else if( lhash > rhash ) return false;
          
        int df = lhs.m_name.compare( rhs.m_name );
        if ( df < 0 )
          return true;
        if ( df == 0 ) {
          if ( !lhs.m_hasArguments && rhs.m_hasArguments ) {
            return true;
          } else if ( lhs.m_hasArguments == rhs.m_hasArguments ) {
            return lhs.m_argumentList.count() < rhs.m_argumentList.count();

          } else {
            return false;
          }
        }
        return false;
      }
    };

    struct NameArgHash {
        size_t operator () ( const Macro& macro ) const {
            return macro.idHash();
        }
    };
    
    struct NameArgEqual {
        bool operator () ( const Macro& lhs, const Macro& rhs ) const {
            int df = lhs.m_name.compare( rhs.m_name );
            if ( df == 0 ) {
                if ( lhs.m_hasArguments != rhs.m_hasArguments ) {
                    return false;
                } else {
                    if( lhs.m_argumentList.count() != rhs.m_argumentList.count() ) return false;
                    /*QStringList::const_iterator it2 = rhs.m_argumentList.begin();
                    for( QStringList::const_iterator it = lhs.m_argumentList.begin(); it != lhs.m_argumentList.end(); ) {
                        if( *it != *it2 ) return false;

                        ++it;
                        ++it2;
                    }*/
                    return true;

                }
            }
            return false;
        }
    };

    Macro( const Macro& source )
        : m_idHashValid( source.m_idHashValid ), m_valueHashValid( source.m_valueHashValid ), m_idHash( source.m_idHash ), m_valueHash( source.m_valueHash ), m_name( source.m_name ),
        m_fileName( source.m_fileName ),
        m_body( source.m_body ),
        m_hasArguments( source.m_hasArguments ),
        m_argumentList( source.m_argumentList ), m_isUndefMacro( source.m_isUndefMacro ) {}

    Macro& operator = ( const Macro& source ) {
        m_idHashValid = source.m_idHashValid;
        m_valueHashValid = source.m_valueHashValid;
        m_idHash = source.m_idHash;
        m_valueHash = source.m_valueHash;
        m_name = source.m_name;
        m_body = source.m_body;
        m_fileName = source.m_fileName;
        m_hasArguments = source.m_hasArguments;
        m_argumentList = source.m_argumentList;
        m_isUndefMacro = source.m_isUndefMacro;
        return *this;
    }

    bool operator == ( const Macro& source ) const {
        if( !m_idHashValid || !m_valueHashValid ) computeHash();
        if( !source.m_idHashValid || !source.m_valueHashValid ) source.computeHash();
        
        if( m_idHash != source.m_idHash ) return false;
        if( m_valueHash != source.m_valueHash ) return false;
        
        return m_name == source.m_name &&
        m_fileName == source.m_fileName &&
        m_body == source.m_body &&
        m_hasArguments == source.m_hasArguments &&
                m_argumentList == source.m_argumentList && m_isUndefMacro == source.m_isUndefMacro;
    }

    void read( QDataStream& stream ) {
        Q_INT8 i;
        stream >> i; m_idHashValid = i;
        stream >> i; m_valueHashValid = i;
        stream >> i; m_hasArguments = i;
        
        stream >> m_idHash;
        stream >> m_valueHash;
        stream >> m_name;
        stream >> m_body;
        stream >> m_fileName;
        stream >> m_argumentList;
    }
    
    void write( QDataStream& stream ) const {
        Q_INT8 i;
        i = m_idHashValid; stream << i;
        i = m_valueHashValid; stream << i;
        i = m_hasArguments; stream << i;
        
        stream << m_idHash;
        stream << m_valueHash;
        stream << m_name;
        stream << m_body;
        stream << m_fileName;
        stream << m_argumentList;
    }
    
    /** Get the name for this macro */
    QString name() const {
      return m_name;
    }
    /** Set the name for this macro */
    void setName( const QString& name ) {
      m_name = name;
      invalidateHash();
    }

    /** Get the file name that contains this macro */
    QString fileName() const {
      return m_fileName;
    }
    /** Set the file name that contains this macro */
    void setFileName( const QString& fileName ) {
      m_fileName = fileName;
      invalidateHash();
    }

    /** Get the body of the macro */
    QString body() const {
      return m_body;
    }
    /** Set the body of the macro */
    void setBody( const QString& body ) {
      m_body = body;
      invalidateHash();
    }

    /** This is used so the lexer does not have to remove macros that should really stay(they are just temporarily shadowed by an isUndef-macro */
    bool isUndef() const {
      return m_isUndefMacro;
    };

    void setUndef() {
      m_isUndefMacro = true;
      invalidateHash();
    };

    /** Check whether the macro has arguments that are passed to it */
    bool hasArguments() const {
      return m_hasArguments;
    }
    void setHasArguments( bool hasArguments ) {
      m_hasArguments = hasArguments;
      invalidateHash();
    }
    /** Get a list of arguments passed to this macro */
    QValueList<Argument> argumentList() const {
      return m_argumentList;
    }

    /** Clear the list of arguments this macro has */
    void clearArgumentList() {
      m_argumentList.clear();
      m_hasArguments = false;
      invalidateHash();
    }
    /** Add an argument to this macro */
    void addArgument( const Argument& argument ) {
      m_argumentList << argument;
    }
    /** Add a list of arguments to this macro */
    void addArgumentList( const QValueList<Argument>& arguments ) {
      m_argumentList += arguments;
      invalidateHash();
    }

    ///This hash respects macro-name and argument-count
    size_t idHash() const {
        if( !m_idHashValid ) computeHash();
        return m_idHash;
    }

    ///This hash respects body and if it is an undef-macro 
    size_t valueHash() const {
        if( !m_valueHashValid ) computeHash();
        return m_valueHash;
    }
    
  private:
    inline void invalidateHash() const {
        m_idHashValid = m_valueHashValid = false;
    }
    
    void computeHash() const {
        m_idHash = 7 * ( HashedString::hashString( m_name ) + m_argumentList.count() * 13 );
        int a = 1;
        m_idHash += 31 * m_argumentList.count();
    
        m_valueHash = 27 * ( HashedString::hashString( m_body ) +  (m_isUndefMacro ? 1 : 0 ) );

        for( QValueList<Argument>::const_iterator it = m_argumentList.begin(); it != m_argumentList.end(); ++it ) {
            a *= 19;
            m_valueHash += a * HashedString::hashString( *it );
        }
        m_valueHashValid = true;
        m_idHashValid = true;
    }
      
    mutable bool m_idHashValid;
    mutable bool m_valueHashValid;
    mutable size_t m_idHash; //Hash that represents the ids of all macros
    mutable size_t m_valueHash; //Hash that represents the values of all macros

    QString m_name;
    QString m_fileName;
    QString m_body;
    bool m_hasArguments;
    QStringList m_argumentList; //While identification, only the count plays a role, not the values.
    bool m_isUndefMacro;
    friend class NameCompare;
    friend class NameArgEqual;
};

///NOTE: Currently #ifdef, #ifndef, etc. do not create a usingMacro-call. Think about whether it should be done.

class MacroSet {
    public:
        //typedef __gnu_cxx::hash_set< Macro, Macro::NameArgHash, Macro::NameArgEqual > Macros;
        typedef std::set< Macro, Macro::NameArgCompare > Macros;
        MacroSet() : m_idHashValid( false ), m_valueHashValid( false ) {
        }

        void addMacro( const Macro& macro, int line, int column );

        void read( QDataStream& stream )  {
            //stream >> m_idHashValid >> m_idHash >> m_valueHashValid >> m_valueHash;
            m_idHashValid = false;
            m_valueHashValid = false;
            int cnt;
            stream >> cnt;
            m_usedMacros.clear();
            Macro m;
            for( int a = 0; a < cnt; a++ ) {
                m.read( stream );
                m_usedMacros.insert( m );
            }
        }

        void write( QDataStream& stream ) const {
            //stream << m_idHashValid << m_idHash << m_valueHashValid << m_valueHash;
            stream << m_usedMacros.size();
            for( Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it ) {
                (*it).write( stream );
            }
        }
        
        size_t idHash() const;
        size_t valueHash() const;
    private:
        void computeHash() const;
        Macros m_usedMacros;
        mutable bool m_idHashValid;
        mutable bool m_valueHashValid;
        mutable size_t m_idHash; //Hash that represents the ids of all macros
        mutable size_t m_valueHash; //Hash that represents the values of all macros
};

class ParsedFile;
typedef KSharedPtr< ParsedFile > ParsedFilePointer;

class ParsedFile : public AbstractParseResult {
  public:
      struct IncludeDesc {
          bool local; //Whether it is a local include(#include "local.h", not #include <global.h>)
          QString includePath;
          ParsedFilePointer parsed; //May be zero!
      };
    ParsedFile() {
    }

    ParsedFile( const QString& fileName, const QDateTime& timeStamp );
    
    ///Deserializes the ParsedFile from a previous call to serialize(). AST will always be zero after a call to this.
    ParsedFile( const QByteArray& array );

    /**
     * @return All Macros that were used while processing this translation-unit. May be modified.
     */
    MacroSet& usedMacros();

    /**
     * @return Absolutely all files included by this one(no matter through how many other files they were included)
     */
    HashedStringSet& includeFiles();
    
    const HashedStringSet& includeFiles() const;

    void addIncludeFile( const QString& includePath, const ParsedFilePointer& parsed, bool localInclude );
    
    /**
     * @return Reference to the internal list of all directly included files(without those included indirectly)
     */
    const QValueList<IncludeDesc>& directIncludeFiles() const;

    operator TranslationUnitAST* () const; //May be zero!

    TranslationUnitAST* operator -> () const {
        if( !this ) return 0;
        return m_translationUnit;
    }
    
    void setTranslationUnit( const TranslationUnitAST::Node& trans );

    QString fileName() const;

    QDateTime timeStamp() const;

    ///Serializes the content of this class into a byte-array. Note that this does not serialize the AST.
    QByteArray serialize() const;

    /*void read( QDataStream& stream );
    void write( QDataStream& stream ) const;*/
    
    virtual void read( QDataStream& stream ) {
        int directIncludeFilesCount;
        stream >> directIncludeFilesCount;
        m_directIncludeFiles.clear();
        for( int a = 0; a < directIncludeFilesCount; a++ ) {
            IncludeDesc i;
            Q_INT8 in;
            stream >> in;
            i.local = in;
            stream >> i.includePath;
        //"parsed" will not be reconstructed
            m_directIncludeFiles.push_back( i );
        }
        stream >> m_fileName;
        stream >> m_timeStamp;
        m_usedMacros.read( stream );
        m_translationUnit = 0;
        m_includeFiles.read( stream );
    }

    virtual void write( QDataStream& stream ) const {
      int i = m_directIncludeFiles.size();
      stream << i;
        for( QValueList<IncludeDesc>::const_iterator it = m_directIncludeFiles.begin(); it != m_directIncludeFiles.end(); ++it ) {
            Q_INT8 i = (*it).local;
            stream << i;
            stream << (*it).includePath;
        }
        stream << m_fileName;
        stream << m_timeStamp;
        m_usedMacros.write( stream );
        m_includeFiles.write( stream );
    }

    virtual ParsedFileType type() const {
      return CppParsedFile;
    }

    private:
    QValueList<IncludeDesc> m_directIncludeFiles;
    MacroSet m_usedMacros;
    TranslationUnitAST::Node m_translationUnit;
    HashedStringSet m_includeFiles;
    QString m_fileName;
    QDateTime m_timeStamp;
    
};

/**
 * An interface that provides source code to the Driver
 */
class SourceProvider {
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

class Driver {
  public:
    Driver();
    virtual ~Driver();

    typedef std::multimap< HashedString, Macro > MacroMap;

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
     * @param macrosGlobal Should the macros be global? (Global macros are not deleted once a new translation-unit is parsed)
     */
    virtual void parseFile( const QString& fileName, bool onlyPreProcesss = false, bool force = false, bool macrosGlobal = false );

    /**
     * Indicates that the file has been parsed
     * @param fileName The name of the file parsed. It is legal to create a ParsedFilePointer on the given item.
     */
    virtual void fileParsed( const ParsedFile& fileName );

    /**
     * Removes the file specified by @p fileName from the driver
     * @param fileName The name of the file to remove
     */
    virtual void remove
      ( const QString& fileName );

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
    QString currentFileName() const {
      return m_currentFileName;
    }
    ParsedFilePointer takeTranslationUnit( const QString& fileName );
    
    void takeTranslationUnit( const ParsedFile& file );
    /**
     * Get the translation unit contained in the driver for @p fileName.
     * @param fileName The name of the file to get the translation unit for
     * @return The TranslationUnitAST pointer that represents the translation unit
     * @return 0 if no translation unit exists for the file
     */
    ParsedFilePointer translationUnit( const QString& fileName ) const;
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
    MacroMap macros() const;
    /**
     * Get the list of problem areas the driver contains
     * @param fileName The filename to get problems for
     * @return The list of problems for @p fileName
     */
    QValueList<Problem> problems( const QString& fileName ) const;

    /**
     * Check if we have a macro in the driver
    * If the last stacked macro of that name is an undef-macro, false is returned.
     * @param name The name of the macro to check for
     * @return true if we have the macro in the driver
     * @return false if we don't have the macro in the driver
     */
    bool hasMacro( const HashedString& name ) const;
    /**
     * Get the macro identified by @p name
     * @param name The name of the macro to get
     * @return A const reference of the macro object represented by @p name
     */
    const Macro& macro( const HashedString& name ) const;
    /**
     * Get the last inserted macro identified by @p name
     * @override
     * @param name The name of the macro to get
     * @return A non-const reference of the macro object represented by @p name
     * 
     */
    Macro& macro( const HashedString& name );

    /**
     * Remove the last inserted Macro of that name
     * @param macroName The name of the macro to remove
     */
    virtual void removeMacro( const HashedString& macroName );

    /**
       * Remove all macros from the driver for a certain file
       * @param fileName The file name 
       */
    virtual void removeAllMacrosInFile( const QString& fileName ); ///Check when this is called. It may be wrong.

    QStringList includePaths() const {
      return m_includePaths;
    }

    virtual void updateIncludePath( const QString& file ) const;

  
    virtual void addIncludePath( const QString &path );

    virtual void clearIncludePaths();

    /// @todo remove
    const QMap<QString, ParsedFilePointer> &parsedUnits() const {
      return m_parsedUnits;
    }

    /**
     * Set whether or not to enable dependency resolving for files added to the driver
     */
    virtual void setResolveDependencesEnabled( bool enabled );
    /**
     * Check if dependency resolving is enabled
     * \return true if dependency resolving is enabled
     * \return false if dependency resolving is disabled
     */
    bool isResolveDependencesEnabled() const {
      return depresolv;
    }

    void setMaxDependenceDepth( int depth );

    /**
     * Used by the Lexer to indicate that a Macro was used
     * @param macro The used macro
     * */
    void usingMacro( const Macro& macro, int line, int column );

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

    /**
     * Is code-information for this file already available? If false is returned, the file will be parsed.
     * Code-model and static repository should be checked to find out whether the file is already available.
     * This function is only used when dependency-resolving is activated.
     * @arg file absolute path to the file
     */
    virtual bool shouldParseFile( const ParsedFilePointer& /*file*/ ) {
      return true;
    }

    void clearMacros();
    
    void clearParsedMacros();
    
    private:
    QMap<QString, Dependence>& findOrInsertDependenceList( const QString& fileName );
    QValueList<Problem>& findOrInsertProblemList( const QString& fileName );
    QString findIncludeFile( const Dependence& dep ) const;

    
  private:
    QString m_currentFileName;
    QString m_currentMasterFileName;
    typedef QMap<QString, Dependence> DependenceMap;
    typedef QMap< QString, DependenceMap> DependencesMap;
    DependencesMap m_dependences;
    MacroMap m_macros;
    QMap< QString, QValueList<Problem> > m_problems;
    QMap<QString, ParsedFilePointer> m_parsedUnits;
    QStringList m_includePaths;
    uint depresolv :
    1;
    Lexer *lexer;
    SourceProvider* m_sourceProvider;

    ParsedFilePointer m_currentParsedFile;

    int m_dependenceDepth;
    int m_maxDependenceDepth;
    
    class ParseHelper;
    friend class ParseHelper;

  private:
    Driver( const Driver& source );
    void operator = ( const Driver& source );
};

#endif
