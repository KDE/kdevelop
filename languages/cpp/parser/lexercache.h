/***************************************************************************
   copyright            : (C) 2006, 2007 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LEXERCACHE_H
#define LEXERCACHE_H
#include <hashedstring.h>
#include <ext/hash_map>
#include <ksharedptr.h>
#include <QDateTime>
#include <ext/hash_set>
#include <QList>
#include <QMap>
#include <map>
#include "macroset.h"
#include "cachemanager.h"

//#define LEXERCACHE_DEBUG

/**
 * The lexer-cache helps achieving right representation of the way c++ works:
 * When a file is processed by the preprocessor, the same file may create totally
 * different results depending on the defined macros. Think for example of header-
 * guards.
 *
 * Now when one file includes another, we want to know whether there already
 * is a readily parsed du-context for the file that WOULD BE CREATED if it was
 * preprocessed under the current environment of macros.
 *
 * The lexer-cache is there to answer that question:
 * CachedLexedFile collects all information about the context a file was parsed in,
 * the macros used, the words contained in a file that can be influenced by macros,
 * and the defined macros.
 *
 * The lexer-cache is able to match that information agains a current parsing-environment
 * to see whether preprocessing the file would yield the same result as an already stored
 * run.
 *
 * If the result would be different, the file will be re-preprocessed, parsed, and imported.
 * Else the set of defined macros is taken from the stored CachedLexedFile,
 * and the already available du-context will be imported. The result: correct behavior, perfectly working header-guards, no missing macros, intelligent reparsing of changed headers, ...
 * 
 * */

namespace rpp {
  class pp_macro;
  class Environment;
};

class Problem;

namespace Cpp {

class LexerCache;
class MacroSet;

class KDEVCPPPARSER_EXPORT CachedLexedFile : public KShared, public CacheNode {
  public:
    ///@todo Respect changing include-paths: Check if the included files are still the same(maybe new files are found that were not found before)
    CachedLexedFile( const HashedString& fileName, LexerCache* manager );
    
    inline void addString( const HashedString& string ) {
        if( !m_definedMacroNames[ string ] ) {
          m_strings.insert( string );
        }
    }

    void addDefinedMacro( const rpp::pp_macro& macro  );

    ///the given macro will only make it into usedMacros() if it was not defined in this file
    void addUsedMacro( const rpp::pp_macro& macro );

    void addIncludeFile( const HashedString& file, const QDateTime& modificationTime );

    inline bool hasString( const HashedString& string ) const {
      return m_strings[string];
    }

    QDateTime modificationTime() const;

    void addProblem( const Problem& p );

    QList<Problem>  problems() const;

    //The parameter should be a CachedLexedFile that was lexed AFTER the content of this file
    void merge( const CachedLexedFile& file );

    bool operator <  ( const CachedLexedFile& rhs ) const {
      return m_fileName < rhs.m_fileName;
    }

    size_t hash() const;

    HashedString fileName() const {
      return m_fileName;
    }

    /**Set of all files with absolute paths, including those included indirectly
     * 
     * This by definition also includes this file, so when the count is 1,
     * no other files were included.
     *
     * */
    const HashedStringSet& includeFiles() const {
      return m_includeFiles;
    }

    ///Set of all defined macros, including those of all deeper included files
    const MacroSet& definedMacros() const {
      return m_definedMacros;
    }
    
    ///Set of all macros used from outside, including those used in deeper included files
    const MacroSet& usedMacros() const {
      return m_usedMacros;
    }

    ///Should contain a modification-time for each included-file
    const QMap<HashedString, QDateTime>& allModificationTimes() const {
      return m_allModificationTimes;
    }

  private:
    friend class LexerCache;
    HashedString m_fileName;
    QDateTime m_modificationTime;
    HashedStringSet m_strings; //Set of all strings that can be affected by macros from outside
    HashedStringSet m_includeFiles; //Set of all files with absolute paths
    MacroSet m_usedMacros; //Set of all macros that were used, and were defined outside of this file
    MacroSet m_definedMacros; //Set of all macros that were defined while lexing this file
    HashedStringSet m_definedMacroNames;
    QList<Problem> m_problems;
    QMap<HashedString, QDateTime>  m_allModificationTimes;
    /*
    Needed data:
    1. Set of all strings that appear in this file(For memory-reasons they should be taken from a global string-repository, because many will be the same)
    2. Set of all macros that were defined outside of, but affected the file

    Algorithm:
      Iterate over all available macros, and check whether they affect the file. If it does, make sure that the macro is in the macro-set and has the same body.
      If the check fails: We need to reparse.
    */
};

typedef KSharedPtr<CachedLexedFile>  CachedLexedFilePointer;

struct KDEVCPPPARSER_EXPORT CachedLexedFilePointerCompare {
  bool operator() ( const CachedLexedFilePointer& lhs, const CachedLexedFilePointer& rhs ) const {
    return (*lhs) < (*rhs );
  }
};

class Driver;

class KDEVCPPPARSER_EXPORT LexerCache : public CacheManager {
  public:
    LexerCache();
    virtual ~LexerCache();

    void addLexedFile( const CachedLexedFilePointer& file );

    ///Returns zero if no fitting file is available for the given Environment
    CachedLexedFilePointer lexedFile( const HashedString& fileName, rpp::Environment* environment );

    void clear();

    const HashedString& unifyString( const HashedString& str ) {
      __gnu_cxx::hash_set<HashedString>::const_iterator it = m_totalStringSet.find( str );
      if( it != m_totalStringSet.end() ) {
        return *it;
      } else {
        m_totalStringSet.insert( str );
        return str;
      }
    }
    
    virtual void saveMemory();
  private:
    ///before this can be called, initFileModificationCache should be called once
    QDateTime fileModificationTimeCached( const HashedString& fileName );
    void initFileModificationCache();
    virtual void erase( const CacheNode* node );
    bool sourceChanged( const CachedLexedFile& file );///Returns true if the file itself, or any of its dependencies was modified.
    //typedef __gnu_cxx::hash_multimap<HashedString, CachedLexedFilePointer> CachedLexedFileMap;
    typedef std::multimap<HashedString, CachedLexedFilePointer> CachedLexedFileMap;
    CachedLexedFileMap m_files;
    __gnu_cxx::hash_set<HashedString> m_totalStringSet; ///This is used to reduce memory-usage: Most strings appear again and again. Because QString is reference-counted, this set contains a unique copy of each string to used for each appearance of the string
    struct FileModificationCache {
      QDateTime m_readTime;
      QDateTime m_modificationTime;
    };
    typedef __gnu_cxx::hash_map<HashedString, FileModificationCache> FileModificationMap;
    FileModificationMap m_fileModificationCache;
    QDateTime m_currentDateTime;
};

}

#endif
