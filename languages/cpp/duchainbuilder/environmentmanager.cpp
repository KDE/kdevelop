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

#include <QFileInfo>
#include "environmentmanager.h"
#include <kdebug.h>
#include "rpp/pp-macro.h"
#include "rpp/pp-environment.h"
#include "problem.h"
#include "cppparserexport.h"

#define LEXERCACHE_DEBUG

#ifndef LEXERCACHE_DEBUG
#define ifDebug(X) /**/
#else
#define ifDebug(X) X
#endif

using namespace Cpp;
using namespace KDevelop;

EnvironmentManager::EnvironmentManager() {
}

EnvironmentManager::~EnvironmentManager() {
}

void EnvironmentManager::addLexedFile( const LexedFilePointer& file ) {
  ifDebug( kDebug( 9007 ) << "EnvironmentManager: adding an instance of " << file->url() << endl );

  std::pair< LexedFileMap::iterator, LexedFileMap::iterator> files = m_files.equal_range( file->hashedUrl() );

  if ( files.first == files.second ) {
    m_files.insert( std::make_pair( file->hashedUrl(), file ) );
  } else {
      //Make sure newer files appear first
      m_files.insert( files.first, std::make_pair( file->hashedUrl(), file ) );
  }

  int cnt = 0;
  while ( files.first != files.second ) {
    if ( hasSourceChanged( *( *( files.first ) ).second ) ) {
      m_files.erase( files.first++ ); ///@todo give notification to du-chain
    } else  {
      cnt++;
      files.first++;
    }
  }
  //kDebug( 9007 ) << "EnvironmentManager: new count of cached instances for the file: " << cnt << endl;
}

void EnvironmentManager::removeLexedFile( const LexedFilePointer& file ) {
  ifDebug( kDebug( 9007 ) << "EnvironmentManager::removeLexedFile: removing an instance of " << file->url() << endl );

  std::pair< LexedFileMap::iterator, LexedFileMap::iterator> files = m_files.equal_range( file->hashedUrl() );

  int cnt = 0;
  while ( files.first != files.second ) {
    if ( (*files.first).second == file ) {
      m_files.erase( files.first++ );
    } else  {
      ++cnt;
      files.first++;
    }
  }
  ifDebug( kDebug( 9007 ) << "EnvironmentManager::removeLexedFile: new count of cached instances for the file: " << cnt << endl );
}

LexedFilePointer EnvironmentManager::lexedFile( const KUrl& url, const rpp::Environment* environment )  {
  return lexedFile( HashedString( url.prettyUrl( KUrl::RemoveTrailingSlash )), environment );
}

LexedFilePointer EnvironmentManager::lexedFile( const HashedString& fileName, const rpp::Environment* environment ) {
  initFileModificationCache();
  std::pair< LexedFileMap::iterator, LexedFileMap::iterator> files = m_files.equal_range( fileName );

  ///@todo optimize with standard-algorithms(by first computing the intersection)

#ifdef LEXERCACHE_DEBUG
    if( files.first != files.second ) {
      int count = 0;
      std::pair< LexedFileMap::iterator, LexedFileMap::iterator> files2 = files;
      for( ; files2.first != files2.second; ++files2.first )
        ++count;
        kDebug( 9007 ) << "EnvironmentManager::lexedFile: cache for file " << fileName.str() << " has " << count << " entries" << endl;
    } else {
        kDebug( 9007 ) << "EnvironmentManager::lexedFile: cache for file " << fileName.str() << " is empty" << endl;
    }
#endif

  while ( files.first != files.second ) {
    const LexedFile& file( *( *( files.first ) ).second );
    if ( hasSourceChanged( file ) ) {
      ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: cache for file " << fileName.str() << " is being discarded because the file was modified" << endl );
      m_files.erase( files.first++ ); ///@todo give notification to du-chain
      continue;
    }
    bool success = true;
    //Make sure that none of the macros stored in the driver affect the file in a different way than the one before
    rpp::Environment::EnvironmentMap::const_iterator end = environment->environment().end();
    for ( rpp::Environment::EnvironmentMap::const_iterator rit = environment->environment().begin(); rit != end; ) {
      rpp::Environment::EnvironmentMap::const_iterator it = rit;
      ++rit;
      if ( rit != end && it.key() == rit.key() ) continue; //Always only use the last macro of the same name for comparison, it is on top of the macro-stack
      if (( *it )->isUndef() ) continue; //Undef-macros practically don't exist

      if ( file.hasString( it.key() ) ) {
        if ( file.m_usedMacros.hasMacro( it.key() ) ) {
          rpp::pp_macro m( file.m_usedMacros.macro(it.key() ) );
          if ( !( m == **it ) ) {
            ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: The cached file " << fileName.str() << " depends on the string \"" << it.key() << "\" and used a macro for it with the body \"" << m.definition << "\"(from " << m.file << "), but the driver contains the same macro with body \"" << ( *it )->definition << "\"(from " << ( *it )->file << "), cache is not used" << endl );

            //rpp::pp_macro with the same name was used, but it is different
            success = false;
            break;
          }

        } else {
          //There is a macro that affects the file, but was not used while the previous parse
          ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: The cached file " << fileName.str() << " depends on the string \"" << ( it ).key() << "\" and the driver contains a macro of that name with body \"" << ( *it )->definition << "\"(from " << ( *it )->file << "), the cached file is not used" << endl );
          success = false;
          break;
        }
      }
    }
    //Make sure that all external macros used by the file now exist too
  MacroSet::Macros::const_iterator end2 = file.usedMacros().macros().end();
    for ( MacroSet::Macros::const_iterator it = file.usedMacros().macros().begin(); it != end2; ++it ) {
      if ( !environment->retrieveMacro( ( *it ).name ) ) {
        ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: The cached file " << fileName.str() << " used a macro called \"" << it->name << "\"(from " << it->file << "), but the driver does not contain that macro, the cached file is not used" << endl );
        success = false;
        break;
      }
    }

    if ( success ) {
      ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: Using cached file " << fileName.str() << endl );
      (*files.first).second->access();
      return ( *files.first ).second;
    }
    ++files.first;
  }
  return LexedFilePointer();
}

QDateTime EnvironmentManager::fileModificationTimeCached( const HashedString& fileName ) {
  FileModificationMap::const_iterator it = m_fileModificationCache.find( fileName );
  if( it != m_fileModificationCache.end() ) {
    ///Use the cache for 10 seconds
    if( (*it).second.m_readTime.secsTo( m_currentDateTime ) < 10 ) {
      return (*it).second.m_modificationTime;
    }
  }

  KUrl u(fileName.str());
  QFileInfo fileInfo( u.path() ); ///@todo support non-local files and modifications in editor
  m_fileModificationCache[fileName].m_readTime = QDateTime::currentDateTime();
  m_fileModificationCache[fileName].m_modificationTime = fileInfo.lastModified();
  return fileInfo.lastModified();
  
}

//Should be cached too!
bool EnvironmentManager::hasSourceChanged( const LexedFile& file ) {
  //@todo Check if any of the dependencies changed

  QDateTime modTime = fileModificationTimeCached( file.url().path() );
  
  if ( modTime != file.modificationTime() ) {
    ifDebug( kDebug( 9007 ) << "EnvironmentManager::hasSourceChanged: " << file.url() << " has changed, stored stamp: " << file.modificationTime() << " new time: " << modTime << endl );
    return true;
  }

  for( QMap<HashedString, QDateTime>::const_iterator it = file.allModificationTimes().begin(); it != file.allModificationTimes().end(); ++it ) {
    QDateTime modTime = fileModificationTimeCached( it.key() );
    if( modTime != *it ) {
      ifDebug( kDebug( 9007 ) << "EnvironmentManager::hasSourceChanged: dependency " << it.key().str() << " has changed" << endl );
      return true;
    }
  }
  
  return false;
}


void EnvironmentManager::clear() {
  m_files.clear();
  m_totalStringSet.clear();
  m_fileModificationCache.clear();
}

void EnvironmentManager::erase( const CacheNode* node ) {
  std::pair< LexedFileMap::iterator, LexedFileMap::iterator> files = m_files.equal_range( ((const LexedFile*)(node))->hashedUrl() );
  while ( files.first != files.second ) {
    if( (*files.first).second.data() == ((const LexedFile*)(node)) ) {
      m_files.erase( files.first );
      return;
    }
      files.first++;
  }
  ifDebug( kDebug( 9007 ) << "Error: could not find a node in the list for file " << ((const LexedFile*)(node))->url() << endl );
}

LexedFile::LexedFile( const KUrl& fileName, EnvironmentManager* manager ) : CacheNode( manager ), m_url( fileName ) {
  QFileInfo fileInfo( fileName.path() ); ///@todo care about remote documents
  m_modificationTime = fileInfo.lastModified();
  ifDebug( kDebug(9007) << "LexedFile::LexedFile: created for " << fileName << " modification-time: " << m_modificationTime << endl );

  m_hashedUrl = fileName.prettyUrl(KUrl::RemoveTrailingSlash);
  addIncludeFile( m_hashedUrl, m_modificationTime );
  m_allModificationTimes[ m_hashedUrl ] = m_modificationTime;
}

void LexedFile::addDefinedMacro( const rpp::pp_macro& macro ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) << "defined macro " << macro.name << endl;
#endif
  m_definedMacros.addMacro( macro );
  m_definedMacroNames.insert( HashedString( macro.name ) );
}

void LexedFile::addUsedMacro( const rpp::pp_macro& macro ) {
  if ( !m_definedMacros.hasMacro( macro.name ) ) {
#ifdef LEXERCACHE_DEBUG
    kDebug( 9007 ) << "used macro " << macro.name << endl;
#endif
    m_usedMacros.addMacro( macro );
  }
}

const HashedStringSet& LexedFile::includeFiles() const {
  return m_includeFiles;
}

///Set of all defined macros, including those of all deeper included files
const MacroSet& LexedFile::definedMacros() const {
  return m_definedMacros;
}

///Set of all macros used from outside, including those used in deeper included files
const MacroSet& LexedFile::usedMacros() const {
  return m_usedMacros;
}

///Should contain a modification-time for each included-file
const QMap<HashedString, QDateTime>& LexedFile::allModificationTimes() const {
  return m_allModificationTimes;
}

KUrl LexedFile::url() const {
  return m_url;
}

HashedString LexedFile::hashedUrl() const {
  return m_hashedUrl;
}

void LexedFile::addIncludeFile( const HashedString& file, const QDateTime& modificationTime ) {
  m_includeFiles.insert( file );
  m_allModificationTimes[file] = modificationTime;
}


QDateTime LexedFile::modificationTime() const {
  return m_modificationTime;
}

void LexedFile::addProblem( const Problem& p ) {
  m_problems << p;
}

QList<Problem>  LexedFile::problems() const {
  return m_problems;
}

//The parameter should be a LexedFile that was lexed AFTER the content of this file
void LexedFile::merge( const LexedFile& file ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) << url() << ": merging " << file.url() << endl << "defined in this: " << m_definedMacroNames.print().c_str() << endl << "defined macros in other: " << file.m_definedMacroNames.print().c_str() << endl;;
#endif
  HashedStringSet tempStrings = file.m_strings;
  tempStrings -= m_definedMacroNames;
  m_strings += tempStrings;
  m_includeFiles += file.m_includeFiles;
  //Only add macros to the usedMacros-list that were not defined locally
  for ( MacroSet::Macros::const_iterator it = file.m_usedMacros.macros().begin(); it != file.m_usedMacros.macros().end(); ++it ) {
    if ( !m_definedMacros.hasMacro(( *it ).name ) ) {///If the macro was not defined locally, add it to the macros-list.
#ifdef LEXERCACHE_DEBUG
      kDebug( 9007 ) << "inserting used macro " << ( *it ).name << endl;
#endif
      m_usedMacros.addMacro( *it );
    }
  }

  m_definedMacros.merge( file.m_definedMacros );
  m_definedMacroNames += file.m_definedMacroNames;
  
  for( QMap<HashedString, QDateTime>::const_iterator it = file.m_allModificationTimes.begin(); it != file.m_allModificationTimes.end(); ++it )
    m_allModificationTimes[it.key()] = *it;

  
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) << url() << ": defined in this after merge: " << m_definedMacroNames.print().c_str() << endl;
#endif
  m_problems += file.m_problems;
}

size_t LexedFile::hash() const {
  return m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + m_strings.hash();
}

IdentifiedFile LexedFile::identity() const {
  return IdentifiedFile(m_url, (uint)hash());
}

int LexedFile::type() const {
  return CppParsingEnvironment;
}

void EnvironmentManager::initFileModificationCache() {
  m_currentDateTime = QDateTime::currentDateTime();
}

int EnvironmentManager::type() const {
  return CppParsingEnvironment;
}


void EnvironmentManager::saveMemory() {
  m_fileModificationCache.clear();

  m_totalStringSet.clear(); ///it's unclear how often this should be emptied. It may happen that completely unused strings end up in this set, then deleting it will save us memory.
}

///Add a new file to the manager
void EnvironmentManager::addFile( ParsingEnvironmentFile* file ) {
  LexedFile* cfile = dynamic_cast<LexedFile*>(file);
  if( !cfile ) {
    kDebug() << "EnvironmentManager::addFile() called with a non-CachedLexFile of type " << file->type() << endl;
    return;
  }
  ifDebug( kDebug(9007) << "EnvironmentManager::addFile " << cfile->url() << endl );
  addLexedFile(LexedFilePointer(cfile));
}
///Remove a file from the manager
void EnvironmentManager::removeFile( ParsingEnvironmentFile* file ) {
  LexedFile* cfile = dynamic_cast<LexedFile*>(file);
  if( !cfile ) {
    kDebug() << "EnvironmentManager::removeFile() called with a non-CachedLexFile of type " << file->type() << endl;
    return;
  }
  ifDebug( kDebug(9007) << "EnvironmentManager::removeFile " << cfile->url() << endl );
  removeLexedFile(LexedFilePointer(cfile));
}

/**
 * Search for the availability of a file parsed in a given environment
 * */
KDevelop::ParsingEnvironmentFile* EnvironmentManager::find( const KUrl& url, const ParsingEnvironment* environment ) {
  const rpp::Environment* env = dynamic_cast<const rpp::Environment*>(environment);
  if( !env ) {
    kDebug() << "EnvironmentManager::find() called with a wrong environment of type " << environment->type() << endl;
    return 0;
  }
  return lexedFile( url, env ).data();
}

    
