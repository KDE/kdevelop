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

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QMutex>

#include <language/duchain/parsingenvironment.h>
#include <language/editor/modificationrevision.h>

#include "cppduchainexport.h"
#include "parser/rpp/macrorepository.h"

namespace KDevelop {
  class TopDUContext;
}

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

class CppPreprocessEnvironment;

namespace Cpp {

struct MacroSetRepository : public Utils::BasicSetRepository {
  MacroSetRepository() : Utils::BasicSetRepository("macro sets") {
  }
  virtual void itemRemovedFromSets(uint index) override;
};

///@todo Make string-sets work correctly with IndexedString reference-counting
struct KDEVCPPDUCHAIN_EXPORT IndexedStringConversion {
  KDevelop::IndexedString toItem(uint index) const {
    return KDevelop::IndexedString::fromIndex(index);
  }
  uint toIndex(const KDevelop::IndexedString& str) const {
    return str.index();
  }
};

struct KDEVCPPDUCHAIN_EXPORT MacroIndexConversion {
  const rpp::pp_macro& toItem(uint index) const;
  uint toIndex(const rpp::pp_macro& _macro) const;
};

struct KDEVCPPDUCHAIN_EXPORT StaticStringSetRepository {
  static Utils::BasicSetRepository* repository();
  struct Locker : public QMutexLocker {
    Locker() : QMutexLocker(repository()->mutex()) {
    }
  };
};

struct KDEVCPPDUCHAIN_EXPORT StaticMacroSetRepository {
  static Utils::BasicSetRepository* repository();
  struct Locker : public QMutexLocker {
    Locker() : QMutexLocker(repository()->mutex()) {
    }
  };
};

typedef Utils::StorableSet<KDevelop::IndexedString, IndexedStringConversion, StaticStringSetRepository, true, StaticStringSetRepository::Locker> ReferenceCountedStringSet;
typedef Utils::StorableSet<rpp::pp_macro, MacroIndexConversion, StaticMacroSetRepository, true, StaticMacroSetRepository::Locker> ReferenceCountedMacroSet;

class EnvironmentManager;
class MacroSet;

DECLARE_LIST_MEMBER_HASH(EnvironmentFileData, m_includePaths, KDevelop::IndexedString)

class EnvironmentFileData : public KDevelop::ParsingEnvironmentFileData {
public:

    EnvironmentFileData() {
      m_contentStartLine = 0;
//       m_includeFiles = 0;
      m_identityOffset = 0;
      m_includePaths = 0;
    }
    EnvironmentFileData(const EnvironmentFileData& rhs) : KDevelop::ParsingEnvironmentFileData(rhs) {
      m_url = rhs.m_url;
      m_strings = rhs.m_strings; //String-set
//       m_includeFiles = rhs.m_includeFiles; //String-set
      m_missingIncludeFiles = rhs.m_missingIncludeFiles; //String-set
      m_usedMacros = rhs.m_usedMacros; //Macro-set
      m_usedMacroNames = rhs.m_usedMacroNames; //String-set
      m_definedMacros = rhs.m_definedMacros; //Macro-set
      m_definedMacroNames = rhs.m_definedMacroNames; //String-set
      m_unDefinedMacroNames = rhs.m_unDefinedMacroNames; //String-set
      m_contentStartLine = rhs.m_contentStartLine;
      m_topContext = rhs.m_topContext;
      m_identityOffset = rhs.m_identityOffset;
      m_includePaths = rhs.m_includePaths;
      m_guard = rhs.m_guard;
      m_includePathDependencies = rhs.m_includePathDependencies;
    }
    
    ~EnvironmentFileData() {
    }
    uint m_identityOffset;
    //All the following sets get their reference-count increased whenever put in here
    //Set of all strings that can be affected by macros from outside
    ReferenceCountedStringSet m_strings;
    ReferenceCountedStringSet m_missingIncludeFiles;
    ReferenceCountedMacroSet m_usedMacros;
    ReferenceCountedStringSet m_usedMacroNames;
    ReferenceCountedMacroSet m_definedMacros;
    ReferenceCountedStringSet m_definedMacroNames;
    ReferenceCountedStringSet m_unDefinedMacroNames;
    
    uint m_includePaths; //Index in the internal include-paths repository
    int m_contentStartLine;
    
    //Name of the header-guard macro that protects this file
    KDevelop::IndexedString m_guard;
    
    KDevelop::ModificationRevisionSet m_includePathDependencies;
};

class KDEVCPPDUCHAIN_EXPORT EnvironmentFile : public KDevelop::ParsingEnvironmentFile {
  public:
    ///@todo Respect changing include-paths: Check if the included files are still the same(maybe new files are found that were not found before)
    EnvironmentFile( const KDevelop::IndexedString& url, KDevelop::TopDUContext* topContext );

    EnvironmentFile( EnvironmentFileData& data );
    
    ~EnvironmentFile();
    
    void addStrings( const std::set<Utils::BasicSetRepository::Index>& strings );

    ///If there previously was a macro defined of the same name, it must be given through previousOfSameName, else it can be zero.
    void addDefinedMacro( const rpp::pp_macro& macro, const rpp::pp_macro* previousOfSameName );

    ///the given macro will only make it into usedMacros() if it was not defined in this file
    void usingMacro( const rpp::pp_macro& macro );

    void addIncludeFile( const KDevelop::IndexedString& file, const KDevelop::ModificationRevision& modificationTime );

    ///Returns the set of all strings that can affect this file from outside.
    const ReferenceCountedStringSet& strings() const;
    
    ///The parameter should be a EnvironmentFile that was processed AFTER the content of this file
    ///@param file The file to merge
    ///@warning The file must _not_ be merged yet into the environment when this is called. Also,
    ///         after merging the environment-files a file cannot be merged into the environment any more,
    ///         so this should only be used indirectly through CppPreprocessEnvironment::merge
    void merge( const EnvironmentFile& file );

    bool operator <  ( const EnvironmentFile& rhs ) const {
      return url() < rhs.url();
    }

    size_t hash() const;

    /**Set of all files with absolute paths, including those included indirectly
     *
     * This by definition also includes this file, so when the count is 1,
     * no other files were included.
     *
     * */
    //const IndexedStringSet& includeFiles() const;

    void addMissingIncludeFile(const KDevelop::IndexedString& file);
    const ReferenceCountedStringSet& missingIncludeFiles() const;

    void clearMissingIncludeFiles();
  
    KDevelop::IndexedString headerGuard() const;
    void setHeaderGuard( KDevelop::IndexedString guardName );
    
    ///Set of all defined macros, including those of all deeper included files
    const ReferenceCountedMacroSet& definedMacros() const;

    ///Set of all macros used from outside, including those used in deeper included files
    const ReferenceCountedMacroSet& usedMacros() const;

    const ReferenceCountedStringSet& usedMacroNames() const;
    
    const ReferenceCountedStringSet& definedMacroNames() const;
    
    ///Set of all macros undefined to the outside
    const ReferenceCountedStringSet& unDefinedMacroNames() const;
  
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
    
    virtual bool matchEnvironment(const KDevelop::ParsingEnvironment* environment) const override;
    
    virtual bool needsUpdate(const KDevelop::ParsingEnvironment* environment = 0) const override;
    
    const KDevelop::ModificationRevisionSet& includePathDependencies() const;
    void  setIncludePathDependencies(const KDevelop::ModificationRevisionSet&);
    
    enum {
      Identity = 73
    };
    
  private:
    
    virtual int type() const override;

    friend class EnvironmentManager;

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

typedef QExplicitlySharedDataPointer<EnvironmentFile>  EnvironmentFilePointer;

class KDEVCPPDUCHAIN_EXPORT EnvironmentManager {
  public:
    static EnvironmentManager* self()
    {
      Q_ASSERT_X(m_self, "EnvironmentManager::self()", "call EnvironmentManager::init() before ::self()");
      return m_self;
    }
    /**
     * Initialize the static EnvironmentManager
     */
    static void init();

    MacroDataRepository& macroDataRepository()
    {
      return m_macroDataRepository;
    }
    //Set-repository that contains the string-sets
    Utils::StringSetRepository& stringSetRepository()
    {
      return m_stringSetRepository;
    }
    //Set-repository that contains the macro-sets
    MacroSetRepository& macroSetRepository()
    {
      return m_macroSetRepository;
    }

    ///See the comment about simplified matching at the top
    void setSimplifiedMatching(bool simplified);
    bool isSimplifiedMatching() const {
      return m_simplifiedMatching;
    }
    
    enum MatchingLevel {
      IgnoreGuardsForImporting = 1,
      
      Disabled = IgnoreGuardsForImporting | (1<<5),
      Naive = IgnoreGuardsForImporting | (1<<6),
      Full = 1 << 7
    };
    
    bool ignoreGuardsForImporting() const {
      return matchingLevel() & IgnoreGuardsForImporting;
    }
    
    void setMatchingLevel(MatchingLevel level);
    MatchingLevel matchingLevel() const {
      return m_matchingLevel;
    }

  private:
    EnvironmentManager();
    static EnvironmentManager* m_self;
    MatchingLevel m_matchingLevel;
    bool m_simplifiedMatching;
    //Repository that contains the actual macros, and maps them to indices
    MacroDataRepository m_macroDataRepository;
    //Set-repository that contains the string-sets
    Utils::StringSetRepository m_stringSetRepository;
    //Set-repository that contains the macro-sets
    MacroSetRepository m_macroSetRepository;
};

}

#endif
