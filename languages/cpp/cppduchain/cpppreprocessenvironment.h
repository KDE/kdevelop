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

#ifndef CPPPREPROCESSENVIRONMENT_H
#define CPPPREPROCESSENVIRONMENT_H

#include <language/duchain/parsingenvironment.h>
#include "parser/rpp/pp-environment.h"
#include "environmentmanager.h"
#include "cppduchainexport.h"

namespace Cpp {
class MacroSet;
class EnvironmentFile;
}

namespace KDevelop {
class IndexedString;
}

class KDEVCPPDUCHAIN_EXPORT CppPreprocessEnvironment : public rpp::Environment, public KDevelop::ParsingEnvironment {
public:
  CppPreprocessEnvironment( const QExplicitlySharedDataPointer<Cpp::EnvironmentFile>& environmentFile );

  ~CppPreprocessEnvironment();

  ///@param leaveEnvironmentFile Whether the environment-file should be left untouched
  void finishEnvironment(bool leaveEnvironmentFile = false);

  virtual rpp::pp_macro* retrieveMacro( const KDevelop::IndexedString& name, bool isImportant ) const;

  void setEnvironmentFile( const QExplicitlySharedDataPointer<Cpp::EnvironmentFile>& environmentFile );
  QExplicitlySharedDataPointer<Cpp::EnvironmentFile> environmentFile() const;

  void swapMacros( rpp::Environment* parentEnvironment );

  /**
    * Merges the given set of macros into the environment. Does not modify m_environmentFile.
    * */
  void merge( const Cpp::ReferenceCountedMacroSet& macros );
  
  ///Merges the macros  from the given EnvironmentFile(including undef macros). Does not modify m_environmentFile.
  ///@param mergeEnvironments Whether the environment-files should also be merged using Cpp::EnvironmentFile::merge
  void merge( const Cpp::EnvironmentFile* file, bool mergeEnvironments = false );

  virtual void setMacro(rpp::pp_macro* macro);

  virtual int type() const;

  ///Does not include the names of undef macros
  QSet<KDevelop::IndexedString> macroNameSet() const;

  void removeString(const KDevelop::IndexedString& str);
  
  ///Effectively removes the macro from the environment, without noting the change in the environment-file
  ///and other bookkeeping data. This for example allows 'temporarily' shadowing header-guards to enforce the non-empty processing of a file.
  void removeMacro(const KDevelop::IndexedString& macroName);
  
  ///Restricts the header branching hash of searched contexts to the given number
  ///(Is only stored here, it is used in the environment-manager)
  void setIdentityOffsetRestriction(uint value);

  ///Call this to disable a previously enabled identity offset restriction
  void disableIdentityOffsetRestriction();

  ///Whether an identity-offset restriction is set
  bool identityOffsetRestrictionEnabled() const;
  
  ///Returns the header branching hash restriction that has been set through the function above.
  ///Only use it if identityOffsetRestrictionEnabled() returns true
  uint identityOffsetRestriction() const;
  
  static void setRecordOnlyImportantString(bool);
  
private:
    uint m_identityOffsetRestriction;
    bool m_identityOffsetRestrictionEnabled;
    bool m_finished;
    QSet<KDevelop::IndexedString> m_macroNameSet;
    mutable std::set<Utils::BasicSetRepository::Index> m_strings;
    mutable QExplicitlySharedDataPointer<Cpp::EnvironmentFile> m_environmentFile;
};

#endif
