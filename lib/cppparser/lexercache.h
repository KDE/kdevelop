/***************************************************************************
   copyright            : (C) 2006 by David Nolden
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
#include  "macro.h"
#include <kdebug.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <ext/hash_set>
#include "cachemanager.h"

//#define LEXERCACHE_DEBUG

class LexerCache;

class CachedLexedFile : public KShared, public CacheNode {
  public:
    ///@todo add and manage the set of included files
    CachedLexedFile( const HashedString& fileName, LexerCache* manager );
    
    inline void addString( const HashedString& string ) {
        if( !m_definedMacroNames[ string ] ) {
          m_strings.insert( string );
        }
    }

    void addDefinedMacro( const Macro& macro  );

    void addUsedMacro( const Macro& macro, int line, int column );

    void addIncludeFile( const HashedString& file, const QDateTime& modificationTime );

    inline bool hasString( const HashedString& string ) const {
      return m_strings[string];
    }

    QDateTime modificationTime() const;

    void addProblem( const Problem& p );

    QValueList<Problem>  problems() const;

    //The parameter should be a CachedLexedFile that was lexed AFTER the content of this file
    void merge( const CachedLexedFile& file );

    bool operator <  ( const CachedLexedFile& rhs ) const {
      return m_fileName < rhs.m_fileName;
    }

    size_t hash() const;

    HashedString fileName() const {
      return m_fileName;
    }
    
    const HashedStringSet& includeFiles() const {
      return m_includeFiles;
    }

    const MacroSet& definedMacros() const {
      return m_definedMacros;
    }
    
    const MacroSet& usedMacros() const {
      return m_usedMacros;
    }

    ///Should contain a modification-time for each include-file
    const QMap<HashedString, QDateTime>& allModificationTimes() const {
      return m_allModificationTimes;
    }

  private:
    friend class LexerCache;
    HashedString m_fileName;
    QDateTime m_modificationTime;
    HashedStringSet m_strings; //Set of all strings that can be affected by macros from outside
    HashedStringSet m_includeFiles; //Set of all files
    MacroSet m_usedMacros; //Set of all macros that were used, and were defined outside of this file
    MacroSet m_definedMacros; //Set of all macros that were defined while lexing this file
    HashedStringSet m_definedMacroNames;
    QValueList<Problem> m_problems;
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

struct CachedLexedFilePointerCompare {
  bool operator() ( const CachedLexedFilePointer& lhs, const CachedLexedFilePointer& rhs ) const {
    return (*lhs) < (*rhs );
  }
};

class Driver;

class LexerCache : public CacheManager {
  public:
    LexerCache( Driver* d );

    void addLexedFile( const CachedLexedFilePointer& file );

    ///Returns zero if no fitting file is available for the current context
    CachedLexedFilePointer lexedFile( const HashedString& fileName );

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
    typedef __gnu_cxx::hash_multimap<HashedString, CachedLexedFilePointer> CachedLexedFileMap;
    CachedLexedFileMap m_files;
    __gnu_cxx::hash_set<HashedString> m_totalStringSet; ///This is used to reduce memory-usage: Most strings appear again and again. Because QString is reference-counted, this set contains a unique copy of each string to used for each appearance of the string
    struct FileModificationCache {
      QDateTime m_readTime;
      QDateTime m_modificationTime;
    };
    typedef __gnu_cxx::hash_map<HashedString, FileModificationCache> FileModificationMap;
    FileModificationMap m_fileModificationCache;
    Driver* m_driver;
    QDateTime m_currentDateTime;
};


#endif
