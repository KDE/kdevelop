/* This file is part of KDevelop
   Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2006 David Nolden <david.nolden.kdevelop@art-master.de>

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

#define CACHELEXER

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include <kdebug.h>
#include <klocale.h>
#include <stdlib.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <assert.h>
    

//     void Macro::read( QDataStream& stream ) {
//         stream >> m_idHashValid;
//         stream >> m_valueHashValid;
//         stream >> m_idHash;
//         stream >> m_valueHash;
//         stream >> m_name;
//         stream >> m_body;
//         stream >> m_fileName;
//         stream >> m_hasArguments;
//         stream >> m_argumentList;
//     }
//     
//     void Macro::write( QDataStream& stream ) const {
//         stream << m_idHashValid;
//         stream << m_valueHashValid;
//         stream << m_idHash;
//         stream << m_valueHash;
//         stream << m_name;
//         stream << m_body;
//         stream << m_fileName;
//         stream << m_hasArguments;
//         stream << m_argumentList;
//     }


class IntIncreaser {
    public:
    IntIncreaser( int& i ) : m_i( i ) {
        ++m_i;
    }
    ~IntIncreaser() {
        --m_i;
    }
    private:
    int& m_i;
};

class DefaultSourceProvider: public SourceProvider {
  public:
    DefaultSourceProvider() {}

    virtual QString contents( const QString& fileName ) {
      QString source;

      QFile f( fileName );
      if ( f.open( IO_ReadOnly ) ) {
        QTextStream s( &f );
        source = s.read();
        f.close();
      }
      return source;
    }

    virtual bool isModified( const QString& fileName ) {
      Q_UNUSED( fileName );
      return true;
    }

  private:
    DefaultSourceProvider( const DefaultSourceProvider& source );
    void operator = ( const DefaultSourceProvider& source );
};


Driver::Driver()
  : depresolv( FALSE ), lexer( 0 ), m_lexerCache( this ), m_dependenceDepth( 0 ), m_maxDependenceDepth( 20 ) {
  m_sourceProvider = new DefaultSourceProvider();
}

Driver::~Driver() {
  reset();
  delete m_sourceProvider;
}

void Driver::setMaxDependenceDepth( int depth ) {
    m_maxDependenceDepth = depth;
}

SourceProvider* Driver::sourceProvider() {
  return m_sourceProvider;
}

void Driver::setSourceProvider( SourceProvider* sourceProvider ) {
  delete m_sourceProvider;
  m_sourceProvider = sourceProvider;
}

void Driver::reset( ) {
  m_lexerCache.clear();
  m_dependences.clear();
  m_macros.clear();
  m_problems.clear();
  m_includePaths.clear();

  while ( m_parsedUnits.size() ) {
    //TranslationUnitAST* unit = **m_parsedUnits.begin();
    m_parsedUnits.remove( m_parsedUnits.begin() );
    //delete( unit );
  }
}

void Driver::updateIncludePath( const QString& file ) const {
}

void Driver::remove
  ( const QString & fileName ) {
  m_dependences.remove( fileName );
  m_problems.remove( fileName );
    if( !isResolveDependencesEnabled() )
      removeAllMacrosInFile( fileName );

  QMap<QString, ParsedFilePointer>::Iterator it = m_parsedUnits.find( fileName );
  if ( it != m_parsedUnits.end() ) {
    //TranslationUnitAST * unit = **it;
    m_parsedUnits.remove( it );
    //delete( unit );
  }
}

void Driver::removeAllMacrosInFile( const QString& fileName ) {
  MacroMap::iterator it = m_macros.begin();
  while ( it != m_macros.end() ) {
    Macro m = ( *it ).second;
    if ( m.fileName() == fileName ) {
      m_macros.erase( it++ );
    } else {
      ++it;
    }
  }
}

void Driver::usingString( const HashedString& str ) {
  #ifdef CACHELEXER
  if( m_currentLexerCache ) {
    m_currentLexerCache->addString( m_lexerCache.unifyString( str ) );
  }
  #endif
}

bool Driver::hasMacro( const HashedString& name ) {
  std::pair< MacroMap::const_iterator, MacroMap::const_iterator > range = m_macros.equal_range( name );
  if ( range.first == range.second ) {
    return false;
  } else {
    const Macro& m( ( *( --range.second ) ).second );
    if ( m.isUndef() )
      return false;
    else
      return true;
  }
  return false;
}

QString deepCopy( const QString& str ) {
    return str;
    //return str.ascii();
}

const Macro& Driver::macro( const HashedString& name ) const {
  std::pair< MacroMap::const_iterator, MacroMap::const_iterator > range = m_macros.equal_range( name );
  if ( range.first == range.second ) {
      return ( *const_cast<MacroMap&>( m_macros ).insert( std::make_pair( deepCopy( name.str() ), Macro() ) ) ).second;  ///Since we need to return a reference, there's no other way.
  } else {
    return ( *( --range.second ) ).second;
  }
}
Macro& Driver::macro( const HashedString& name ) {
  std::pair< MacroMap::iterator, MacroMap::iterator > range = m_macros.equal_range( name );
  if ( range.first == range.second ) {
      return ( *m_macros.insert( std::make_pair( deepCopy( name.str() ), Macro() ) ) ).second;
  } else {
    return ( *( --range.second ) ).second;
  }
}

void Driver::addMacro( const Macro & macro ) {
  std::pair< MacroMap::iterator, MacroMap::iterator > range = m_macros.equal_range( macro.name() );

  if ( range.first == range.second ) {
      m_macros.insert( std::make_pair( deepCopy( macro.name() ), macro ) );
  } else {
    ///Insert behind the other macros
      m_macros.insert( range.second, std::make_pair( deepCopy( macro.name() ), macro ) );
    Macro cp = this->macro( macro.name() );
    assert( macro == cp );
  }

#ifdef CACHELEXER
  if( m_currentLexerCache )
    m_currentLexerCache->addDefinedMacro( macro );
#endif
}

void Driver::removeMacro( const HashedString& macroName ) {
  std::pair< MacroMap::iterator, MacroMap::iterator > range = m_macros.equal_range( macroName );
  if ( range.first != range.second ) {
    m_macros.erase( --range.second );
  }
}

ParsedFilePointer Driver::takeTranslationUnit( const QString& fileName ) {
  QMap<QString, ParsedFilePointer>::Iterator it = m_parsedUnits.find( fileName );
  ParsedFilePointer unit( *it );
  //m_parsedUnits.remove( it );
  m_parsedUnits[ fileName ] = 0;
  return unit;
}

void Driver::takeTranslationUnit( const ParsedFile& file ) {
    QMap<QString, ParsedFilePointer>::Iterator it = m_parsedUnits.find( file.fileName() );
    m_parsedUnits[ file.fileName() ] = 0;
}

ParsedFilePointer Driver::translationUnit( const QString& fileName ) const {
  QMap<QString, ParsedFilePointer>::ConstIterator it = m_parsedUnits.find( fileName );
  return it != m_parsedUnits.end() ? *it : 0;
}

class Driver::ParseHelper {
  public:
    ParseHelper( const QString& fileName, bool force, Driver* driver, bool reportMessages = true ) : m_wasReset( false ), m_fileName( fileName ), m_previousFileName( driver->m_currentFileName ),  m_previousLexer( driver->lexer ), m_previousParsedFile( driver->m_currentParsedFile ), m_previousCachedLexedFile( driver->m_currentLexerCache ), m_force( force ), m_driver( driver ), m_lex( m_driver ) {
      QFileInfo fileInfo( fileName );
      m_driver->m_currentParsedFile = new ParsedFile( fileName, fileInfo.lastModified() );
#ifdef CACHELEXER
      m_driver->m_currentLexerCache = new CachedLexedFile( fileName, &m_driver->m_lexerCache );
#endif
      m_absFilePath = fileInfo.absFilePath();

      QMap<QString, ParsedFilePointer>::Iterator it = m_driver->m_parsedUnits.find( m_absFilePath );

      if ( force && it != m_driver->m_parsedUnits.end() ) {
        m_driver->takeTranslationUnit( m_absFilePath );
      } else if ( it != m_driver->m_parsedUnits.end() && *it != 0 ) {
        // file already processed
        return ;
      }

      CachedLexedFilePointer lexedFileP = m_driver->m_lexerCache.lexedFile(  HashedString( fileName ) );
      
      m_driver->m_dependences.remove( fileName );
      m_driver->m_problems.remove( fileName );

      driver->m_currentFileName = fileName;

      m_driver->lexer = &m_lex;
      m_driver->setupLexer( &m_lex );

      m_lex.setReportMessages( reportMessages );
      
      kdDebug( 9007 ) << "lexing file " << fileName << endl;
      m_lex.setSource( m_driver->sourceProvider() ->contents( fileName ) );
      if(m_previousCachedLexedFile)
        m_previousCachedLexedFile->merge( *m_driver->m_currentLexerCache );
      else
        m_driver->findOrInsertProblemList( m_driver->m_currentMasterFileName )  += m_driver->m_currentLexerCache->problems();
      
      if( !lexedFileP && m_previousParsedFile ) //only add the new cache-instance if a fitting isn't already stored, and if this file was included by another one.
        m_driver->m_lexerCache.addLexedFile( m_driver->m_currentLexerCache );
      }

  
    void parse() {
      CachedLexedFilePointer lf = m_driver->m_currentLexerCache; //Set the lexer-cache to zero, so the problems registered through addProblem go directly into the file
      m_driver->m_currentLexerCache = 0;
      
      Parser parser( m_driver, m_driver->lexer );
      m_driver->setupParser( &parser );

      TranslationUnitAST::Node unit;
      parser.parseTranslationUnit( unit );
      m_driver->m_currentParsedFile->setTranslationUnit( unit );
      m_driver->m_parsedUnits.insert( m_fileName, m_driver->m_currentParsedFile );
      m_driver->fileParsed( *m_driver->m_currentParsedFile );

      m_driver->m_currentLexerCache = lf;
    }

    ParsedFilePointer parsedFile() const {
        return m_driver->m_currentParsedFile;
    }

    void reset() {
      if ( !m_wasReset ) {
        m_driver->m_currentFileName = m_previousFileName;
        m_driver->lexer = m_previousLexer;
        m_driver->m_currentParsedFile = m_previousParsedFile;
        m_driver->m_currentLexerCache = m_previousCachedLexedFile;
        if( m_driver->m_currentLexerCache == 0 ) {

        }
          
        m_wasReset = true;
      }
    }

    ~ParseHelper() {
      reset();
    }


  private:
    bool m_wasReset;
    QString m_fileName;
    QString m_absFilePath;
    QString m_previousFileName;
    Lexer* m_previousLexer;
    ParsedFilePointer m_previousParsedFile;
    CachedLexedFilePointer m_previousCachedLexedFile;
    bool m_force;
    Driver* m_driver;
    Lexer m_lex;
};


void Driver::addDependence( const QString & fileName, const Dependence & dep ) {
  //@todo prevent cyclic dependency-loops
  QFileInfo fileInfo( dep.first );
  QString fn = fileInfo.absFilePath();
    
#ifdef LEXERCACHE
      if( m_currentLexerCache )
        m_currentLexerCache->addIncludeFile( file, QDateTime() ); ///The time will be overwritten in CachedLexedFile::merge(...)
    #endif

  if ( !depresolv ) {
    findOrInsertDependenceList( fileName ).insert( fn, dep );
    m_currentParsedFile->addIncludeFile( dep.first, 0, dep.second == Dep_Local );
    return ;
  }

  QString file = findIncludeFile( dep );
  findOrInsertDependenceList( fileName ).insert( file, dep );
  m_currentParsedFile->addIncludeFile( file, 0, dep.second == Dep_Local );
  
  if ( !QFile::exists( file ) ) {
    Problem p( i18n( "Could not find include file %1" ).arg( dep.first ),
               lexer ? lexer->currentLine() : -1,
               lexer ? lexer->currentColumn() : -1, Problem::Level_Warning );
    addProblem( fileName, p );
    return ;
  }

  /**What should be done:
   * 1. Lex the file to get all the macros etc.
   * 2. TODO: Check what previously set macros the file was affected by, and compare those macros to any previously parsed instances of this file.
   *  2.1 If there is a parse-instance of the file where all macros that played a role had the same values, we do not need to reparse this file.
   *  2.2 If there is no such fitting instance, the file needs to be parsed and put to the code-model.
   *
   * It'll be necessary to have multiple versions of one file in the code-model.
   */

  IntIncreaser i( m_dependenceDepth );
  if( m_dependenceDepth > m_maxDependenceDepth ) {
      kdDebug( 9007 ) << "maximum dependence-depth of " << m_maxDependenceDepth << " was reached, " << fileName << " will not be processed" << endl;
      return;
  }

  CachedLexedFilePointer lexedFileP = m_lexerCache.lexedFile(  HashedString( file ) );
  if( lexedFileP ) {
    CachedLexedFile& lexedFile( *lexedFileP );
    m_currentLexerCache->merge( lexedFile );
    for( MacroSet::Macros::const_iterator it = lexedFile.definedMacros().macros().begin(); it != lexedFile.definedMacros().macros().end(); ++it ) {
      addMacro( (*it) );
    }
    ///@todo fill usingMacro(...)
    return;
  }

  ParseHelper h( file, true, this, false );

  /*if ( m_parsedUnits.find(file) != m_parsedUnits.end() )
  	return;*/

  //h.parse();
}

void Driver::addProblem( const QString & fileName, const Problem & problem ) {
  Problem p( problem );
  p.setFileName( fileName );
  
  if( m_currentLexerCache )
    m_currentLexerCache->addProblem( p );
  else
    findOrInsertProblemList( m_currentMasterFileName ).append( problem );
}

QMap< QString, Dependence >& Driver::findOrInsertDependenceList( const QString & fileName ) {
  QMap<QString, QMap<QString, Dependence> >::Iterator it = m_dependences.find( fileName );
  if ( it != m_dependences.end() )
    return it.data();

  QMap<QString, Dependence> l;
  m_dependences.insert( deepCopy( fileName ), l );
  return m_dependences[ fileName ];
}

QValueList < Problem >& Driver::findOrInsertProblemList( const QString & fileName ) {
  QMap<QString, QValueList<Problem> >::Iterator it = m_problems.find( fileName );
  if ( it != m_problems.end() )
    return it.data();

  QValueList<Problem> l;
  m_problems.insert( fileName, l );
  return m_problems[ fileName ];
}

QMap< QString, Dependence > Driver::dependences( const QString & fileName ) const {
  QMap<QString, QMap<QString, Dependence> >::ConstIterator it = m_dependences.find( fileName );
  if ( it != m_dependences.end() )
    return it.data();
  return QMap<QString, Dependence>();
}

const Driver::MacroMap& Driver::macros() const {
  return m_macros;
}

void Driver::insertMacros( const MacroSet& macros ) {
    for( MacroSet::Macros::const_iterator it = macros.m_usedMacros.begin(); it != macros.m_usedMacros.end(); ++it ) {
        addMacro( *it );
    }
}

QValueList < Problem > Driver::problems( const QString & fileName ) const {
  QMap<QString, QValueList<Problem> >::ConstIterator it = m_problems.find( fileName );
  if ( it != m_problems.end() )
    return it.data();
  return QValueList<Problem>();
}

void Driver::clearMacros() {
    m_macros.clear();
}

void Driver::clearParsedMacros() {
    //Keep global macros
    for( MacroMap::iterator it = m_macros.begin(); it != m_macros.end(); ) {
        if( !(*it).second.fileName().isEmpty() ) {
            m_macros.erase( it++ );
        } else {
            ++it;
        }
    }
}

void Driver::parseFile( const QString& fileName, bool onlyPreProcess, bool force , bool macrosGlobal ) {
   //if( isResolveDependencesEnabled() )
    clearParsedMacros(); ///Since everything will be re-lexed, we do not need any old macros

    m_lexerCache.increaseFrame();
  m_currentMasterFileName = fileName;
  
  updateIncludePath( fileName );
  
    ParseHelper p( fileName, force, this );
    if( !onlyPreProcess ){
        p.parse();
    }
    if( macrosGlobal ) {
        for( MacroMap::iterator it = m_macros.begin(); it != m_macros.end(); ++it) {
            if( (*it).second.fileName() == fileName ) {
                (*it).second.setFileName( QString::null );
            }
    }
}
}

void Driver::setupLexer( Lexer * lexer ) {
  // stl
  lexer->addSkipWord( "__STL_BEGIN_NAMESPACE" );
  lexer->addSkipWord( "__STL_END_NAMESPACE" );
  lexer->addSkipWord( "__STL_BEGIN_RELOPS_NAMESPACE" );
  lexer->addSkipWord( "__STL_END_RELOPS_NAMESPACE" );
  lexer->addSkipWord( "__STL_TEMPLATE_NULL" );
  lexer->addSkipWord( "__STL_TRY" );
  lexer->addSkipWord( "__STL_UNWIND" );
  lexer->addSkipWord( "__STL_NOTHROW" );
  lexer->addSkipWord( "__STL_NULL_TMPL_ARGS" );
  lexer->addSkipWord( "__STL_UNWIND", SkipWordAndArguments );
  lexer->addSkipWord( "__GC_CONST" );
  lexer->addSkipWord( "__HASH_ALLOC_INIT", SkipWordAndArguments );
  lexer->addSkipWord( "__STL_DEFAULT_ALLOCATOR", SkipWordAndArguments, "T" );
  lexer->addSkipWord( "__STL_MUTEX_INITIALIZER" );
  lexer->addSkipWord( "__STL_NULL_TMPL_ARGS" );

  // antlr
  lexer->addSkipWord( "ANTLR_BEGIN_NAMESPACE", SkipWordAndArguments );
  lexer->addSkipWord( "ANTLR_USE_NAMESPACE", SkipWordAndArguments );
  lexer->addSkipWord( "ANTLR_USING_NAMESPACE", SkipWordAndArguments );
  lexer->addSkipWord( "ANTLR_END_NAMESPACE" );
  lexer->addSkipWord( "ANTLR_C_USING", SkipWordAndArguments );
  lexer->addSkipWord( "ANTLR_API" );

  // gnu
  lexer->addSkipWord( "__extension__", SkipWordAndArguments );
  lexer->addSkipWord( "__attribute__", SkipWordAndArguments );
  lexer->addSkipWord( "__BEGIN_DECLS" );
  lexer->addSkipWord( "__END_DECLS" );
  lexer->addSkipWord( "__THROW" );
  lexer->addSkipWord( "__restrict" );
  lexer->addSkipWord( "__restrict__" );
  lexer->addSkipWord( "__attribute_pure__" );
  lexer->addSkipWord( "__attribute_malloc__" );
  lexer->addSkipWord( "__attribute_format_strfmon__" );
  lexer->addSkipWord( "__asm__", SkipWordAndArguments );
  lexer->addSkipWord( "__devinit" );
  lexer->addSkipWord( "__devinit__" );
  lexer->addSkipWord( "__init" );
  lexer->addSkipWord( "__init__" );
  lexer->addSkipWord( "__signed" );
  lexer->addSkipWord( "__signed__" );
  lexer->addSkipWord( "__unsigned" );
  lexer->addSkipWord( "__unsigned__" );
  lexer->addSkipWord( "asmlinkage" );
  lexer->addSkipWord( "____cacheline_aligned" );
  lexer->addSkipWord( "__glibcpp_class_requires", SkipWordAndArguments );
  lexer->addSkipWord( "__glibcpp_class2_requires", SkipWordAndArguments );
  lexer->addSkipWord( "__glibcpp_class4_requires", SkipWordAndArguments );
  lexer->addSkipWord( "__glibcpp_function_requires", SkipWordAndArguments );
  lexer->addSkipWord( "restrict" );

  lexer->addSkipWord( "__BEGIN_NAMESPACE_STD" );
  lexer->addSkipWord( "__END_NAMESPACE_STD" );
  lexer->addSkipWord( "__BEGIN_NAMESPACE_C99" );
  lexer->addSkipWord( "__END_NAMESPACE_C99" );
  lexer->addSkipWord( "__USING_NAMESPACE_STD", SkipWordAndArguments );

  // kde
  lexer->addSkipWord( "K_SYCOCATYPE", SkipWordAndArguments );
  lexer->addSkipWord( "EXPORT_DOCKCLASS" );
  lexer->addSkipWord( "K_EXPORT_COMPONENT_FACTORY", SkipWordAndArguments );
  lexer->addSkipWord( "K_SYCOCAFACTORY", SkipWordAndArguments );
  lexer->addSkipWord( "KDE_DEPRECATED" );

  // qt
  lexer->addSkipWord( "Q_OBJECT" );
  lexer->addSkipWord( "Q_OVERRIDE", SkipWordAndArguments );
  lexer->addSkipWord( "Q_ENUMS", SkipWordAndArguments );
  lexer->addSkipWord( "Q_PROPERTY", SkipWordAndArguments );
  lexer->addSkipWord( "Q_CLASSINFO", SkipWordAndArguments );
  lexer->addSkipWord( "Q_SETS", SkipWordAndArguments );
  lexer->addSkipWord( "Q_UNUSED", SkipWordAndArguments );
  lexer->addSkipWord( "Q_CREATE_INSTANCE", SkipWordAndArguments );
  lexer->addSkipWord( "Q_DUMMY_COMPARISON_OPERATOR", SkipWordAndArguments );
  lexer->addSkipWord( "ACTIVATE_SIGNAL_WITH_PARAM", SkipWordAndArguments );
  lexer->addSkipWord( "Q_INLINE_TEMPLATES" );
  lexer->addSkipWord( "Q_TEMPLATE_EXTERN" );
  lexer->addSkipWord( "Q_TYPENAME" );
  lexer->addSkipWord( "Q_REFCOUNT" );
  lexer->addSkipWord( "Q_EXPLICIT" );
  lexer->addSkipWord( "QMAC_PASCAL" );
  lexer->addSkipWord( "QT_STATIC_CONST" );
  lexer->addSkipWord( "QT_STATIC_CONST_IMPL" );
  lexer->addSkipWord( "QT_WIN_PAINTER_MEMBERS" );
  lexer->addSkipWord( "QT_NC_MSGBOX" );
  lexer->addSkipWord( "Q_VARIANT_AS", SkipWordAndArguments );
  lexer->addSkipWord( "CALLBACK_CALL_TYPE" );

  // qt4 [erbsland]
  lexer->addSkipWord( "Q_DECLARE_FLAGS", SkipWordAndArguments );
  lexer->addSkipWord( "Q_DECLARE_OPERATORS_FOR_FLAGS", SkipWordAndArguments );

  // flex
  lexer->addSkipWord( "yyconst" );
  lexer->addSkipWord( "YY_RULE_SETUP" );
  lexer->addSkipWord( "YY_BREAK" );
  lexer->addSkipWord( "YY_RESTORE_YY_MORE_OFFSET" );

  // gtk
  lexer->addSkipWord( "G_BEGIN_DECLS" );
  lexer->addSkipWord( "G_END_DECLS" );
  lexer->addSkipWord( "G_GNUC_CONST" );
  lexer->addSkipWord( "G_CONST_RETURN" );
  lexer->addSkipWord( "GTKMAIN_C_VAR" );
  lexer->addSkipWord( "GTKVAR" );
  lexer->addSkipWord( "GDKVAR" );
  lexer->addSkipWord( "G_GNUC_PRINTF", SkipWordAndArguments );

  // windows
  lexer->addSkipWord( "WINAPI" );
  lexer->addSkipWord( "__stdcall" );
  lexer->addSkipWord( "__cdecl" );
  lexer->addSkipWord( "_cdecl" );
  lexer->addSkipWord( "CALLBACK" );

  // gcc extensions
  if( !hasMacro( "__asm__" ) ) addMacro( Macro( "__asm__", "asm" ) );
  if( !hasMacro( "__inline" ) ) addMacro( Macro( "__inline", "inline" ) );
  if( !hasMacro( "__inline__" ) ) addMacro( Macro( "__inline__", "inline" ) );
  if( !hasMacro( "__const" ) ) addMacro( Macro( "__const", "const" ) );
  if( !hasMacro( "__const__" ) ) addMacro( Macro( "__const__", "const" ) );
  if( !hasMacro( "__volatile__" ) ) addMacro( Macro( "__volatile__", "volatile" ) );
  if( !hasMacro( "__complex__" ) ) addMacro( Macro( "__complex__", "" ) );
}

void Driver::setupParser( Parser * parser ) {
  Q_UNUSED( parser );
}

void Driver::clearIncludePaths() {
    m_includePaths.clear();
}

void Driver::addIncludePath( const QString &path ) {
  if ( !path.stripWhiteSpace().isEmpty() )
    m_includePaths << path;
}

QString Driver::findIncludeFile( const Dependence& dep ) const {
  QString fileName = dep.first;

  if ( dep.second == Dep_Local ) {
    QString path = QFileInfo( currentFileName() ).dirPath( true );
    QFileInfo fileInfo( QFileInfo( path, fileName ) );
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

void Driver::setResolveDependencesEnabled( bool enabled ) {
  depresolv = enabled;
  if ( depresolv )
    setupPreProcessor();
}

void Driver::setupPreProcessor() {}

void Driver::fileParsed( const ParsedFile & fileName ) {
  Q_UNUSED( fileName );
}

void Driver::usingMacro( const Macro& macro, int line, int column ) {
    if( m_currentParsedFile )
        m_currentParsedFile->usedMacros().addMacro( macro, line, column );
#ifdef CACHELEXER
    if( m_currentLexerCache )
      m_currentLexerCache->addUsedMacro( macro, line, column );
#endif
}

// void Macro::computeHash() const {
//     m_idHash = 7 * ( HashedString::hashString( m_name ) + m_argumentList.count() * 13 );
//     int a = 1;
//     m_idHash += 31 * m_argumentList.count();
//     
//     m_valueHash = 27 * ( HashedString::hashString( m_body ) +  (m_isUndefMacro ? 1 : 0 ) );
// 
//     for( QValueList<Argument>::const_iterator it = m_argumentList.begin(); it != m_argumentList.end(); ++it ) {
//         a *= 19;
//         m_valueHash += a * HashedString::hashString( *it );
//     }
//     m_valueHashValid = true;
//     m_idHashValid = true;
// }

// MacroSet::MacroSet() : m_idHashValid( false ), m_valueHashValid( false ) {
// }

void MacroSet::addMacro( const Macro& macro, int line, int column ) {
  std::pair<Macros::const_iterator, bool> r = m_usedMacros.insert( macro );
  if( !r.second ) {
    //Make sure the macro added later will be used
    m_usedMacros.erase( r.first );
    m_usedMacros.insert( macro );
  }
    
  m_idHashValid = m_valueHashValid = false;
}

void MacroSet::merge( const MacroSet& macros ) {
  Macros m = macros.m_usedMacros; //Swap is needed so the merged macros take precedence
  m.insert( m_usedMacros.begin(), m_usedMacros.end() ); 
  m_usedMacros = m;
  m_idHashValid = m_valueHashValid = false;
}


size_t MacroSet::idHash() const {
    if( !m_idHashValid ) computeHash();
    return m_idHash;
}

size_t MacroSet::valueHash() const {
    if( !m_valueHashValid ) computeHash();
    return m_valueHash;
}

void MacroSet::computeHash() const {
    m_idHash = 0;
    m_valueHash = 0;
    int mult = 1;
    for( Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it ) {
        mult *= 31;
        m_idHash += (*it).idHash();
        m_valueHash += (*it).valueHash();
    }
}

// void MacroSet::read( QDataStream& stream )  {
//     stream >> m_idHashValid >> m_idHash >> m_valueHashValid >> m_valueHash;
//     int cnt;
//     stream >> cnt;
//     m_usedMacros.clear();
//     Macro m;
//     for( int a = 0; a < cnt; a++ ) {
//         m.read( stream );
//         m_usedMacros.insert( m );
//     }
// }
// 
// void MacroSet::write( QDataStream& stream ) const {
//     stream << m_idHashValid << m_idHash << m_valueHashValid << m_valueHash;
//     stream << m_usedMacros.size();
//     for( Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it ) {
//         (*it).write( stream );
//     }
// }

/**
 * @return All Macros that were used while processing this translation-unit
 * */
MacroSet& ParsedFile::usedMacros() {
  return m_usedMacros;
}

ParsedFile::ParsedFile( const QString& fileName, const QDateTime& timeStamp ) : m_translationUnit( 0 ), m_fileName( fileName ), m_timeStamp( timeStamp ) {
    m_includeFiles.insert( fileName );
}

ParsedFile::ParsedFile( const QByteArray& array ) {
    QBuffer b( array );
    QDataStream d( &b );
    read( d );
}

QByteArray ParsedFile::serialize() const {
    QByteArray array;
    QBuffer b( array );
    QDataStream d( &b );
    write( d );
    return array;
}

// void ParsedFile::read( QDataStream& stream ) {
//     int directIncludeFilesCount;
//     stream >> directIncludeFilesCount;
//     m_directIncludeFiles.clear();
//     for( int a = 0; a < directIncludeFilesCount; a++ ) {
//         IncludeDesc i;
//         stream >> i.local;
//         stream >> i.includePath;
//         //"parsed" will not be reconstructed
//         m_directIncludeFiles.push_back( i );
//     }
//     stream >> m_fileName;
//     stream >> m_timeStamp;
//     m_usedMacros.read( stream );
//     m_translationUnit = 0;
//     m_includeFiles.read( stream );
// }
// 
// void ParsedFile::write( QDataStream& stream ) const {
//     for( QValueList<IncludeDesc>::const_iterator it = m_directIncludeFiles.begin(); it != m_directIncludeFiles.end(); ++it ) {
//         stream << (*it).local;
//         stream << (*it).includePath;
//     }
//     stream << m_fileName;
//     stream << m_timeStamp;
//     m_usedMacros.write( stream );
//     m_includeFiles.write( stream );
// }

ParsedFile::operator TranslationUnitAST* () const {
  if( !this ) return 0;
  return m_translationUnit;
}

void ParsedFile::setTranslationUnit( const TranslationUnitAST::Node& trans ) {
  m_translationUnit = trans;
}

// HashedStringSet& ParsedFile::includeFiles() {
//     return m_includeFiles;
// }

const HashedStringSet& ParsedFile::includeFiles() const {
    return m_includeFiles;
}

QString ParsedFile::fileName() const {
    return m_fileName;
}

QDateTime ParsedFile::timeStamp() const {
    return m_timeStamp;
}

void ParsedFile::addIncludeFile( const QString& includePath, const ParsedFilePointer& parsed, bool localInclude ) {
    m_includeFiles.insert( includePath );
    if( parsed )
        m_includeFiles += parsed->includeFiles();
    IncludeDesc d;
    d.local = localInclude;
    d.includePath = includePath;
    d.parsed = parsed;
    m_directIncludeFiles << d;
}

const QValueList<ParsedFile::IncludeDesc>& ParsedFile::directIncludeFiles() const {
    return m_directIncludeFiles;
}

bool MacroSet::hasMacro( const QString& name ) const {
    Macros::const_iterator it = m_usedMacros.find( Macro( name, "" ) );
    if( it != m_usedMacros.end() ) {
        return true;
    } else {
        return false;
    }
}

bool MacroSet::hasMacro( const HashedString& name ) const {
    Macros::const_iterator it = m_usedMacros.find( Macro( name.str(), "" ) );
    if( it != m_usedMacros.end() ) {
        return true;
    } else {
        return false;
    }
}

Macro MacroSet::macro( const QString& name ) const {
    Macros::const_iterator it = m_usedMacros.find( Macro( name, "" ) );
    
    if( it != m_usedMacros.end() ) {
        return *it;
    } else {
        return Macro();
    }
}

LexerCache* Driver::lexerCache() {
  return &m_lexerCache;
}

