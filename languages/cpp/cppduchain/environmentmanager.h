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
#include <QtCore/qglobal.h>
#ifdef Q_CC_MSVC
    #include <hash_map>
    #include <hash_set>
#else
    #include <ext/hash_set>
    #include <ext/hash_map>
#endif

#include "hashedstringset.h"
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QMutex>

#include <kurl.h>
#include <ksharedptr.h>

#include <language/duchain/parsingenvironment.h>
#include <language/editor/modificationrevision.h>
#include <language/editor/editorintegrator.h>
#include <language/interfaces/iproblem.h>
#include <language/duchain/topducontext.h>

#include "cppduchainexport.h"
#include "setrepository.h"
#include "parser/rpp/macrorepository.h"

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

/**
 * Simplified matching:
 * Consider this case:
----- File 3.h
#ifndef HAD_3
#define HAD_3
int i;
#endif

----- File 2.h
#include "3.h"
int independent;

----- File test_1.h
#include "2.h"

----- File test_2.h
#include "3.h"
#include "2.h"

----- end

2.h and 3.h both depend on the macro HAD_3. In test_2.h, 3.h is included before 3.h, and when 2.h is included in the next line,
it needs to be completeley re-parsed, because 2.h depends on the macro HAD_3, and that macro is different(it is set) then when it was parsed
in test_1.h.

In practice this is very problematic, because it leads to massive multiple parsing of everything, which costs a lot of resources.
To solve this, "Simplified matching" can be used(it is enabled in CppLanguageSupport).
With simplified matching enabled, every file is always represented by at least 2 parts:
One part for all the headers at the top(we will call it header-section), and one part for the real content below the headers.
From the header-section, we create a proxy-context, that only stores the environment-matching information, and that imports the next created content-context.
The content-context starts right behind the header-context, and its environment-matching information only represents the real content.
The content-context can then be re-used many times, as long as the environment matches, while many different proxy-contexts will be created that represent
different constellations of macro-dependences across included headers.

The downside of this approach:
- Many different includes may be added to the content-context, coming from different proxy-contexts. This is not 100% correct visibility-wise.
- This only works perfectly if the includes are at the top, within one block. If this is not the case, the behavior will partially become like without simplified matching.
The good things:
- The general code-completion should be exactly as good as without simplified matching.
* */

namespace rpp {
  class pp_macro;
  class pp_macro;
  class Environment;
}

namespace Cpp {

struct KDEVCPPDUCHAIN_EXPORT IndexedStringConversion {
  KDevelop::IndexedString toItem(uint index) const {
    return KDevelop::IndexedString(index);
  }
  uint toIndex(const KDevelop::IndexedString& str) const {
    return str.index();
  }
};

//This is expensive, because it needs to copy the complete macro with each conversion
struct KDEVCPPDUCHAIN_EXPORT MacroIndexConversion {
  const rpp::pp_macro& toItem(uint index) const;
  uint toIndex(const rpp::pp_macro& _macro) const;
};

typedef Utils::ConvenientIterator<KDevelop::IndexedString, IndexedStringConversion> StringSetIterator;
typedef Utils::LazySet<KDevelop::IndexedString, IndexedStringConversion> LazyStringSet;

typedef Utils::ConvenientIterator<rpp::pp_macro, MacroIndexConversion> MacroSetIterator;
typedef Utils::LazySet<rpp::pp_macro, MacroIndexConversion> LazyMacroSet;

class EnvironmentManager;
class MacroSet;

DECLARE_LIST_MEMBER_HASH(EnvironmentFileData, m_includePaths, KDevelop::IndexedString)

struct EnvironmentFileData : public KDevelop::DUChainBaseData {
    EnvironmentFileData() {
      initializeAppendedLists();
      m_contentStartLine = 0;
      m_strings = 0;
      m_includeFiles = 0;
      m_missingIncludeFiles = 0;
      m_usedMacros = 0;
      m_usedMacroNames = 0;
      m_definedMacros = 0;
      m_definedMacroNames = 0;
      m_unDefinedMacroNames = 0;
      m_identityOffset = 0;
      m_allModificationTimes = 0;
    }
    EnvironmentFileData(const EnvironmentFileData& rhs) : KDevelop::DUChainBaseData(rhs) {
      initializeAppendedLists();
      copyListsFrom(rhs);
      m_url = rhs.m_url;
      m_modificationTime = rhs.m_modificationTime;
      m_strings = rhs.m_strings; //String-set
      m_includeFiles = rhs.m_includeFiles; //String-set
      m_missingIncludeFiles = rhs.m_missingIncludeFiles; //String-set
      m_usedMacros = rhs.m_usedMacros; //Macro-set
      m_usedMacroNames = rhs.m_usedMacroNames; //String-set
      m_definedMacros = rhs.m_definedMacros; //Macro-set
      m_definedMacroNames = rhs.m_definedMacroNames; //String-set
      m_unDefinedMacroNames = rhs.m_unDefinedMacroNames; //String-set
      m_allModificationTimes = rhs.m_allModificationTimes;
      m_contentStartLine = rhs.m_contentStartLine;
      m_topContext = rhs.m_topContext;
      m_identityOffset = rhs.m_identityOffset;
    }
    
    ~EnvironmentFileData() {
      freeAppendedLists();
    }
    KDevelop::IndexedString m_url;
    KDevelop::IndexedTopDUContext m_topContext;
    KDevelop::ModificationRevision m_modificationTime;
    uint m_identityOffset;
    //Set of all strings that can be affected by macros from outside
    uint m_strings; //String-set
    uint m_includeFiles; //String-set
    uint m_missingIncludeFiles; //String-set
    uint m_usedMacros; //Macro-set
    uint m_usedMacroNames; //String-set
    uint m_definedMacros; //Macro-set
    uint m_definedMacroNames; //String-set
    uint m_unDefinedMacroNames; //String-set
    uint m_allModificationTimes;
    int m_contentStartLine;
    
    START_APPENDED_LISTS_BASE(EnvironmentFileData, KDevelop::DUChainBaseData);
    APPENDED_LIST_FIRST(EnvironmentFileData, KDevelop::IndexedString, m_includePaths);
    END_APPENDED_LISTS(EnvironmentFileData, m_includePaths);
};

class KDEVCPPDUCHAIN_EXPORT EnvironmentFile : public KDevelop::ParsingEnvironmentFile {
  public:
    ///@todo Respect changing include-paths: Check if the included files are still the same(maybe new files are found that were not found before)
    EnvironmentFile( KDevelop::IndexedString url, KDevelop::TopDUContext* topContext );

    EnvironmentFile( EnvironmentFileData& data );
    
    void addStrings( const std::set<Utils::BasicSetRepository::Index>& strings );

    ///If there previously was a macro defined of the same name, it must be given through previousOfSameName, else it can be zero.
    void addDefinedMacro( const rpp::pp_macro& macro, const rpp::pp_macro* previousOfSameName );

    ///the given macro will only make it into usedMacros() if it was not defined in this file
    void usingMacro( const rpp::pp_macro& macro );

    void addIncludeFile( const KDevelop::IndexedString& file, const KDevelop::ModificationRevision& modificationTime );

//     inline bool hasString( const KDevelop::IndexedString& string ) const {
//       return m_strings[string];
//     }

    ///Returns the set of all strings that can affect this file from outside.
    Utils::Set strings() const;
    
    //The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
    void merge( const EnvironmentFile& file );

    bool operator <  ( const EnvironmentFile& rhs ) const {
      return url() < rhs.url();
    }

    size_t hash() const;

    void setTopContext(KDevelop::IndexedTopDUContext context);
    
    virtual KDevelop::IndexedTopDUContext indexedTopContext() const;
    
    KDevelop::IndexedString url() const;

    void setModificationRevision( const KDevelop::ModificationRevision& rev ) ;
    
    virtual KDevelop::ModificationRevision modificationRevision() const;
    
    /**Set of all files with absolute paths, including those included indirectly
     *
     * This by definition also includes this file, so when the count is 1,
     * no other files were included.
     *
     * */
    //const IndexedStringSet& includeFiles() const;

    void addMissingIncludeFile(const KDevelop::IndexedString& file);
    const LazyStringSet& missingIncludeFiles() const;

    void clearMissingIncludeFiles();
  
    ///Set of all defined macros, including those of all deeper included files
    const LazyMacroSet& definedMacros() const;

    ///Set of all macros used from outside, including those used in deeper included files
    const LazyMacroSet& usedMacros() const;

    const LazyStringSet& usedMacroNames() const;
    
    const LazyStringSet& definedMacroNames() const;
    
    ///Set of all macros undefined to the outside
    const LazyStringSet& unDefinedMacroNames() const;
  
    ///Should contain a modification-time for each included-file
    ///Relatively expensive, since a conversion has to be done internally.
    const QMap<KDevelop::IndexedString, KDevelop::ModificationRevision> allModificationTimes() const;

    ///Clears the modification times of all dependencies
    void clearModificationTimes();

    ///Should return the include-paths that were used while parsing this file(as used/found in CppLanguageSupport)
    const QList<KDevelop::IndexedString> includePaths() const;
    void setIncludePaths( const QList<KDevelop::IndexedString>& paths );

    ///Set the first line of actual content, behind includes etc.
    void setContentStartLine(int line);
    int contentStartLine() const;

    /**
    * The identity-value usually only depends on the content of the environment-information. This can be used to separate environments that have the same content.
    * For example a content-environment from a proxy-environment.
    * */
    void setIdentityOffset(uint offset);
    uint identityOffset() const;
    
    virtual bool matchEnvironment(const KDevelop::ParsingEnvironment* environment) const;
    
    virtual bool needsUpdate() const;
    
    enum {
      Identity = 73
    };
    
  private:
    virtual void aboutToSave();
    
    virtual int type() const;

    friend class EnvironmentManager;
    Cpp::LazyStringSet m_includeFiles; //Set of all files with absolute paths
    Cpp::LazyStringSet m_missingIncludeFiles; //Set of relative file-names of missing includes
    Cpp::LazyMacroSet m_usedMacros; //Set of all macros that were used, and were defined outside of this file
    Cpp::LazyStringSet m_usedMacroNames; //Set the names of all used macros
    Cpp::LazyMacroSet m_definedMacros; //Set of all macros that were defined while lexing this file
    Cpp::LazyStringSet m_definedMacroNames;
    Cpp::LazyStringSet m_unDefinedMacroNames; //Set of all macros that were undefined in this file, from outside perspective(not changed ones)

    Utils::Set  m_allModificationTimes; //Set of indices in the modification-time repository
    
    DUCHAIN_DECLARE_DATA(EnvironmentFile)
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

class KDEVCPPDUCHAIN_EXPORT EnvironmentManager {
  public:
    
    static MacroDataRepository macroDataRepository;
    //Set-repository that contains the string-sets
    static Utils::BasicSetRepository stringSetRepository;
    //Set-repository that contains the macro-sets
    static Utils::BasicSetRepository macroSetRepository;
        
    ///See the comment about simplified matching at the top
    static void setSimplifiedMatching(bool simplified);
    static bool isSimplifiedMatching();
    
    static bool m_simplifiedMatching;
};

}

#endif
