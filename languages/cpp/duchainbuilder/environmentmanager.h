/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENVIRONMENTMANAGER_H
#define ENVIRONMENTMANAGER_H

#include <map>
#include <ext/hash_set>
#include <ext/hash_map>

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QMap>

#include <kurl.h>
#include <ksharedptr.h>

#include <parsingenvironment.h>
#include "cppduchainbuilderexport.h"
#include "hashedstring.h"
#include "macroset.h"
#include "cachemanager.h"

/**@todo Increase the intelligence of the include-file logic:
 * When a header was already included BEFORE the header represented by the
 * cached-lexed-file, and the cached-lexed-file includes that header,
 * it should be accepted anyway.
 *
 * That's complicated.
 * */

/**
 * The environment-manager helps achieving right representation of the way c++ works:
 * When a file is processed by the preprocessor, the same file may create totally
 * different results depending on the defined macros. Think for example of header-
 * guards.
 *
 * Now when one file includes another, we want to know whether there already
 * is a readily parsed du-context for the file that WOULD BE CREATED if it was
 * preprocessed under the current environment of macros.
 *
 * The environment-manager is there to answer that question:
 * EnvironmentFile collects all information about the context a file was parsed in,
 * the macros used, the words contained in a file that can be influenced by macros,
 * and the defined macros.
 *
 * The environment-manager is able to match that information agains a current parsing-environment
 * to see whether preprocessing the file would yield the same result as an already stored
 * run.
 *
 * If the result would be different, the file will be re-preprocessed, parsed, and imported.
 * Else the set of defined macros is taken from the stored EnvironmentFile,
 * and the already available du-context will be imported. The result: correct behavior, perfectly working header-guards, no missing macros, intelligent reparsing of changed headers, ...
 *
 * There is only one assumption made that needs to be fulfilled to be correct:
 * - Headers should be included at the top of the file
 * */

namespace rpp {
  class pp_macro;
  class Environment;
}

class Problem;

namespace Cpp {

class EnvironmentManager;
class MacroSet;

class KDEVCPPDUCHAINBUILDER_EXPORT EnvironmentFile : public CacheNode, public KDevelop::ParsingEnvironmentFile {
  public:
    ///@todo Respect changing include-paths: Check if the included files are still the same(maybe new files are found that were not found before)
    EnvironmentFile( const KUrl& url, EnvironmentManager* manager );

    inline void addString( const KDevelop::HashedString& string ) {
        if( !m_definedMacroNames[ string ] ) {
          m_strings.insert( string );
        }
    }

    void addDefinedMacro( const rpp::pp_macro& macro  );

    ///the given macro will only make it into usedMacros() if it was not defined in this file
    void addUsedMacro( const rpp::pp_macro& macro );

    void addIncludeFile( const KDevelop::HashedString& file, const QDateTime& modificationTime );

    inline bool hasString( const KDevelop::HashedString& string ) const {
      return m_strings[string];
    }

    QDateTime modificationTime() const;

    void addProblem( const Problem& p );

    QList<Problem>  problems() const;

    //The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
    void merge( const EnvironmentFile& file );

    bool operator <  ( const EnvironmentFile& rhs ) const {
      return m_hashedUrl < rhs.m_hashedUrl;
    }

    size_t hash() const;

    virtual KDevelop::IdentifiedFile identity() const;

    KUrl url() const;

    KDevelop::HashedString hashedUrl() const;

    /**Set of all files with absolute paths, including those included indirectly
     *
     * This by definition also includes this file, so when the count is 1,
     * no other files were included.
     *
     * */
    const HashedStringSet& includeFiles() const;

    ///Set of all defined macros, including those of all deeper included files
    const MacroSet& definedMacros() const;

    ///Set of all macros used from outside, including those used in deeper included files
    const MacroSet& usedMacros() const;

    ///Should contain a modification-time for each included-file
    const QMap<KDevelop::HashedString, QDateTime>& allModificationTimes() const;

  private:
    virtual int type() const;

    friend class EnvironmentManager;
    KUrl m_url;
    KDevelop::HashedString m_hashedUrl;
    QDateTime m_modificationTime;
    HashedStringSet m_strings; //Set of all strings that can be affected by macros from outside
    HashedStringSet m_includeFiles; //Set of all files with absolute paths
    MacroSet m_usedMacros; //Set of all macros that were used, and were defined outside of this file
    MacroSet m_definedMacros; //Set of all macros that were defined while lexing this file
    HashedStringSet m_definedMacroNames;
    QList<Problem> m_problems;
    QMap<KDevelop::HashedString, QDateTime>  m_allModificationTimes;
    /*
    Needed data:
    1. Set of all strings that appear in this file(For memory-reasons they should be taken from a global string-repository, because many will be the same)
    2. Set of all macros that were defined outside of, but affected the file

    Algorithm:
      Iterate over all available macros, and check whether they affect the file. If it does, make sure that the macro is in the macro-set and has the same body.
      If the check fails: We need to reparse.
    */
};

typedef KSharedPtr<EnvironmentFile>  EnvironmentFilePointer;

struct KDEVCPPDUCHAINBUILDER_EXPORT EnvironmentFilePointerCompare {
  bool operator() ( const EnvironmentFilePointer& lhs, const EnvironmentFilePointer& rhs ) const {
    return (*lhs) < (*rhs );
  }
};

class Driver;

class KDEVCPPDUCHAINBUILDER_EXPORT EnvironmentManager : public CacheManager, public KDevelop::ParsingEnvironmentManager {
  public:
    EnvironmentManager();
    virtual ~EnvironmentManager();

    const KDevelop::HashedString& unifyString( const KDevelop::HashedString& str ) {
      __gnu_cxx::hash_set<KDevelop::HashedString>::const_iterator it = m_totalStringSet.find( str );
      if( it != m_totalStringSet.end() ) {
        return *it;
      } else {
        m_totalStringSet.insert( str );
        return str;
      }
    }

    virtual void saveMemory();

    //Overridden from ParsingEnvironmentManager
    virtual void clear();

    ///Add a new file to the manager
    virtual void addFile( KDevelop::ParsingEnvironmentFile* file );
    ///Remove a file from the manager
    virtual void removeFile( KDevelop::ParsingEnvironmentFile* file );

    /**
     * Search for the availability of a file parsed in a given environment
     * */
    virtual KDevelop::ParsingEnvironmentFile* find( const KUrl& url, const KDevelop::ParsingEnvironment* environment );

  private:
    virtual int type() const;
    ///before this can be called, initFileModificationCache should be called once
    QDateTime fileModificationTimeCached( const KDevelop::HashedString& fileName );
    void initFileModificationCache();
    virtual void erase( const CacheNode* node );
    bool hasSourceChanged( const EnvironmentFile& file );///Returns true if the file itself, or any of its dependencies was modified.

    ///Returns zero if no fitting file is available for the given Environment
    EnvironmentFilePointer lexedFile( const KDevelop::HashedString& fileName, const rpp::Environment* environment );
    EnvironmentFilePointer lexedFile( const KUrl& url, const rpp::Environment* environment );

    void addEnvironmentFile( const EnvironmentFilePointer& file );
    void removeEnvironmentFile( const EnvironmentFilePointer& file );

    //typedef __gnu_cxx::hash_multimap<KDevelop::HashedString, EnvironmentFilePointer> EnvironmentFileMap;
    typedef std::multimap<KDevelop::HashedString, EnvironmentFilePointer> EnvironmentFileMap;
    EnvironmentFileMap m_files;
    __gnu_cxx::hash_set<KDevelop::HashedString> m_totalStringSet; ///This is used to reduce memory-usage: Most strings appear again and again. Because QString is reference-counted, this set contains a unique copy of each string to used for each appearance of the string
    struct FileModificationCache {
      QDateTime m_readTime;
      QDateTime m_modificationTime;
    };
    typedef __gnu_cxx::hash_map<KDevelop::HashedString, FileModificationCache> FileModificationMap;
    FileModificationMap m_fileModificationCache;
    QDateTime m_currentDateTime;
};

}

#endif
