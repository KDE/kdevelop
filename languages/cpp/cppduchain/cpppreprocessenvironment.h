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

#include <duchain/parsingenvironment.h>
#include "parser/rpp/pp-environment.h"
#include "environmentmanager.h"
#include "cppduchainexport.h"

namespace Cpp {
class MacroSet;
class EnvironmentFile;
}

namespace KDevelop {
class HashedString;
}

class KDEVCPPDUCHAIN_EXPORT CppPreprocessEnvironment : public rpp::Environment, public KDevelop::ParsingEnvironment {
public:
  CppPreprocessEnvironment( rpp::pp* preprocessor, KSharedPtr<Cpp::EnvironmentFile> environmentFile );

  ~CppPreprocessEnvironment();

  void finish();


  virtual rpp::pp_macro* retrieveMacro( const KDevelop::IndexedString& name ) const;

  void setEnvironmentFile( const KSharedPtr<Cpp::EnvironmentFile>& environmentFile );
  KSharedPtr<Cpp::EnvironmentFile> environmentFile() const;

  void swapMacros( Environment* parentEnvironment );

  /**
    * Merges the given set of macros into the environment. Does not modify m_environmentFile.
    * */
  void merge( const Cpp::LazyMacroSet& macros );
  
  ///Merges the macros  from the given EnvironmentFile(including undef macros). Does not modify m_environmentFile.
  void merge( const Cpp::EnvironmentFile* file );

  virtual void setMacro(rpp::pp_macro* macro);

  virtual int type() const;

  ///Does not include the names of undef macros
  const Cpp::LazyStringSet& macroNameSet() const;

  ///Restricts the header branching hash of searched contexts to the given number
  ///(Is only stored here, it is used in the environment-manager)
  ///Set to zero to disable again
  void setIdentityOffsetRestriction(uint value);

  ///Returns the header branching hash restriction that has been set through the function above.
  ///If zero, should be ignored.
  uint identityOffsetRestriction() const;
  
private:
    uint m_identityOffsetRestriction;
    bool m_finished;
    Cpp::LazyStringSet m_macroNameSet;
    mutable std::set<Utils::BasicSetRepository::Index> m_strings;
    mutable KSharedPtr<Cpp::EnvironmentFile> m_environmentFile;
};

#endif
