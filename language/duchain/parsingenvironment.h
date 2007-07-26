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

#include <languageexport.h>
#include <ksharedptr.h>
#include <kurl.h>

namespace KDevelop
{

/**
 * This class represents an identified file in the du-chain.
 * Since the du-chain may contain multiple instances of the same file,
 * each parsed in a different way, this is used to identify those files.
 *
 * While searching for an identified file, an identity of 0 means "any".
 * That logic is also implemented by "operator <", so it is automatically
 * implemented by a QMap<IdentifiedFile,...>
 * */
class KDEVPLATFORMLANGUAGE_EXPORT IdentifiedFile
{
  public:
    IdentifiedFile();
    IdentifiedFile(const IdentifiedFile&);

    explicit IdentifiedFile( const KUrl& url , uint identity = 0 );

    ~IdentifiedFile();

    KUrl url() const;

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

  ///Access to all these classes must be serialized through du-chain locking

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
  StandardParsingEnvironment,
  CppParsingEnvironment,
  PythonParsingEnvironment,
  CSharpParsingEnvironment,
  JavaParsingEnvironment,
  RubyParsingEnvironment,
  PhpParsingEnvironment
};

/**
 * Use this as base-class to define new parsing-environments.
 *
 * Parsing-environments are needed for languages that create different
 * parsing-results depending on the environment. For example in c++,
 * the environment mainly consists of macros. The include-path can
 * be considered to be a part of the parsing-environment too, because
 * different files may be included using another include-path.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironment
{
  public:
    virtual ~ParsingEnvironment();

    ///@see ParsingEnvironmentType
    virtual int type() const;
private:
  class ParsingEnvironmentPrivate const *d;
};

/**
 * This represents all information about a specific parsed file that is needed
 * to match the file to a parsing-environment.
 *
 * It is KShared because it is embedded into top-du-contexts and at the same time
 * references may be held by ParsingEnvironmentManager.
 * */


class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironmentFile : public KShared
{
  public:
    virtual ~ParsingEnvironmentFile();

    ///@see ParsingEnvironmentType
    virtual int type() const;

    virtual IdentifiedFile identity() const = 0;
private:
  class ParsingEnvironmentFilePrivate const *d;
};

typedef KSharedPtr<ParsingEnvironmentFile> ParsingEnvironmentFilePointer;


/**
 * This class is responsible for managing parsing-environments and
 * especialla a whole set of ParsingEnvironmentFile instances.
 * It must be implemented case-by-case to work nicely together
 * with specific implementations of ParsingEnvironment.
 *
 * Storing and saving of ParsingEnvironmentFile entries should be implemented within
 * ParsingEnvironmentManager, because that can use additional structural information.
 * */

class KDEVPLATFORMLANGUAGE_EXPORT ParsingEnvironmentManager
{
  public:
    virtual ~ParsingEnvironmentManager();

    ///@see ParsingEnvironmentType
    virtual int type() const;
    virtual void clear();

    ///Add a new file to the manager
    virtual void addFile( ParsingEnvironmentFile* file );
    ///Remove a file from the manager
    virtual void removeFile( ParsingEnvironmentFile* file );

    /**
     * Search for the availability of a file parsed in a given environment
     * */
    virtual ParsingEnvironmentFile* find( const KUrl& url, const ParsingEnvironment* environment );
private:
  class ParsingEnvironmentFilePrivate const *d;
};
}

#endif

