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
#include <QString>

#include <ksharedptr.h>
#include <kurl.h>

#include "indexedstring.h"

#include "language/editor/hashedstring.h"

#include "language/languageexport.h"

namespace KDevelop
{

class ModificationRevision; //Can be found in  editorintegrator.h

/**
 * This class represents an identified file in the du-chain.
 * Since the du-chain may contain multiple instances of the same file,
 * each parsed in a different way, this is used to identify those files.
 *
 * While searching for an identified file, an identity of 0 means "any".
 * */
class KDEVPLATFORMLANGUAGE_EXPORT IdentifiedFile
{
  public:
    IdentifiedFile();
    IdentifiedFile(const IdentifiedFile&);

    ///These strings should be constructed from KUrls using pathOrUrl
    explicit IdentifiedFile( const IndexedString& url , uint identity = 0 );
    ///These strings should be constructed from KUrls using pathOrUrl
    explicit IdentifiedFile( const HashedString& url , uint identity = 0 );
    explicit IdentifiedFile( const KUrl& url , uint identity = 0 );

    ~IdentifiedFile();

    ///@return url of the file
    IndexedString url() const;

    ///@return A number that identifies the version of this file above the url. If a language does not need multiple versions of a file, it can always return 0
    uint identity() const;

    bool operator<( const IdentifiedFile& rhs ) const;

    IdentifiedFile& operator=( const IdentifiedFile& rhs );

    bool isEmpty() const;

    operator bool() const;

    ///Gives a short description(url identity)
    QString toString() const;

private:
  class IdentifiedFilePrivate* const d;
};

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
 * \warning Access to this class must be serialized through du-chain locking
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironment
{
  public:
    virtual ~ParsingEnvironment();

    ///@see ParsingEnvironmentType
    virtual int type() const;
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
class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironmentFile : public KShared
{
  public:
    virtual ~ParsingEnvironmentFile();

    ///@see ParsingEnvironmentType
    virtual int type() const;

    virtual IdentifiedFile identity() const = 0;

    /**
     * Should return a correctly filled ModificationRevision of the source it was created from. It will be used to decide whether a chain should be removed the the repository.
     *
     * If the time-stamp is invalid and the revision is 0, the file will be automatically deleted.
     * */
    virtual ModificationRevision modificationRevision() const = 0;

    ///Should return whether this file matches into the given environment
    virtual bool matchEnvironment(const ParsingEnvironment* environment) const = 0;

    ///Should use language-specific information to decide whether the top-context that has this data attached needs to be reparsed
    virtual bool needsUpdate() const = 0;
};

typedef KSharedPtr<ParsingEnvironmentFile> ParsingEnvironmentFilePointer;
}

#endif

