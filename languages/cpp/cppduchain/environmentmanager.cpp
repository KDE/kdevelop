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

#include "environmentmanager.h"
#include <QFileInfo>
#include <kdebug.h>
#include "rpp/pp-macro.h"
#include "rpp/pp-environment.h"
#include <iproblem.h>
#include "cpppreprocessenvironment.h"

//#define LEXERCACHE_DEBUG

#ifndef LEXERCACHE_DEBUG
#define ifDebug(X) /**/
#else
#define ifDebug(X) X
#endif

using namespace Cpp;
using namespace KDevelop;

QMutex EnvironmentManager::m_stringRepositoryMutex;
Utils::SetRepository<HashedString, HashedStringHash> EnvironmentManager::m_stringRepository;

//For debugging
QString id(const EnvironmentFile* file) {
  return file->url().prettyUrl() + QString(" %1").arg((size_t)file);
}

//Only for debugging
QString print(const Cpp::StringSetRepository::LazySet& set) {
  Utils::Set s = set.set();
  QString ret;
  bool first = true;
  Cpp::StringSetRepository::Iterator it(&EnvironmentManager::m_stringRepository, s.iterator());
  while(it) {
    if(!first)
      ret += ", ";
    first = false;
    
    ret += (*it).str();
    ++it;
  }
  return ret;
}

EnvironmentManager::EnvironmentManager() : m_simplifiedMatching(false) {
}

EnvironmentManager::~EnvironmentManager() {
}

void EnvironmentManager::setSimplifiedMatching(bool simplified) {
  m_simplifiedMatching = simplified;
}

bool EnvironmentManager::isSimplifiedMatching() const {
  return m_simplifiedMatching;
}

void EnvironmentManager::addEnvironmentFile( const EnvironmentFilePointer& file ) {
  ifDebug( kDebug( 9007 ) << "EnvironmentManager: adding an instance of" << file->url()  );

  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( file->hashedUrl() );

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
  //kDebug( 9007 ) << "EnvironmentManager: new count of cached instances for the file:" << cnt;
}

void EnvironmentManager::removeEnvironmentFile( const EnvironmentFilePointer& file ) {
  ifDebug( kDebug( 9007 ) << "EnvironmentManager::removeEnvironmentFile: removing an instance of" << file->url()  );

  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( file->hashedUrl() );

  int cnt = 0;
  while ( files.first != files.second ) {
    if ( (*files.first).second == file ) {
      m_files.erase( files.first++ );
    } else  {
      ++cnt;
      files.first++;
    }
  }
  ifDebug( kDebug( 9007 ) << "EnvironmentManager::removeEnvironmentFile: new count of cached instances for the file:" << cnt  );
}

EnvironmentFilePointer EnvironmentManager::lexedFile( const KUrl& url, const rpp::Environment* environment, KDevelop::ParsingEnvironmentFileAcceptor* acceptor )  {
  return lexedFile( HashedString( url.prettyUrl( KUrl::RemoveTrailingSlash )), environment, acceptor );
}

EnvironmentFilePointer EnvironmentManager::lexedFile( const HashedString& fileName, const rpp::Environment* environment, KDevelop::ParsingEnvironmentFileAcceptor* acceptor ) {
  initFileModificationCache();
  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( fileName );

  

#ifdef LEXERCACHE_DEBUG
    if( files.first != files.second ) {
      int count = 0;
      std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files2 = files;
      for( ; files2.first != files2.second; ++files2.first )
        ++count;
        kDebug( 9007 ) << "EnvironmentManager::lexedFile: cache for file " << fileName.str() << " has " << count << " entries";
    } else {
        kDebug( 9007 ) << "EnvironmentManager::lexedFile: cache for file " << fileName.str() << " is empty";
    }
#endif

  const CppPreprocessEnvironment* cppEnvironment = dynamic_cast<const CppPreprocessEnvironment*>(environment);
  Q_ASSERT(cppEnvironment);

  Utils::Set environmentMacroNames = cppEnvironment->macroNameSet().set();

  while ( files.first != files.second ) {
    const EnvironmentFile& file( *( *( files.first ) ).second );

    if(acceptor && !acceptor->accept(*( *files.first ).second)) {
      ++files.first;
      continue;
    }
    
    bool success = true;
    //Make sure that none of the macros stored in the driver affect the file in a different way than the one before
   
    Utils::Set conflicts = environmentMacroNames & file.strings();

    StringSetIterator conflictIt(&EnvironmentManager::m_stringRepository, conflicts.iterator());
    
    rpp::Environment::EnvironmentMap::const_iterator end = environment->environment().end();
    for( ; conflictIt; ++conflictIt) {
      rpp::Environment::EnvironmentMap::const_iterator rit = environment->environment().find(*conflictIt);
      if(rit == environment->environment().end()) {
        kDebug(9007) << "Found environment-macro name, but not the fitting macro: " << (*conflictIt).str();
        continue;
      }
      rpp::Environment::EnvironmentMap::const_iterator it = rit;
      ++rit;
      while ( rit != end && it.key() == rit.key() ) {; //Always only use the last macro of the same name for comparison, it is on top of the macro-stack
        it = rit;
        ++rit;
      }
      if (( *it )->isUndef() ) continue; //Undef-macros practically don't exist

      if ( file.m_usedMacros.hasMacro( it.key() ) ) {
        rpp::pp_macro m( file.m_usedMacros.macro(it.key()) );
        if ( !( m == **it ) ) {
          ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: The cached file " << fileName.str() << " depends on the string \"" << it.key().str() << "\" and used a macro for it with the body \"" << m.definition << "\"(from" << m.file << "), but the driver contains the same macro with body \"" << ( *it )->definition << "\"(from" << ( *it )->file << "), cache is not used"  );

          //rpp::pp_macro with the same name was used, but it is different
          success = false;
          break;
        }

      } else {
        //There is a macro that affects the file, but was not used while the previous parse
        ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: The cached file " << fileName.str() << " depends on the string \"" << ( it ).key().str() << "\" and the driver contains a macro of that name with body \"" << ( *it )->definition << "\"(from" << ( *it )->file << "), the cached file is not used"  );
        success = false;
        break;
      }
    }
    //Make sure that all external macros used by the file now exist too
  MacroSet::Macros::const_iterator end2 = file.usedMacros().macros().end();
    for ( MacroSet::Macros::const_iterator it = file.usedMacros().macros().begin(); it != end2; ++it ) {
      if ( !environment->retrieveStoredMacro( ( *it ).name ) ) {
        ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: The cached file " << fileName.str() << " used a macro called \"" << it->name.str() << "\"(from" << it->file << "), but the driver does not contain that macro, the cached file is not used"  );
        success = false;
        break;
      }
    }

    if ( success ) {
      ifDebug( kDebug( 9007 ) << "EnvironmentManager::lexedFile: Using cached file " << fileName.str() );
      (*files.first).second->access();
      return ( *files.first ).second;
    }
    ++files.first;
  }
  return EnvironmentFilePointer();
}

QDateTime EnvironmentManager::fileModificationTimeCached( const HashedString& fileName ) const {
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
bool EnvironmentManager::hasSourceChanged( const EnvironmentFile& file ) const {
  //@todo Check if any of the dependencies changed

  ModificationRevision revision = EditorIntegrator::modificationRevision( file.url() );

  if ( revision != file.modificationRevision() ) {
    ifDebug( kDebug( 9007 ) << "EnvironmentManager::hasSourceChanged:" << file.url() << "has changed, stored stamp:" << file.modificationRevision() << "new time:" << revision  );
    return true;
  }

  for( QMap<HashedString, ModificationRevision>::const_iterator it = file.allModificationTimes().begin(); it != file.allModificationTimes().end(); ++it ) {
    ModificationRevision revision = EditorIntegrator::modificationRevision( file.url() );
    if( revision != *it ) {
      ifDebug( kDebug( 9007 ) << "EnvironmentManager::hasSourceChanged: dependency" << it.key().str() << "has changed"  );
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
  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( ((const EnvironmentFile*)(node))->hashedUrl() );
  while ( files.first != files.second ) {
    if( (*files.first).second.data() == ((const EnvironmentFile*)(node)) ) {
      m_files.erase( files.first );
      return;
    }
      files.first++;
  }
  ifDebug( kDebug( 9007 ) << "Error: could not find a node in the list for file" << ((const EnvironmentFile*)(node))->url()  );
}

EnvironmentFile::EnvironmentFile( const KUrl& fileName, EnvironmentManager* manager ) : CacheNode( manager ), m_identityOffset(0), m_url( fileName ), m_includeFiles(&EnvironmentManager::m_stringRepository, &EnvironmentManager::m_stringRepositoryMutex), m_definedMacroNames(&EnvironmentManager::m_stringRepository, &EnvironmentManager::m_stringRepositoryMutex) {
  QFileInfo fileInfo( fileName.path() ); ///@todo care about remote documents
  m_modificationTime = fileInfo.lastModified();
  ifDebug( kDebug(9007) << "EnvironmentFile::EnvironmentFile: created for" << fileName << "modification-time:" << m_modificationTime  );

  m_hashedUrl = fileName.prettyUrl(KUrl::RemoveTrailingSlash);
  addIncludeFile( m_hashedUrl, m_modificationTime );
  m_allModificationTimes[ m_hashedUrl ] = m_modificationTime;
}

void EnvironmentFile::addDefinedMacro( const rpp::pp_macro& macro ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 )  << id(this) << "defined macro" << macro.name.str();
#endif
  m_definedMacros.addMacro( macro );
  
  m_definedMacroNames.insert( macro.name );
}

void EnvironmentFile::addUsedMacro( const rpp::pp_macro& macro ) {
  if ( !m_definedMacros.hasMacro( macro.name ) ) {
#ifdef LEXERCACHE_DEBUG
    kDebug( 9007 ) << id(this) << "used macro" << macro.name.str() << "from" << macro.file;
#endif
    m_usedMacros.addMacro( macro );
  }
}

// const HashedStringSet& EnvironmentFile::includeFiles() const {
//   return m_includeFiles;
// }

Utils::Set EnvironmentFile::strings() const {
  return m_strings;
}


///Set of all defined macros, including those of all deeper included files
const MacroSet& EnvironmentFile::definedMacros() const {
  return m_definedMacros;
}

///Set of all macros used from outside, including those used in deeper included files
const MacroSet& EnvironmentFile::usedMacros() const {
  return m_usedMacros;
}

const KUrl::List& EnvironmentFile::includePaths() const {
  return m_includePaths;
}

void EnvironmentFile::setIncludePaths( const KUrl::List& paths ) {
  m_includePaths = paths;
}

///Should contain a modification-time for each included-file
const QMap<HashedString, KDevelop::ModificationRevision>& EnvironmentFile::allModificationTimes() const {
  return m_allModificationTimes;
}

KUrl EnvironmentFile::url() const {
  return m_url;
}

HashedString EnvironmentFile::hashedUrl() const {
  return m_hashedUrl;
}

void EnvironmentFile::addIncludeFile( const HashedString& file, const ModificationRevision& modificationTime ) {
  m_includeFiles.insert(file);
  m_allModificationTimes[file] = modificationTime;
}

void EnvironmentFile::setModificationRevision( const KDevelop::ModificationRevision& rev ) {
  m_modificationTime = rev;
}

KDevelop::ModificationRevision EnvironmentFile::modificationRevision() const {
  return m_modificationTime;
}

void EnvironmentFile::addProblem( const Problem& p ) {
  m_problems << p;
}

QList<Problem>  EnvironmentFile::problems() const {
  return m_problems;
}

void EnvironmentFile::addStrings( const std::set<Utils::BasicSetRepository::Index>& strings ) {
  QMutexLocker l(&EnvironmentManager::m_stringRepositoryMutex);
  m_strings += EnvironmentManager::m_stringRepository.createSet(strings);
}

//The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
void EnvironmentFile::merge( const EnvironmentFile& file ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) <<  id(this) << ": merging" << id(&file)  << "defined in this:" << print(m_definedMacroNames)  << "defined macros in other:" << print(file.m_definedMacroNames);
#endif
  Utils::Set tempStrings = file.m_strings;
  tempStrings -= m_definedMacroNames.set();
  m_strings += tempStrings;
  m_includeFiles += file.m_includeFiles.set();
  //Only add macros to the usedMacros-list that were not defined locally
  for ( MacroSet::Macros::const_iterator it = file.m_usedMacros.macros().begin(); it != file.m_usedMacros.macros().end(); ++it ) {
    if ( !m_definedMacros.hasMacro(( *it ).name ) ) {///If the macro was not defined locally, add it to the macros-list.
#ifdef LEXERCACHE_DEBUG
      kDebug( 9007 ) <<  id(this) <<  "inserting used macro" << ( *it ).name.str();
#endif
      m_usedMacros.addMacro( *it );
    }
  }

  m_definedMacros.merge( file.m_definedMacros );
  m_definedMacroNames += file.m_definedMacroNames.set();

  for( QMap<HashedString, KDevelop::ModificationRevision>::const_iterator it = file.m_allModificationTimes.begin(); it != file.m_allModificationTimes.end(); ++it )
    m_allModificationTimes[it.key()] = *it;


#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) << id(this) << ": defined in this after merge:" << print(m_definedMacroNames);
#endif
  m_problems += file.m_problems;
}

size_t EnvironmentFile::hash() const {
  ///@todo remove the (size_t)(this), it is just temporary to make them unique, bit will not work with serialization to disk.
  ///Instead, create a hash over the contained strings, and make sure the other hashes work reliably.
  return m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + (size_t)(this)/*+ m_strings.hash()*/; ///@todo is the string-hash needed here?
}

void EnvironmentFile::setIdentityOffset(uint offset) {
  m_identityOffset = offset;
}

IdentifiedFile EnvironmentFile::identity() const {
  return IdentifiedFile(m_url, (uint)hash() + m_identityOffset);
}

int EnvironmentFile::type() const {
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
  EnvironmentFile* cfile = dynamic_cast<EnvironmentFile*>(file);
  if( !cfile ) {
    kDebug(9007) << "EnvironmentManager::addFile() called with a non-cpp EnvironmentFile of type" << file->type();
    return;
  }
  ifDebug( kDebug(9007) << "EnvironmentManager::addFile" << cfile->url()  );
  addEnvironmentFile(EnvironmentFilePointer(cfile));
}
///Remove a file from the manager
void EnvironmentManager::removeFile( ParsingEnvironmentFile* file ) {
  EnvironmentFile* cfile = dynamic_cast<EnvironmentFile*>(file);
  if( !cfile ) {
    kDebug(9007) << "EnvironmentManager::removeFile() called with a non-cpp EnvironmentFile of type" << file->type();
    return;
  }
  ifDebug( kDebug(9007) << "EnvironmentManager::removeFile" << cfile->url()  );
  removeEnvironmentFile(EnvironmentFilePointer(cfile));
}

/**
 * Search for the availability of a file parsed in a given environment
 * */
KDevelop::ParsingEnvironmentFile* EnvironmentManager::find( const KUrl& url, const ParsingEnvironment* environment, KDevelop::ParsingEnvironmentFileAcceptor* accepter ) {
  const rpp::Environment* env = dynamic_cast<const rpp::Environment*>(environment);
  if( !env ) {
    kDebug(9007) << "EnvironmentManager::find() called with a wrong environment of type" << environment->type();
    return 0;
  }
  return lexedFile( url, env, accepter ).data();
}


