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
#include <language/duchain/repositories/itemrepository.h>
#include <language/duchain/modificationrevision.h>
#include "parser/rpp/macrorepository.h"
#include "cppdebughelper.h"

//  #define LEXERCACHE_DEBUG
//  #define ifDebug(x) x

using namespace Cpp;
using namespace KDevelop;

//Repository that contains the actual macros, and maps them to indices
MacroDataRepository Cpp::EnvironmentManager::macroDataRepository("macro repository");
//Set-repository that contains the string-sets
Utils::BasicSetRepository Cpp::EnvironmentManager::stringSetRepository("string sets", true);
//Set-repository that contains the macro-sets
Utils::BasicSetRepository Cpp::EnvironmentManager::macroSetRepository("macro sets", true);

const rpp::pp_macro& Cpp::MacroIndexConversion::toItem(uint index) const {
  return *EnvironmentManager::macroDataRepository.itemFromIndex( index );
}

uint Cpp::MacroIndexConversion::toIndex(const rpp::pp_macro& macro) const {
  return EnvironmentManager::macroDataRepository.index( MacroRepositoryItemRequest(macro) );
}

//For debugging
QString id(const EnvironmentFile* file) {
  return file->url().str() + QString(" %1").arg((size_t)file) ;
}

//Only for debugging
QString print(const Utils::Set& s) {
  QString ret;
  bool first = true;
  StringSetIterator it( s );
  while(it) {
    if(!first)
      ret += ", ";
    first = false;
    
    ret += (*it).str();
    ++it;
  }
  return ret;
}

QString print(const Cpp::LazyStringSet& set) {
  return print( set.set() );
}

QString print(const Cpp::LazyMacroSet& set) {
  QString ret;
  bool first = true;
  MacroSetIterator it( set.set() );
  while(it) {
    if(!first)
      ret += ", ";
    first = false;
    
    ret += it.ref().toString();
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
    cnt++;
    files.first++;
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

EnvironmentFilePointer EnvironmentManager::lexedFile( const IndexedString& fileName, const rpp::Environment* environment, KDevelop::ParsingEnvironmentFileAcceptor* acceptor ) {
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
    
    if( cppEnvironment->identityOffsetRestriction() && cppEnvironment->identityOffsetRestriction() != file.identityOffset() ) {
      kDebug( 9007 ) << "file" << fileName.str() << "does not match branching hash. Restriction:" << cppEnvironment->identityOffsetRestriction() << "Actual:" << file.identityOffset();
      continue;
    }

    //Make sure that none of the macros stored in the driver affect the file in a different way than the one before
    bool success = true;
   
    Utils::Set conflicts = (environmentMacroNames & file.strings()) - file.m_usedMacroNames.set();

    for( StringSetIterator it( conflicts ); it; ++it ) {
      rpp::pp_macro* m = environment->retrieveStoredMacro( *it );
      if(m && !m->isUndef()) {
#ifdef LEXERCACHE_DEBUG
        kDebug(9007) << "The environment contains a macro that can affect the cached file, but that should not exist:" << m->name.str();
#endif
        success = false;
        break;
      }
    }
    
    if( !success )
      continue;
    //Make sure that all external macros used by the file now exist too

    ///@todo find out why this assertion sometimes triggers
    //ifDebug( Q_ASSERT(file.m_usedMacros.set().count() == file.m_usedMacroNames.set().count()) );

    ifDebug( kDebug(9007) << "Count of used macros that need to be verified:" << file.m_usedMacros.set().count() );
    
    for ( MacroSetIterator it( file.m_usedMacros.set() ); it; ++it ) {
      rpp::pp_macro* m = environment->retrieveStoredMacro( it.ref().name );
      if ( !m || !(*m == it.ref()) ) {
        if( !m && it.ref().isUndef() ) {
          ifDebug( kDebug( 9007 ) << "Undef-macro" << it.ref().name.str() << "is ok" << m );
          //It is okay, we did not find a macro, but the used macro is an undef macro
          //Q_ASSERT(0); //Undef-macros should not be marked as used
        } else {
          ifDebug( kDebug( 9007 ) << "The cached file " << fileName.str() << " used a macro called \"" << it.ref().name.str() << "\"(from" << it.ref().file.str() << "), but the environment" << (m ? "contains differing macro of that name" : "does not contain that macro") << ", the cached file is not used"  );
          ifDebug( if(m) { kDebug() << "Used macro: " << it.ref().toString()  << "from" << it.ref().file.str() << "found:" << m->toString() << "from" << m->file.str(); } );
          success = false;
          break;
        }
      }else{
        ifDebug( kDebug( 9007 ) << it.ref().name.str() << "match" );
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

//Should be cached too!
bool EnvironmentManager::needsUpdate( const KDevelop::ParsingEnvironmentFile* filePtr ) const {
  //@todo Check if any of the dependencies changed
  const EnvironmentFile* file = dynamic_cast<const EnvironmentFile*>(filePtr);

  if(!file) {
    kDebug() << "called with wrong file type";
    return false;
  }

  ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile( file->url() );

  if ( revision != file->modificationRevision() ) {
    ifDebug( kDebug( 9007 ) << file->url().str() << "has changed, stored stamp:" << file->modificationRevision() << "new time:" << revision  );
    return true;
  }

  for( QMap<IndexedString, ModificationRevision>::const_iterator it = file->allModificationTimes().begin(); it != file->allModificationTimes().end(); ++it ) {
    ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile( it.key() );
    if( revision != *it ) {
      ifDebug( kDebug( 9007 ) << "dependency" << it.key().str() << "has changed, stored stamp:" << it.value() << "new time:" << revision  );
      return true;
    }
  }

  return false;
}


void EnvironmentManager::clear() {
  m_files.clear();
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

EnvironmentFile::EnvironmentFile( const IndexedString& fileName, EnvironmentManager* manager ) : CacheNode( manager ), m_identityOffset(0), m_url( fileName ), m_includeFiles(&EnvironmentManager::stringSetRepository), m_missingIncludeFiles(&EnvironmentManager::stringSetRepository), m_usedMacros(&EnvironmentManager::macroSetRepository), m_usedMacroNames(&EnvironmentManager::stringSetRepository), m_definedMacros(&EnvironmentManager::macroSetRepository), m_definedMacroNames(&EnvironmentManager::stringSetRepository), m_unDefinedMacroNames(&EnvironmentManager::stringSetRepository),m_contentStartLine(0) {
  QFileInfo fileInfo( KUrl(fileName.str()).path() ); ///@todo care about remote documents
  m_modificationTime = fileInfo.lastModified();
  ifDebug( kDebug(9007) << "created for" << fileName.str() << "modification-time:" << m_modificationTime  );

  addIncludeFile( m_url, m_modificationTime );
}

void EnvironmentFile::setContentStartLine(int line) {
  m_contentStartLine = line;
}

int EnvironmentFile::contentStartLine() const {
  return m_contentStartLine;
}

void EnvironmentFile::addDefinedMacro( const rpp::pp_macro& macro, const rpp::pp_macro* previousOfSameName ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 )  << id(this) << "defined macro" << macro.name.str();
#endif
  if( previousOfSameName && m_definedMacros.contains(*previousOfSameName) ) ///@todo Make this faster. We cannot remove the definedMacros.contains(..), because else we get problems.
    m_definedMacros.remove( *previousOfSameName );
  else if( m_definedMacroNames.contains(macro.name) ) {
    //Search if there is already a macro of the same name in the set, and remove it
    //This is slow, but should not happen too often
    ///@todo maybe give a warning, and find out how this can happen
    for( MacroSetIterator it( m_definedMacros.set() ); it; ++it )
      if( macro.name == it.ref().name )
        m_definedMacros.remove(it.ref());
  }
  
  if(macro.isUndef()) {
    m_definedMacroNames.remove( macro.name );
    
    m_unDefinedMacroNames.insert( macro.name );
  } else {
    m_unDefinedMacroNames.remove( macro.name );
    m_definedMacroNames.insert( macro.name );
    
    m_definedMacros.insert( macro );
  }
  
  //State: If it is an undef macro, it is not in m_definedMacroNames not in m_definedMacros, and it is in m_unDefinedMacroNames
  //       If  it is a normal macro, it is in m_definedMacroNames, it is in m_definedMacros, and it is not in m_unDefinedMacroNames
}

void EnvironmentFile::usingMacro( const rpp::pp_macro& macro ) {
  if ( !m_definedMacroNames.contains( macro.name ) && !m_unDefinedMacroNames.contains( macro.name ) && !macro.isUndef() ) {
#ifdef LEXERCACHE_DEBUG
    kDebug( 9007 ) << id(this) << "used macro" << macro.name.str() << "from" << macro.file.str();
#endif
    m_usedMacros.insert( macro );
  
    m_usedMacroNames.insert( macro.name );
  }
}

// const IndexedStringSet& EnvironmentFile::includeFiles() const {
//   return m_includeFiles;
// }

Utils::Set EnvironmentFile::strings() const {
  return m_strings;
}


///Set of all defined macros, including those of all deeper included files
const LazyMacroSet& EnvironmentFile::definedMacros() const {
  return m_definedMacros;
}

///Set of all macros used from outside, including those used in deeper included files
const LazyStringSet& EnvironmentFile::usedMacroNames() const {
  return m_usedMacroNames;
}

///Set of all macros used from outside, including those used in deeper included files
const LazyStringSet& EnvironmentFile::definedMacroNames() const {
  return m_definedMacroNames;
}

const LazyStringSet& EnvironmentFile::unDefinedMacroNames() const {
  return m_unDefinedMacroNames;
}

///Set of all macros used from outside, including those used in deeper included files
const LazyMacroSet& EnvironmentFile::usedMacros() const {
  return m_usedMacros;
}

const QList<IndexedString>& EnvironmentFile::includePaths() const {
  return m_includePaths;
}

void EnvironmentFile::setIncludePaths( const QList<IndexedString>& paths ) {
  m_includePaths = paths;
}

///Should contain a modification-time for each included-file
const QMap<IndexedString, KDevelop::ModificationRevision>& EnvironmentFile::allModificationTimes() const {
  return m_allModificationTimes;
}

void EnvironmentFile::clearModificationTimes() {
  m_allModificationTimes.clear();
  m_allModificationTimes[m_url] = m_modificationTime;
}

IndexedString EnvironmentFile::url() const {
  return m_url;
}

void EnvironmentFile::addMissingIncludeFile(const IndexedString& file)
{
  m_missingIncludeFiles.insert(file);
}

const LazyStringSet& EnvironmentFile::missingIncludeFiles() const
{
  return m_missingIncludeFiles;
}

void EnvironmentFile::clearMissingIncludeFiles()
{
  m_missingIncludeFiles.clear();
}

void EnvironmentFile::addIncludeFile( const IndexedString& file, const ModificationRevision& modificationTime ) {
  m_includeFiles.insert(file);
  m_allModificationTimes[file] = modificationTime;
}

void EnvironmentFile::setModificationRevision( const KDevelop::ModificationRevision& rev ) {
  m_modificationTime = rev;
  m_allModificationTimes[m_url] = rev;
}

KDevelop::ModificationRevision EnvironmentFile::modificationRevision() const {
  return m_modificationTime;
}

void EnvironmentFile::addStrings( const std::set<Utils::BasicSetRepository::Index>& strings ) {
  m_strings += EnvironmentManager::stringSetRepository.createSet(strings);
}

//The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
void EnvironmentFile::merge( const EnvironmentFile& file ) {
#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) <<  id(this) << ": merging" << id(&file)  << "defined in macros this:" << print(m_definedMacroNames)  << "defined macros in other:" << print(file.m_definedMacroNames) << "undefined macros in other:" << print(file.m_unDefinedMacroNames) << "strings in other:" << print(file.m_strings);
#endif
  m_strings += (file.m_strings - m_definedMacroNames.set()) - m_unDefinedMacroNames.set();
  ///@todo Probably it's more efficient having 2 sets m_changedMacroNames and m_unDefinedMacroNames, where m_unDefinedMacroNames is a subset of m_changedMacroNames.  
  //Only add macros to the usedMacros-set that were not defined locally
  m_usedMacroNames += (file.m_usedMacroNames.set() - m_definedMacroNames.set()) - m_unDefinedMacroNames.set();

  ///Merge those used macros that were not defined within this environment
  //This is slightly inefficient, would be nicer to have a fast mechanism for this
  for( MacroSetIterator it( file.m_usedMacros.set() ); it; ++it )
    if( !m_definedMacroNames.contains(it.ref().name) && !m_unDefinedMacroNames.contains(it.ref().name) )
      m_usedMacros.insert( it.ref() );

  ifDebug( Q_ASSERT(m_usedMacroNames.set().count() == m_usedMacros.set().count()) );
  
  ///Add defined macros from the merged file.

  //Since merged macros overrule already stored ones, first remove the ones of the same name.
  for( MacroSetIterator it( m_definedMacros.set() ); it; ++it )
    if( file.m_definedMacroNames.contains( it.ref().name ) || file.m_unDefinedMacroNames.contains( it.ref().name ) )
      m_definedMacros.remove(it.ref());

  //Now merge in the new defined macros
  
  m_unDefinedMacroNames += file.m_unDefinedMacroNames.set();
  m_unDefinedMacroNames -= file.m_definedMacroNames.set();
  m_definedMacroNames -= file.m_unDefinedMacroNames.set();
  m_definedMacroNames += file.m_definedMacroNames.set();
  m_definedMacros += file.m_definedMacros.set();

  ///Merge include-files, problems and other stuff
  m_includeFiles += file.m_includeFiles.set();
  m_missingIncludeFiles += file.m_missingIncludeFiles.set();
  
  for( QMap<IndexedString, KDevelop::ModificationRevision>::const_iterator it = file.m_allModificationTimes.begin(); it != file.m_allModificationTimes.end(); ++it )
    m_allModificationTimes[it.key()] = *it;


#ifdef LEXERCACHE_DEBUG
  kDebug( 9007 ) << id(this) << ": defined macro names in this after merge:" << m_definedMacroNames.set().count() << print(m_definedMacroNames);
  kDebug( 9007 ) << id(this) << ": defined in this after merge:" << m_definedMacros.set().count() << print(m_definedMacros);
  ifDebug( Q_ASSERT(m_definedMacros.set().count() == m_definedMacroNames.set().count()) );
  kDebug( 9007 ) << id(this) << ": undefined in this after merge:" << print(m_unDefinedMacroNames);
  kDebug( 9007 ) << id(this) << ": strings in this after merge:" << print(m_strings);
  kDebug( 9007 ) << id(this) << ": macros used in this after merge:" << print(m_usedMacroNames);
#endif
}

size_t EnvironmentFile::hash() const {
  ///@todo remove the (size_t)(this), it is just temporary to make them unique, but will not work with serialization to disk.
  ///Instead, create a hash over the contained strings, and make sure the other hashes work reliably.
  return (size_t)(this); //m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + (size_t)(this)/*+ m_strings.hash()*/; ///@todo is the string-hash needed here?
}

uint EnvironmentFile::identityOffset() const {
  return m_identityOffset;
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

int EnvironmentManager::type() const {
  return CppParsingEnvironment;
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
KDevelop::ParsingEnvironmentFile* EnvironmentManager::find( const IndexedString& url, const ParsingEnvironment* environment, KDevelop::ParsingEnvironmentFileAcceptor* accepter ) {
  const rpp::Environment* env = dynamic_cast<const rpp::Environment*>(environment);
  if( !env ) {
    kDebug(9007) << "called with a wrong environment of type" << environment->type();
    return 0;
  }
  EnvironmentFilePointer p = lexedFile( url, env, accepter );
  Q_ASSERT(!p || !(p->ref == 1 || p->ref == 0));
  return p.data();
}

