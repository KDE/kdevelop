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
#include "rpp/pp-macro.h"
#include "rpp/pp-environment.h"
#include <language/duchain/problem.h>
#include "cpppreprocessenvironment.h"
#include <serialization/itemrepository.h>
#include <language/editor/modificationrevision.h>
#include "parser/rpp/macrorepository.h"
#include "cppdebughelper.h"
#include "debug.h"
#include <language/duchain/duchainregister.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>

using namespace KDevelop;

#define ENSURE_FILE_READ_LOCKED(file)   if((file).indexedTopContext().isValid()) { ENSURE_CHAIN_READ_LOCKED }
#define ENSURE_READ_LOCKED   ENSURE_FILE_READ_LOCKED(*this)
#define ENSURE_WRITE_LOCKED   if(indexedTopContext().isValid()) { ENSURE_CHAIN_READ_LOCKED }

DEFINE_LIST_MEMBER_HASH(IncludePathListItem, m_includePaths, KDevelop::IndexedString)

struct IncludePathListItem {

    IncludePathListItem() {
      initializeAppendedLists(true);
      m_refCount = 0;
    }
    IncludePathListItem(const IncludePathListItem& rhs, bool dynamic) {
      initializeAppendedLists(dynamic);
      m_refCount = rhs.m_refCount;
      copyListsFrom(rhs);
    }
    ~IncludePathListItem() {
      freeAppendedLists();
    }

    bool persistent() const {
      return (bool)m_refCount;
    }

    bool operator==(const IncludePathListItem& rhs) const {
      return listsEqual(rhs);
    }

    uint hash() const {
      uint ret = 0;
      for(uint a = 0; a < m_includePathsSize(); ++a)
        ret = (m_includePaths()[a].hash() + ret) * 17;

      return ret;
    }

    uint itemSize() const {
      return dynamicSize();
    }

    uint classSize() const {
      return sizeof(*this);
    }

    uint m_refCount;
    START_APPENDED_LISTS(IncludePathListItem);

    APPENDED_LIST_FIRST(IncludePathListItem, IndexedString, m_includePaths);
    END_APPENDED_LISTS(IncludePathListItem, m_includePaths);
  private:
    IncludePathListItem& operator=(const IncludePathListItem&);
};

typedef AppendedListItemRequest<IncludePathListItem, 40*4> IncludePathsRequest;

typedef KDevelop::ItemRepository<IncludePathListItem, IncludePathsRequest> IncludePathsRepository;
IncludePathsRepository& includePathsRepository()
{
  static IncludePathsRepository repo("include path repository");
  return repo;
}

//If DYNAMIC_DEBUGGING is defined, debugging can be started at any point in runtime,
//by calling setIsDebugging(true) from within the debugger
// #define DYNAMIC_DEBUGGING

#ifdef DYNAMIC_DEBUGGING
volatile bool is_debugging = true;

bool debugging() {
  return is_debugging;
}

//Ment to be called from within dbg, to start doing debug output at a specific point
void setIsDebugging(bool is) {
  is_debugging = is;
}

#define DEBUG_LEXERCACHE
#define ifDebug(x) if(debugging()) {x;}
#else
inline bool debugging() {
  return false;
}
#endif

using namespace Cpp;

REGISTER_DUCHAIN_ITEM(EnvironmentFile);

namespace Cpp {
  Utils::BasicSetRepository* StaticStringSetRepository::repository() {
    return &Cpp::EnvironmentManager::self()->stringSetRepository();
  }
  Utils::BasicSetRepository* StaticMacroSetRepository::repository() {
    return &Cpp::EnvironmentManager::self()->macroSetRepository();
  }
  void MacroSetRepository::itemRemovedFromSets(uint index) {
    Cpp::EnvironmentManager::self()->macroDataRepository().deleteItem(index);
  }
}

const rpp::pp_macro& Cpp::MacroIndexConversion::toItem(uint index) const {
  return *EnvironmentManager::self()->macroDataRepository().itemFromIndex( index );
}

uint Cpp::MacroIndexConversion::toIndex(const rpp::pp_macro& macro) const {
  return EnvironmentManager::self()->macroDataRepository().index( MacroRepositoryItemRequest(macro) );
}

//For debugging
QString id(const EnvironmentFile* file) {
  return file->url().str() + QString(" %1").arg((size_t)file) ;
}

QString print(const Cpp::ReferenceCountedStringSet& set) {
  QString ret;
  bool first = true;
  Cpp::ReferenceCountedStringSet::Iterator it( set.iterator() );
  while(it) {
    if(!first)
      ret += ", ";
    first = false;

    ret += (*it).str();
    ++it;
  }
  return ret;
}

QString print(const Cpp::ReferenceCountedMacroSet& set) {
  QString ret;
  bool first = true;
  Cpp::ReferenceCountedMacroSet::Iterator it( set.iterator() );
  while(it) {
    if(!first)
      ret += ", ";
    first = false;

    ret += it.ref().toString();
    ++it;
  }
  return ret;
}

EnvironmentManager* EnvironmentManager::m_self = 0;

EnvironmentManager::EnvironmentManager()
  : m_matchingLevel(Full), m_simplifiedMatching(false),
    m_macroDataRepository("macro repository"), m_stringSetRepository("string sets"), m_macroSetRepository()
{
}

void EnvironmentManager::init()
{
  Q_ASSERT_X(!m_self, "EnvironmentManager::init()", "do not call init() twice");
  m_self = new EnvironmentManager();
  // also init the other getters
  includePathsRepository();
}

void EnvironmentManager::setSimplifiedMatching(bool simplified)
{
  m_simplifiedMatching = simplified;
}

void Cpp::EnvironmentManager::setMatchingLevel(Cpp::EnvironmentManager::MatchingLevel level) {
  m_matchingLevel = level;
}

bool EnvironmentFile::matchEnvironment(const ParsingEnvironment* _environment) const {
  ENSURE_READ_LOCKED
  const CppPreprocessEnvironment* cppEnvironment = dynamic_cast<const CppPreprocessEnvironment*>(_environment);
  if(!cppEnvironment)
    return false;

  if( cppEnvironment->identityOffsetRestrictionEnabled() && cppEnvironment->identityOffsetRestriction() != identityOffset() ) {
#ifdef DEBUG_LEXERCACHE
    qCDebug(CPPDUCHAIN) << "file" << url().str() << "does not match branching hash. Restriction:" << cppEnvironment->identityOffsetRestriction() << "Actual:" << identityOffset();
#endif
    return false;
  }

  if(EnvironmentManager::self()->matchingLevel() == EnvironmentManager::Disabled)
    return true;

  //Consider files that are out-guarded by the header-guard as a match, without looking into their content
  ///@todo Pick the version that is already in the environment if there is multiple
  if(EnvironmentManager::self()->matchingLevel() == EnvironmentManager::Naive)
    if(cppEnvironment->macroNameSet().contains(headerGuard())) {
#ifdef DEBUG_LEXERCACHE
      qCDebug(CPPDUCHAIN) << "file" << url().str() << "environment contains the header-guard, returning true";
#endif
      return true;
    }

  const auto& environmentMacroNames = cppEnvironment->macroNameSet();

  const ReferenceCountedStringSet& conflicts = strings() - d_func()->m_usedMacroNames;

  for( ReferenceCountedStringSet::Iterator it(conflicts.iterator()); it; ++it ) {
    if (!environmentMacroNames.contains(it.ref())) {
      continue;
    }
    rpp::pp_macro* m = cppEnvironment->retrieveStoredMacro( *it );
    if(m && !m->isUndef()) {

#ifdef DEBUG_LEXERCACHE
      if(debugging()) {
        qCDebug(CPPDUCHAIN) << "The environment contains a macro that can affect the cached file, but that should not exist:" << m->name.str();
      }
#endif
      return false;
    }
  }

  //Make sure that all external macros used by the file now exist too

  ///@todo find out why this assertion sometimes triggers, maybe different macros with the same name were used?
  //ifDebug( Q_ASSERT(m_usedMacros.set().count() == m_usedMacroNames.set().count()) );
  ifDebug( qCDebug(CPPDUCHAIN) << "Count of used macros that need to be verified:" << d_func()->m_usedMacros.set().count() );

  for ( ReferenceCountedMacroSet::Iterator it( d_func()->m_usedMacros.iterator() ); it; ++it ) {
    rpp::pp_macro* m = cppEnvironment->retrieveStoredMacro( it.ref().name );
    if ( !m || !(*m == it.ref()) ) {
      if( !m && it.ref().isUndef() ) {
        ifDebug( qCDebug(CPPDUCHAIN) << "Undef-macro" << it.ref().name.str() << "is ok" << m );
        //It is okay, we did not find a macro, but the used macro is an undef macro
        //Q_ASSERT(0); //Undef-macros should not be marked as used
      } else {
        ifDebug( qCDebug(CPPDUCHAIN) << "The cached file " << url().str() << " used a macro called \"" << it.ref().name.str() << "\"(from" << it.ref().file.str() << "), but the environment" << (m ? "contains differing macro of that name" : "does not contain that macro") << ", the cached file is not used"  );
        ifDebug( if(m) { qCDebug(CPPDUCHAIN) << "Used macro: " << it.ref().toString()  << "from" << it.ref().file.str() << "found:" << m->toString() << "from" << m->file.str(); } );
        return false;
      }
    }else{
      ifDebug( qCDebug(CPPDUCHAIN) << it.ref().name.str() << "match" );
    }
  }

  ifDebug( qCDebug(CPPDUCHAIN) << "Using cached file " << url().str() );
  return true;
}

const KDevelop::ModificationRevisionSet& Cpp::EnvironmentFile::includePathDependencies() const
{
  ENSURE_READ_LOCKED
  return d_func()->m_includePathDependencies;
}

void Cpp::EnvironmentFile::setIncludePathDependencies(const KDevelop::ModificationRevisionSet& set)
{
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_includePathDependencies = set;
}

bool EnvironmentFile::needsUpdate(const ParsingEnvironment* environment) const {
  ENSURE_READ_LOCKED
  const CppPreprocessEnvironment* cppEnvironment = dynamic_cast<const CppPreprocessEnvironment*>(environment);

  //When in naive matching mode, we even use the non-guarded version when inappropriate. We must make sure not to update it in such
  //a situation, else it will end up empty
  if(cppEnvironment && EnvironmentManager::self()->matchingLevel() <= EnvironmentManager::Naive && !headerGuard().isEmpty() && cppEnvironment->macroNameSet().contains(headerGuard()))
    return false;

  return ParsingEnvironmentFile::needsUpdate(environment) || d_func()->m_includePathDependencies.needsUpdate();
}

EnvironmentFile::EnvironmentFile( const IndexedString& url, TopDUContext* topContext ) : ParsingEnvironmentFile(*new EnvironmentFileData(), url) {

  d_func_dynamic()->setClassId(this);
  setLanguage(IndexedString("C++"));

  d_func_dynamic()->m_topContext = IndexedTopDUContext(topContext);

  d_func_dynamic()->m_url = url;

//   ifDebug( qCDebug(CPPDUCHAIN) << "created for" << url.str() << "modification-time:" << d_func_dynamic()->m_modificationTime );

  clearModificationRevisions();
}

EnvironmentFile::EnvironmentFile( EnvironmentFileData& data ) : ParsingEnvironmentFile(data)
{
}

EnvironmentFile::~EnvironmentFile() {
}

void EnvironmentFile::setContentStartLine(int line) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_contentStartLine = line;
}

int EnvironmentFile::contentStartLine() const {
  ENSURE_READ_LOCKED
  return d_func()->m_contentStartLine;
}

void EnvironmentFile::addDefinedMacro( const rpp::pp_macro& macro, const rpp::pp_macro* previousOfSameName ) {
  ENSURE_WRITE_LOCKED
#ifdef DEBUG_LEXERCACHE
  if(debugging()) {
  qCDebug(CPPDUCHAIN)  << id(this) << "defined macro" << macro.name.str();
  }
#endif
  if( previousOfSameName && d_func()->m_definedMacros.contains(*previousOfSameName) )
    d_func_dynamic()->m_definedMacros.remove( *previousOfSameName );
  else if( d_func()->m_definedMacroNames.contains(macro.name) ) {
    //Search if there is already a macro of the same name in the set, and remove it
    //This is slow, but should not happen too often
    ///@todo maybe give a warning, and find out how this can happen
    for( ReferenceCountedMacroSet::Iterator it( d_func()->m_definedMacros.iterator() ); it; ++it )
      if( macro.name == it.ref().name )
        d_func_dynamic()->m_definedMacros.remove(it.ref());
  }

  if(macro.isUndef()) {
    d_func_dynamic()->m_definedMacroNames.remove( macro.name );

    d_func_dynamic()->m_unDefinedMacroNames.insert( macro.name );
  } else {
    d_func_dynamic()->m_unDefinedMacroNames.remove( macro.name );
    d_func_dynamic()->m_definedMacroNames.insert( macro.name );

    d_func_dynamic()->m_definedMacros.insert( macro );
  }

  //State: If it is an undef macro, it is not in m_definedMacroNames not in m_definedMacros, and it is in m_unDefinedMacroNames
  //       If  it is a normal macro, it is in m_definedMacroNames, it is in m_definedMacros, and it is not in m_unDefinedMacroNames
}

void EnvironmentFile::usingMacro( const rpp::pp_macro& macro ) {
  ENSURE_WRITE_LOCKED
  if ( !d_func()->m_definedMacroNames.contains( macro.name ) && !d_func()->m_unDefinedMacroNames.contains( macro.name ) && !macro.isUndef() ) {
#ifdef DEBUG_LEXERCACHE
  if(debugging()) {
    qCDebug(CPPDUCHAIN) << id(this) << "used macro" << macro.name.str() << "from" << macro.file.str();
  }
#endif
    d_func_dynamic()->m_usedMacros.insert( macro );

    d_func_dynamic()->m_usedMacroNames.insert( macro.name );
  }
}

// const IndexedStringSet& EnvironmentFile::includeFiles() const {
//   return m_includeFiles;
// }

const ReferenceCountedStringSet& EnvironmentFile::strings() const {
  ENSURE_READ_LOCKED
  return d_func()->m_strings;
}


///Set of all defined macros, including those of all deeper included files
const ReferenceCountedMacroSet& EnvironmentFile::definedMacros() const {
  ENSURE_READ_LOCKED
  return d_func()->m_definedMacros;
}

///Set of all macros used from outside, including those used in deeper included files
const ReferenceCountedStringSet& EnvironmentFile::usedMacroNames() const {
  ENSURE_READ_LOCKED
  return d_func()->m_usedMacroNames;
}

///Set of all macros used from outside, including those used in deeper included files
const ReferenceCountedStringSet& EnvironmentFile::definedMacroNames() const {
  ENSURE_READ_LOCKED
  return d_func()->m_definedMacroNames;
}

const ReferenceCountedStringSet& EnvironmentFile::unDefinedMacroNames() const {
  ENSURE_READ_LOCKED
  return d_func()->m_unDefinedMacroNames;
}

///Set of all macros used from outside, including those used in deeper included files
const ReferenceCountedMacroSet& EnvironmentFile::usedMacros() const {
  ENSURE_READ_LOCKED
  return d_func()->m_usedMacros;
}

const QList<IndexedString> EnvironmentFile::includePaths() const {
  ENSURE_READ_LOCKED
  QList<IndexedString> ret;
  if(d_func()->m_includePaths) {
    const IncludePathListItem* item = includePathsRepository().itemFromIndex(d_func()->m_includePaths);

    FOREACH_FUNCTION(const IndexedString& include, item->m_includePaths)
      ret << include;
  }
  return ret;
}

void EnvironmentFile::setIncludePaths( const QList<IndexedString>& paths ) {
  ENSURE_WRITE_LOCKED

  QMutexLocker lock(includePathsRepository().mutex());

  if(d_func()->m_includePaths) {
    KDevelop::DynamicItem<IncludePathListItem, true> item = includePathsRepository().dynamicItemFromIndex(d_func()->m_includePaths);
    --item->m_refCount;
    if(!item->m_refCount)
      includePathsRepository().deleteItem(d_func()->m_includePaths);
    d_func_dynamic()->m_includePaths = 0;
  }
  if(!paths.isEmpty()) {
    IncludePathListItem item;
    foreach(const IndexedString &include, paths)
      item.m_includePathsList().append(include);
    d_func_dynamic()->m_includePaths = includePathsRepository().index(item);
    KDevelop::DynamicItem<IncludePathListItem, true> gotItem = includePathsRepository().dynamicItemFromIndex(d_func()->m_includePaths);
    ++gotItem->m_refCount;
  }
}

KDevelop::IndexedString Cpp::EnvironmentFile::headerGuard() const {
  ENSURE_READ_LOCKED
  return d_func()->m_guard;
}

void Cpp::EnvironmentFile::setHeaderGuard(KDevelop::IndexedString guardName) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_guard = guardName;
}

void EnvironmentFile::addMissingIncludeFile(const IndexedString& file)
{
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_missingIncludeFiles.insert(file);
}

const ReferenceCountedStringSet& EnvironmentFile::missingIncludeFiles() const
{
  ENSURE_READ_LOCKED
  return d_func()->m_missingIncludeFiles;
}

void EnvironmentFile::clearMissingIncludeFiles()
{
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_missingIncludeFiles = ReferenceCountedStringSet();
}

void EnvironmentFile::addIncludeFile( const IndexedString& file, const ModificationRevision& modificationTime ) {
//   m_includeFiles.insert(file);
  ENSURE_WRITE_LOCKED

  addModificationRevision(file, modificationTime);
}

void EnvironmentFile::addStrings( const std::set<Utils::BasicSetRepository::Index>& strings ) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_strings += ReferenceCountedStringSet( strings );
}

//The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
void EnvironmentFile::merge( const EnvironmentFile& file ) {
  ENSURE_WRITE_LOCKED
  //We have to read the other file
  ENSURE_FILE_READ_LOCKED(file)

#ifdef DEBUG_LEXERCACHE
  if(debugging()) {
  qCDebug(CPPDUCHAIN) <<  id(this) << ": merging" << id(&file)  << "defined in macros this:" << print(d_func()->m_definedMacroNames)  << "defined macros in other:" << print(file.d_func()->m_definedMacroNames) << "undefined macros in other:" << print(file.d_func()->m_unDefinedMacroNames) << "strings in other:" << print(file.strings());
  }
#endif
  d_func_dynamic()->m_strings = (d_func()->m_strings + (file.d_func()->m_strings - d_func()->m_definedMacroNames)) - d_func()->m_unDefinedMacroNames;

  ///@todo Probably it's more efficient having 2 sets m_changedMacroNames and m_unDefinedMacroNames, where m_unDefinedMacroNames is a subset of m_changedMacroNames.
  //Only add macros to the usedMacros-set that were not defined locally
  d_func_dynamic()->m_usedMacroNames += (file.d_func()->m_usedMacroNames - d_func()->m_definedMacroNames) - d_func()->m_unDefinedMacroNames;

  ///Merge those used macros that were not defined within this environment
  //This is slightly inefficient, would be nicer to have a fast mechanism for this.
  //This is not tragic since usually only few macros are used, and thus few need to be iterated.

  {
    Utils::Set definedMacroNamesSet = d_func()->m_definedMacroNames.set();
    Utils::Set unDefinedMacroNamesSet = d_func()->m_unDefinedMacroNames.set();

    std::set<uint> addUsedMacros;

    ReferenceCountedMacroSet backup = file.d_func()->m_usedMacros;
    Q_ASSERT(backup.set().setIndex() == file.d_func()->m_usedMacros.set().setIndex());

    for(ReferenceCountedMacroSet::Iterator it( file.d_func()->m_usedMacros.iterator() ); it; ++it) {
      const rpp::pp_macro& macro(it.ref());
      if( !definedMacroNamesSet.contains(macro.name.index()) && !unDefinedMacroNamesSet.contains(macro.name.index()) )
        addUsedMacros.insert(it.index());
    }

    //Must not happen, since we hold the locks
    Q_ASSERT(backup.set().setIndex() == file.d_func()->m_usedMacros.set().setIndex());

    if(!addUsedMacros.empty())
      d_func_dynamic()->m_usedMacros += ReferenceCountedMacroSet( addUsedMacros );
  }

  ifDebug( Q_ASSERT(d_func()->m_usedMacroNames.set().count() == d_func()->m_usedMacros.set().count()) );

  ///Add defined macros from the merged file.

  {
    Utils::Set otherDefinedMacroNamesSet = file.d_func()->m_definedMacroNames.set();
    Utils::Set otherUnDefinedMacroNamesSet = file.d_func()->m_unDefinedMacroNames.set();
    //Since merged macros overrule already stored ones, first remove the ones of the same name.

    Cpp::ReferenceCountedStringSet affectedMacros = d_func()->m_definedMacroNames & (file.d_func()->m_definedMacroNames + file.d_func()->m_unDefinedMacroNames);
    ReferenceCountedMacroSet potentiallyRemoveMacros = d_func()->m_definedMacros - file.d_func()->m_definedMacros;

    std::set<uint> removeDefinedMacros;
    #if 0
    if(env && affectedMacros.count() < potentiallyRemoveMacros.count()) {
      //In the environment there is a map that maps from macro-names to macros, which allows us iterating through 'affectedMacros' directly
      for( Cpp::ReferenceCountedStringSet::Iterator it( affectedMacros.iterator() ); it; ++it ) {
        rpp::pp_macro* macro = env->retrieveStoredMacro(*it);
        if(macro)
        {
          uint macroIndex = EnvironmentManager::macroDataRepository.findIndex( MacroRepositoryItemRequest(*macro) );
          if(macroIndex && potentiallyRemoveMacros.containsIndex(macroIndex))
            removeDefinedMacros.insert(macroIndex);
        }
      }
    }else
    #endif

    if(!affectedMacros.isEmpty()) {
      //We have to iterate through all potentially removed macros
      for( ReferenceCountedMacroSet::Iterator it( potentiallyRemoveMacros.iterator() ); it; ++it ) {
        const rpp::pp_macro& macro(it.ref());
        if( affectedMacros.contains( macro.name ) )
          removeDefinedMacros.insert(it.index());
      }
    }

    if(!removeDefinedMacros.empty())
      d_func_dynamic()->m_definedMacros -= ReferenceCountedMacroSet( removeDefinedMacros );
  }

  //Now merge in the new defined macros

  d_func_dynamic()->m_unDefinedMacroNames += file.d_func()->m_unDefinedMacroNames;
  d_func_dynamic()->m_unDefinedMacroNames -= file.d_func()->m_definedMacroNames;
  d_func_dynamic()->m_definedMacroNames -= file.d_func()->m_unDefinedMacroNames;
  d_func_dynamic()->m_definedMacroNames += file.d_func()->m_definedMacroNames;
  d_func_dynamic()->m_definedMacros += file.d_func()->m_definedMacros;

  ///Merge include-files, problems and other stuff
//   m_includeFiles += file.m_includeFiles.set();
  d_func_dynamic()->m_missingIncludeFiles += file.d_func()->m_missingIncludeFiles;

  addModificationRevisions(file.allModificationRevisions());

#ifdef DEBUG_LEXERCACHE
  if(debugging()) {
  qCDebug(CPPDUCHAIN) << id(this) << ": defined macro names in this after merge:" << d_func()->m_definedMacroNames.set().count() << print(d_func()->m_definedMacroNames);
  qCDebug(CPPDUCHAIN) << id(this) << ": defined in this after merge:" << d_func()->m_definedMacros.set().count() << print(d_func()->m_definedMacros);
  ifDebug( Q_ASSERT(d_func()->m_definedMacros.set().count() == d_func()->m_definedMacroNames.set().count()) );
  qCDebug(CPPDUCHAIN) << id(this) << ": undefined in this after merge:" << print(d_func()->m_unDefinedMacroNames);
  qCDebug(CPPDUCHAIN) << id(this) << ": strings in this after merge:" << print(strings());
  qCDebug(CPPDUCHAIN) << id(this) << ": macros used in this after merge:" << print(d_func()->m_usedMacroNames);
  }
#endif
}

size_t EnvironmentFile::hash() const {
  ///@todo remove the (size_t)(this), it is just temporary to make them unique, but will not work with serialization to disk.
  ///Instead, create a hash over the contained strings, and make sure the other hashes work reliably.
  return (size_t)(this); //m_usedMacros.valueHash() + m_usedMacros.idHash() + m_definedMacros.idHash() + m_definedMacros.valueHash() + (size_t)(this)/*+ m_strings.hash()*/; ///@todo is the string-hash needed here?
}

uint EnvironmentFile::identityOffset() const {
  ENSURE_READ_LOCKED
  return d_func()->m_identityOffset;
}

void EnvironmentFile::setIdentityOffset(uint offset) {
  ENSURE_WRITE_LOCKED
  d_func_dynamic()->m_identityOffset = offset;
}


int EnvironmentFile::type() const {
  ENSURE_READ_LOCKED
  return CppParsingEnvironment;
}
