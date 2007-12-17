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

QMutex EnvironmentManager::m_repositoryMutex(QMutex::Recursive);
StringSetRepository EnvironmentManager::m_stringRepository;
MacroRepository EnvironmentManager::m_macroRepository;

//For debugging
QString id(const EnvironmentFile* file) {
  return file->url().str() + QString(" %1").arg((size_t)file);
}

//Only for debugging
QString print(const Utils::Set& s) {
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

QString print(const Cpp::StringSetRepository::LazySet& set) {
  return print( set.set() );
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
  ifDebug( kDebug( 9007 ) << "adding an instance of" << file->url().str()  );

  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( file->url() );

  if ( files.first == files.second ) {
    m_files.insert( std::make_pair( file->url(), file ) );
  } else {
      //Make sure newer files appear first
      m_files.insert( files.first, std::make_pair( file->url(), file ) );
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
  ifDebug( kDebug( 9007 ) << "removing an instance of" << file->url().str()  );

  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( file->url() );

  int cnt = 0;
  while ( files.first != files.second ) {
    if ( (*files.first).second == file ) {
      m_files.erase( files.first++ );
    } else  {
      ++cnt;
      files.first++;
    }
  }
  ifDebug( kDebug( 9007 ) << "new count of cached instances for the file:" << cnt  );
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
        kDebug( 9007 ) << "cache for file " << fileName.str() << " has " << count << " entries";
    } else {
        kDebug( 9007 ) << "cache for file " << fileName.str() << " is empty";
    }
#endif

  const CppPreprocessEnvironment* cppEnvironment = dynamic_cast<const CppPreprocessEnvironment*>(environment);
  Q_ASSERT(cppEnvironment);

  Utils::Set environmentMacroNames = cppEnvironment->macroNameSet().set();

  for ( ;files.first != files.second; ++files.first ) {
    const EnvironmentFile& file( *( *( files.first ) ).second );

    if(acceptor && !acceptor->accept(*( *files.first ).second)) {
      continue;
    }

    //Make sure that none of the macros stored in the driver affect the file in a different way than the one before
    bool success = true;
   
    Utils::Set conflicts = (environmentMacroNames & file.strings()) - file.m_usedMacroNames.set();

    for( MacroRepository::Iterator it( &EnvironmentManager::m_macroRepository, conflicts.iterator() ); it; ++it ) {
      rpp::pp_macro* m = environment->retrieveStoredMacro( (*it).name );
      if(m && !m->isUndef()) {
#ifdef LEXERCACHE_DEBUG
        kDebug() << "The environment contains a macro that can affect the cached file:" << m->name.str();
#endif
        success = false;
        break;
      }
    }
    
    if( !success )
      continue;
    //Make sure that all external macros used by the file now exist too

    for ( MacroRepository::Iterator it( &EnvironmentManager::m_macroRepository, file.m_usedMacros.set().iterator() ); it; ++it ) {
      rpp::pp_macro* m = environment->retrieveStoredMacro( ( *it ).name );
      if ( !m || !(*m == *it) ) {
        if( !m && !(*it).isUndef() ) {
          //It is okay, we did not find a macro, but the used macro is an undef macro
          //Q_ASSERT(0); //Undef-macros should not be marked as used
        } else {
          ifDebug( kDebug( 9007 ) << "The cached file " << fileName.str() << " used a macro called \"" << (*it).name.str() << "\"(from" << (*it).file.str() << "), but the driver does not contain that macro or the macro differs, the cached file is not used"  );
          success = false;
          break;
        }
      }
    }
    if( !success )
      continue;

    ifDebug( kDebug( 9007 ) << "Using cached file " << fileName.str() );
    //(*files.first).second->access();
    return ( *files.first ).second;
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
    ifDebug( kDebug( 9007 ) << file.url().str() << "has changed, stored stamp:" << file.modificationRevision() << "new time:" << revision  );
    return true;
  }

  for( QMap<HashedString, ModificationRevision>::const_iterator it = file.allModificationTimes().begin(); it != file.allModificationTimes().end(); ++it ) {
    ModificationRevision revision = EditorIntegrator::modificationRevision( file.url() );
    if( revision != *it ) {
      ifDebug( kDebug( 9007 ) << "dependency" << it.key().str() << "has changed"  );
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
  std::pair< EnvironmentFileMap::iterator, EnvironmentFileMap::iterator> files = m_files.equal_range( ((const EnvironmentFile*)(node))->url() );
  while ( files.first != files.second ) {
    if( (*files.first).second.data() == ((const EnvironmentFile*)(node)) ) {
      m_files.erase( files.first );
      return;
    }
      files.first++;
  }
  ifDebug( kDebug( 9007 ) << "Error: could not find a node in the list for file" << ((const EnvironmentFile*)(node))->url().str()  );
}

EnvironmentFile::EnvironmentFile( const HashedString& fileName, EnvironmentManager* manager ) : CacheNode( manager ), m_identityOffset(0), m_url( fileName ), m_includeFiles(&EnvironmentManager::m_stringRepository, &EnvironmentManager::m_repositoryMutex), m_usedMacros(&EnvironmentManager::m_macroRepository, &EnvironmentManager::m_repositoryMutex), m_usedMacroNames(&EnvironmentManager::m_stringRepository, &EnvironmentManager::m_repositoryMutex), m_definedMacros(&EnvironmentManager::m_macroRepository, &EnvironmentManager::m_repositoryMutex), m_definedMacroNames(&EnvironmentManager::m_stringRepository, &EnvironmentManager::m_repositoryMutex) {
  QFileInfo fileInfo( KUrl(fileName.str()).path() ); ///@todo care about remote documents
  m_modificationTime = fileInfo.lastModified();
  ifDebug( kDebug(9007) << "created for" << fileName.str() << "modification-time:" << m_modificationTime  );

  addIncludeFile( m_url, m_modificationTime );
  m_allModificationTimes[ m_url ] = m_modificationTime;
}

void EnvironmentFile::addDefinedMacro( const rpp::pp_macro& macro, const rpp::pp_macro* previousOfSameName ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 )  << id(this) << "defined macro" << macro.name.str();
#endif
  Q_ASSERT( previousOfSameName || !m_definedMacroNames.contains(macro.name) );
  if( previousOfSameName )
    m_definedMacros.remove( *previousOfSameName );
  
  m_definedMacros.insert( macro );
  
  m_definedMacroNames.insert( macro.name );
}

void EnvironmentFile::usingMacro( const rpp::pp_macro& macro ) {
  if ( !m_definedMacroNames.contains( macro.name ) && !macro.isUndef() ) {
#ifdef LEXERCACHE_DEBUG
    kDebug( 9007 ) << id(this) << "used macro" << macro.name.str() << "from" << macro.file.str();
#endif
    m_usedMacros.insert( macro );
  
    m_usedMacroNames.insert( macro.name );
  }
}

// const HashedStringSet& EnvironmentFile::includeFiles() const {
//   return m_includeFiles;
// }

Utils::Set EnvironmentFile::strings() const {
  return m_strings;
}


///Set of all defined macros, including those of all deeper included files
const MacroRepository::LazySet& EnvironmentFile::definedMacros() const {
  return m_definedMacros;
}

///Set of all macros used from outside, including those used in deeper included files
const StringSetRepository::LazySet& EnvironmentFile::usedMacroNames() const {
  return m_usedMacroNames;
}

///Set of all macros used from outside, including those used in deeper included files
const StringSetRepository::LazySet& EnvironmentFile::definedMacroNames() const {
  return m_definedMacroNames;
}

///Set of all macros used from outside, including those used in deeper included files
const MacroRepository::LazySet& EnvironmentFile::usedMacros() const {
  return m_usedMacros;
}

const QList<HashedString>& EnvironmentFile::includePaths() const {
  return m_includePaths;
}

void EnvironmentFile::setIncludePaths( const QList<HashedString>& paths ) {
  m_includePaths = paths;
}

///Should contain a modification-time for each included-file
const QMap<HashedString, KDevelop::ModificationRevision>& EnvironmentFile::allModificationTimes() const {
  return m_allModificationTimes;
}

HashedString EnvironmentFile::url() const {
  return m_url;
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
  QMutexLocker l(&EnvironmentManager::m_repositoryMutex);
  m_strings += EnvironmentManager::m_stringRepository.createSet(strings);
}

//The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
void EnvironmentFile::merge( const EnvironmentFile& file ) {
  QMutexLocker l(&EnvironmentManager::m_repositoryMutex);
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) <<  id(this) << ": merging" << id(&file)  << "defined in this:" << print(m_definedMacroNames)  << "defined macros in other:" << print(file.m_definedMacroNames) << "strings in other:" << print(file.m_strings);
#endif
  m_strings += file.m_strings - m_definedMacroNames.set();
  
  //Only add macros to the usedMacros-list that were not defined locally
  m_usedMacroNames += file.m_usedMacroNames.set() - m_definedMacroNames.set();

  ///Merge those used macros that were not defined within this environment
  //This is slightly inefficient, would be nicer to have a fast mechanism for this
  for( MacroRepository::Iterator it( &EnvironmentManager::m_macroRepository, file.m_usedMacros.set().iterator() ); it; ++it )
    if( !m_definedMacroNames.contains((*it).name) )
      m_usedMacros.insert( *it );

  ///Add defined macros from the merged file.

  //Since merged macros overrule already stored ones, first remove the ones of the same name.
  for( MacroRepository::Iterator it( &EnvironmentManager::m_macroRepository, m_definedMacros.set().iterator() ); it; ++it )
    if( file.m_definedMacroNames.contains( (*it).name ) )
      m_definedMacros.remove(*it);

  //Now merge in the new defined macros
  
  m_definedMacros += file.m_definedMacros.set();
  m_definedMacroNames += file.m_definedMacroNames.set();

  ///Merge include-files, problems and other stuff
  m_includeFiles += file.m_includeFiles.set();
  
  for( QMap<HashedString, KDevelop::ModificationRevision>::const_iterator it = file.m_allModificationTimes.begin(); it != file.m_allModificationTimes.end(); ++it )
    m_allModificationTimes[it.key()] = *it;


#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) << id(this) << ": defined in this after merge:" << print(m_definedMacroNames);
#endif
  m_problems += file.m_problems;
}

size_t EnvironmentFile::hash() const {
  ///@todo remove the (size_t)(this), it is just temporary to make them unique, but will not work with serialization to disk.
  ///Instead, create a hash over the contained strings, and make sure the other hashes work reliably.
  return (size_t)(this); //m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + (size_t)(this)/*+ m_strings.hash()*/; ///@todo is the string-hash needed here?
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
    kDebug(9007) << "called with a non-cpp EnvironmentFile of type" << file->type();
    return;
  }
  ifDebug( kDebug(9007) << cfile->url().str()  );
  addEnvironmentFile(EnvironmentFilePointer(cfile));
}
///Remove a file from the manager
void EnvironmentManager::removeFile( ParsingEnvironmentFile* file ) {
  EnvironmentFile* cfile = dynamic_cast<EnvironmentFile*>(file);
  if( !cfile ) {
    kDebug(9007) << "called with a non-cpp EnvironmentFile of type" << file->type();
    return;
  }
  ifDebug( kDebug(9007) << cfile->url().str()  );
  removeEnvironmentFile(EnvironmentFilePointer(cfile));
}

/**
 * Search for the availability of a file parsed in a given environment
 * */
KDevelop::ParsingEnvironmentFile* EnvironmentManager::find( const HashedString& url, const ParsingEnvironment* environment, KDevelop::ParsingEnvironmentFileAcceptor* accepter ) {
  const rpp::Environment* env = dynamic_cast<const rpp::Environment*>(environment);
  if( !env ) {
    kDebug(9007) << "called with a wrong environment of type" << environment->type();
    return 0;
  }
  EnvironmentFilePointer p = lexedFile( url, env, accepter );
  Q_ASSERT(!p || p->ref.value > 1);
  return p.data();
}

__gnu_cxx::hash_set<KDevelop::HashedString> EnvironmentManager::m_totalStringSet;

