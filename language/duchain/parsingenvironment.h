/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PARSINGENVIRONMENT_H
#define PARSINGENVIRONMENT_H

#include <QtCore/QDateTime>
#include <QtCore/QString>

#include <ksharedptr.h>
#include <kurl.h>

#include "indexedstring.h"

#include "../editor/hashedstring.h"
#include "../languageexport.h"
#include "duchainbase.h"
#include "topducontext.h"

namespace KDevelop
{

class ModificationRevision; //Can be found in  editorintegrator.h
class IndexedTopDUContext;
/**
 * Just an enumeration of a few parsing-environment types.
 * Enumerate a few possible future parsing-environment types.
 * A parsing-environment could also have a type not in this enumeration,
 * the only important thing is that it's unique for the type.
 *
 * The type is needed to match ParsingEnvironment, ParsingEnvironmentFile, and ParsingEnvironmentManager together so they fit.
 * For example the c++-versions would have their specific type.
 *
 * The type must be unique(no other language may have the same type),
 * and the type must be persistent.(it must be same after restarting kdevelop)
 *
 * */
enum ParsingEnvironmentType
{
  StandardParsingEnvironment /**< a basic standard parsing environment */,
  CppParsingEnvironment      /**< a C++ parsing environment */,
  PythonParsingEnvironment   /**< a python parsing environment */,
  CMakeParsingEnvironment    /**< a CMake parsing environment */,
  CSharpParsingEnvironment   /**< a CSharp parsing environment */,
  JavaParsingEnvironment     /**< a JAva parsing environment */,
  RubyParsingEnvironment     /**< a Ruby parsing environment */,
  PhpParsingEnvironment      /**< a PHP parsing environment */
};

/**
 * Use this as base-class to define new parsing-environments.
 *
 * Parsing-environments are needed for languages that create different
 * parsing-results depending on the environment. For example in c++,
 * the environment mainly consists of macros. The include-path can
 * be considered to be a part of the parsing-environment too, because
 * different files may be included using another include-path.
 *
 * The classes have to use the serialization scheme from the duchain.
 * Each class must be registered using REGISTER_DUCHAIN_ITEM with a unique id.
 *
 * \warning Access to this class must be serialized through du-chain locking
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironment
{
  public:
    ParsingEnvironment();
    virtual ~ParsingEnvironment();

    ///@see ParsingEnvironmentType
    virtual int type() const;
};

///The data class used for storing. Use this as base-class of your custom data classes for classes derived from
///ParsingEnvironmentFile
class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironmentFileData : public DUChainBaseData {
  public:
  ParsingEnvironmentFileData() : m_isProxyContext(false), m_features(TopDUContext::VisibleDeclarationsAndContexts) {
  }
  bool m_isProxyContext;
  TopDUContext::Features m_features;
};

/**
 * This represents all information about a specific parsed file that is needed
 * to match the file to a parsing-environment.
 *
 * It is KShared because it is embedded into top-du-contexts and at the same time
 * references may be held by ParsingEnvironmentManager.
 *
 * \warning Access to this class must be serialized through du-chain locking
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironmentFile : public DUChainBase, public KShared
{
  public:
    virtual ~ParsingEnvironmentFile();
    ParsingEnvironmentFile();
    ParsingEnvironmentFile(ParsingEnvironmentFileData& data);

    ///@see ParsingEnvironmentType
    virtual int type() const;

    ///Convenience-function that returns the top-context
    virtual TopDUContext* topContext() const;
    
    ///Should return the indexed version of the top-context
    virtual KDevelop::IndexedTopDUContext indexedTopContext() const = 0;
    
    ///Should return a correctly filled ModificationRevision of the source it was created from.
    virtual ModificationRevision modificationRevision() const = 0;

    ///Should return whether this file matches into the given environment
    virtual bool matchEnvironment(const ParsingEnvironment* environment) const = 0;

    ///Should use language-specific information to decide whether the top-context that has this data attached needs to be reparsed
    virtual bool needsUpdate() const = 0;
    
    /**
     * A language-specific flag used by C++ to mark one context as a proxy of another.
     * If this flag is set on a context, the first imported context should be used for any computations
     * like searches, listing, etc. instead of using this context.
     *
     * The problems should be stored within the proxy-contexts, and optionally there may be
     * any count of imported proxy-contexts imported behind the content-context(This can be used for tracking problems)
     *
     * Note: This flag does not directly change the behavior of the language-independent du-chain.
     */
    bool isProxyContext() const;
    
    ///Sets the flag
    void setIsProxyContext(bool);
    
    ///The features of the attached top-context. They are automatically replicated here by the top-context, so they
    ///are accessible even without the top-context loaded.
    TopDUContext::Features features() const;
    
    ///Returns the parsing-environment informations of all importers of the coupled TopDUContext. This is more efficient than
    ///loading the top-context and finding out, because when a top-context is loaded, also all its recursive imports are loaded
    QList< KSharedPtr<ParsingEnvironmentFile> > importers();

    ///Returns the parsing-environment informations of all imports of the coupled TopDUContext. This is more efficient than
    ///loading the top-context and finding out
    QList< KSharedPtr<ParsingEnvironmentFile> > imports();
    
    
    DUCHAIN_DECLARE_DATA(ParsingEnvironmentFile)
    
  private:
    friend class TopDUContext;
    void setFeatures(TopDUContext::Features);
};

typedef KSharedPtr<ParsingEnvironmentFile> ParsingEnvironmentFilePointer;
}

#endif

