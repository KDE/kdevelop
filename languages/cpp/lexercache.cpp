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

#include <QFileInfo>
#include "environmentmanager.h"
#include <kdebug.h>
#include "rpp/pp-macro.h"
#include "rpp/pp-environment.h"
#include "problem.h"
#include "cppparserexport.h"

using namespace Cpp;
using namespace KDevelop;

EnvironmentManager::EnvironmentManager() {
}

EnvironmentManager::~EnvironmentManager() {
}

void EnvironmentManager::addLexedFile( const CachedLexedFilePointer& file ) {
  //kdDebug( 9007 ) << "EnvironmentManager: adding an instance of " << file->fileName().str() << endl;

  std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range( file->hashedUrl() );

  if ( files.first == files.second ) {
    m_files.insert( std::make_pair( file->hashedUrl(), file ) );
  } else {
      //Make sure newer files appear first
      m_files.insert( files.first, std::make_pair( file->hashedUrl(), file ) );
  }

  int cnt = 0;
  while ( files.first != files.second ) {
    if ( hasSourceChanged( *( *( files.first ) ).second ) ) {
      m_files.erase( files.first++ );
    } else  {
      cnt++;
      files.first++;
    }
  }
  //kdDebug( 9007 ) << "EnvironmentManager: new count of cached instances for the file: " << cnt << endl;
}

void EnvironmentManager::removeLexedFile( const CachedLexedFilePointer& file ) {
  //kdDebug( 9007 ) << "EnvironmentManager: adding an instance of " << file->fileName().str() << endl;

  std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range( file->hashedUrl() );

  while ( files.first != files.second ) {
    if ( (*files.first) == file ) {
      m_files.erase( files.first++ );
    } else  {
      files.first++;
    }
  }
  //kdDebug( 9007 ) << "EnvironmentManager: new count of cached instances for the file: " << cnt << endl;
}

CachedLexedFilePointer EnvironmentManager::lexedFile( const KUrl& url, const rpp::Environment* environment )  {
  return lexedFile( HashedString( url.prettyUrl( KUrl::RemoveTrailingSlash )), environment );
}

CachedLexedFilePointer EnvironmentManager::lexedFile( const HashedString& fileName, const rpp::Environment* environment ) {
  initFileModificationCache();
  std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range( fileName );

  ///@todo optimize with standard-algorithms(by first computing the intersection)

  /*  if( files.first != files.second )
        //kdDebug( 9007 ) << "EnvironmentManager: cache for file " << fileName.str() << " is not empty" << endl;
    else
        //kdDebug( 9007 ) << "EnvironmentManager: cache for file " << fileName.str() << " is empty" << endl;*/

  while ( files.first != files.second ) {
    const CachedLexedFile& file( *( *( files.first ) ).second );
    if ( hasSourceChanged( file ) ) {
      //kdDebug( 9007 ) << "EnvironmentManager: cache for file " << fileName.str() << " is being discarded because the file was modified" << endl;
      m_files.erase( files.first++ );
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
            //kdDebug( 9007 ) << "EnvironmentManager: The cached file " << fileName.str() << " depends on the string \"" << ( *it ).first.str() << "\" and used a macro for it with the body \"" << m.body() << "\"(from " << m.fileName() << "), but the driver contains the same macro with body \"" << ( *it ).second.body() << "\"(from " << ( *it ).second.fileName() << "), cache is not used" << endl;

            //rpp::pp_macro with the same name was used, but it is different
            success = false;
            break;
          }

        } else {
          //There is a macro that affects the file, but was not used while the previous parse
          //kdDebug( 9007 ) << "EnvironmentManager: The cached file " << fileName.str() << " depends on the string \"" << ( *it ).first.str() << "\" and the driver contains a macro of that name with body \"" << ( *it ).second.body() << "\"(from " << ( *it ).second.fileName() << "), the cached file is not used" << endl;
          success = false;
          break;
        }
      }
    }
    //Make sure that all external macros used by the file now exist too
  MacroSet::Macros::const_iterator end2 = file.usedMacros().macros().end();
    for ( MacroSet::Macros::const_iterator it = file.usedMacros().macros().begin(); it != end2; ++it ) {
      if ( !environment->retrieveMacro( ( *it ).name ) ) {
        //kdDebug( 9007 ) << "EnvironmentManager: The cached file " << fileName.str() << " used a macro called \"" << it->name() << "\"(from " << it->fileName() << "), but the driver does not contain that macro, the cached file is not used" << endl;
        success = false;
        break;
      }
    }

    if ( success ) {
      //kdDebug( 9007 ) << "EnvironmentManager: Using cached file " << fileName.str() << endl;
      (*files.first).second->access();
      return ( *files.first ).second;
    }
    ++files.first;
  }
  return CachedLexedFilePointer();
}

QDateTime EnvironmentManager::fileModificationTimeCached( const HashedString& fileName ) {
  FileModificationMap::const_iterator it = m_fileModificationCache.find( fileName );
  if( it != m_fileModificationCache.end() ) {
    ///Use the cache for 10 seconds
    if( (*it).second.m_readTime.secsTo( m_currentDateTime ) < 10 ) {
      return (*it).second.m_modificationTime;
    }
  }
  
  QFileInfo fileInfo( fileName.str() );
  m_fileModificationCache[fileName].m_readTime = QDateTime::currentDateTime();
  m_fileModificationCache[fileName].m_modificationTime = fileInfo.lastModified();
  return fileInfo.lastModified();
  
}

//Should be cached too!
bool EnvironmentManager::hasSourceChanged( const CachedLexedFile& file ) {
  //@todo Check if any of the dependencies changed

  ///@todo make compatible with remote documents
  QDateTime modTime = fileModificationTimeCached( file.url().path() );
  
  if ( modTime != file.modificationTime() )
    return true;

  for( QMap<HashedString, QDateTime>::const_iterator it = file.allModificationTimes().begin(); it != file.allModificationTimes().end(); ++it ) {
    QDateTime modTime = fileModificationTimeCached( it.key() );
    if( modTime != *it )
      return true;
  }
  
  return false;
}


void EnvironmentManager::clear() {
  m_files.clear();
  m_totalStringSet.clear();
  m_fileModificationCache.clear();
}

void EnvironmentManager::erase( const CacheNode* node ) {
  std::pair< CachedLexedFileMap::iterator, CachedLexedFileMap::iterator> files = m_files.equal_range( ((const CachedLexedFile*)(node))->hashedUrl() );
  while ( files.first != files.second ) {
    if( (*files.first).second.data() == ((const CachedLexedFile*)(node)) ) {
      m_files.erase( files.first );
      return;
    }
      files.first++;
  }
  //kdDebug( 9007 ) << "Error: could not find a node in the list for file " << ((const CachedLexedFile*)(node))->fileName().str() << endl;
}

CachedLexedFile::CachedLexedFile( const KUrl& fileName, EnvironmentManager* manager ) : CacheNode( manager ), m_url( fileName ) {
  QFileInfo fileInfo( fileName.path() ); ///@todo care about remote documents
  m_modificationTime = fileInfo.lastModified();

  m_hashedUrl = fileName.prettyUrl(KUrl::RemoveTrailingSlash);
  addIncludeFile( m_hashedUrl, m_modificationTime );
  m_allModificationTimes[ m_hashedUrl ] = m_modificationTime;
}

void CachedLexedFile::addDefinedMacro( const rpp::pp_macro& macro ) {
#ifdef LEXERCACHE_DEBUG
  //kdDebug( 9007 ) << "defined macro " << macro.name() << endl;
#endif
  m_definedMacros.addMacro( macro );
  m_definedMacroNames.insert( HashedString( macro.name ) );
}

void CachedLexedFile::addUsedMacro( const rpp::pp_macro& macro ) {
  if ( !m_definedMacros.hasMacro( macro.name ) ) {
#ifdef LEXERCACHE_DEBUG
    //kdDebug( 9007 ) << "used macro " << macro.name() << endl;
#endif
    m_usedMacros.addMacro( macro );
  }
}

const HashedStringSet& CachedLexedFile::includeFiles() const {
  return m_includeFiles;
}

///Set of all defined macros, including those of all deeper included files
const MacroSet& CachedLexedFile::definedMacros() const {
  return m_definedMacros;
}

///Set of all macros used from outside, including those used in deeper included files
const MacroSet& CachedLexedFile::usedMacros() const {
  return m_usedMacros;
}

///Should contain a modification-time for each included-file
const QMap<HashedString, QDateTime>& CachedLexedFile::allModificationTimes() const {
  return m_allModificationTimes;
}

KUrl CachedLexedFile::url() const {
  return m_url;
}

HashedString CachedLexedFile::hashedUrl() const {
  return m_hashedUrl;
}

void CachedLexedFile::addIncludeFile( const HashedString& file, const QDateTime& modificationTime ) {
  m_includeFiles.insert( file );
  m_allModificationTimes[file] = modificationTime;
}


QDateTime CachedLexedFile::modificationTime() const {
  return m_modificationTime;
}

void CachedLexedFile::addProblem( const Problem& p ) {
  m_problems << p;
}

QList<Problem>  CachedLexedFile::problems() const {
  return m_problems;
}

//The parameter should be a CachedLexedFile that was lexed AFTER the content of this file
void CachedLexedFile::merge( const CachedLexedFile& file ) {
#ifdef LEXERCACHE_DEBUG
  //kdDebug( 9007 ) << fileName().str() << ": merging " << file.fileName().str() << endl << "defined in this: " << m_definedMacroNames.print().c_str() << endl << "defined macros in other: " << file.m_definedMacroNames.print().c_str() << endl;;
#endif
  HashedStringSet tempStrings = file.m_strings;
  tempStrings -= m_definedMacroNames;
  m_strings += tempStrings;
  m_includeFiles += file.m_includeFiles;
  //Only add macros to the usedMacros-list that were not defined locally
  for ( MacroSet::Macros::const_iterator it = file.m_usedMacros.macros().begin(); it != file.m_usedMacros.macros().end(); ++it ) {
    if ( !m_definedMacros.hasMacro(( *it ).name ) ) {///If the macro was not defined locally, add it to the macros-list.
#ifdef LEXERCACHE_DEBUG
      //kdDebug( 9007 ) << "inserting used macro " << ( *it ).name() << endl;
#endif
      m_usedMacros.addMacro( *it );
    }
  }

  m_definedMacros.merge( file.m_definedMacros );
  m_definedMacroNames += file.m_definedMacroNames;
  
  for( QMap<HashedString, QDateTime>::const_iterator it = file.m_allModificationTimes.begin(); it != file.m_allModificationTimes.end(); ++it )
    m_allModificationTimes[it.key()] = *it;

  
#ifdef LEXERCACHE_DEBUG
  //kdDebug( 9007 ) << fileName().str() << ": defined in this after merge: " << m_definedMacroNames.print().c_str() << endl;
#endif
  m_problems += file.m_problems;
}

size_t CachedLexedFile::hash() const {
  return m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + m_strings.hash();
}

IdentifiedFile CachedLexedFile::identity() const {
  return IdentifiedFile(m_url, (uint)hash());
}

uint CachedLexedFile::type() const {
  return CppParsingEnvironment;
}

void EnvironmentManager::initFileModificationCache() {
  m_currentDateTime = QDateTime::currentDateTime();
}

virtual int EnvironmentManager::type() const {
  return CppParsingEnvironment;
}


void EnvironmentManager::saveMemory() {
  m_fileModificationCache.clear();

  m_totalStringSet.clear(); ///it's unclear how often this should be emptied. It may happen that completely unused strings end up in this set, then deleting it will save us memory.
}

///Add a new file to the manager
void EnvironmentManager::addFile( ParsingEnvironmentFilePointer file ) {
  CachedLexedFile* cfile = dynamic_cast<CachedLexedFile*>(file.data());
  if( !cfile ) {
    kDebug() << "EnvironmentManager::addFile() called with a non-CachedLexFile of type " << file->type() << endl;
    return;
  }
  addLexedFile(cfile);
}
///Remove a file from the manager
void EnvironmentManager::removeFile( ParsingEnvironmentFilePointer file ) {
  CachedLexedFile* cfile = dynamic_cast<CachedLexedFile*>(file.data());
  if( !cfile ) {
    kDebug() << "EnvironmentManager::removeFile() called with a non-CachedLexFile of type " << file->type() << endl;
    return;
  }
  removeLexedFile(cfile);
}

/**
 * Search for the availability of a file parsed in a given environment
 * */
IdentifiedFile EnvironmentManager::find( const KUrl& url, const ParsingEnvironment* environment ) {
  const rpp::Environment* env = dynamic_cast<const rpp::Environment*>(environment);
  if( !env ) {
    kDebug() << "EnvironmentManager::find() called with a wrong environment of type " << environment->type() << endl;
    return;
  }
  return lexedFile( url,env );
}

    
