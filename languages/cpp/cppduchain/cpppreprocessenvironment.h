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


  virtual rpp::pp_macro* retrieveMacro( const KDevelop::HashedString& name ) const;

  void setEnvironmentFile( const KSharedPtr<Cpp::EnvironmentFile>& environmentFile );
  KSharedPtr<Cpp::EnvironmentFile> environmentFile() const;

  void swapMacros( Environment* parentEnvironment );

  /**
    * Merges the given set of macros into the environment. Does not modify m_environmentFile
    * */
  void merge( const Cpp::MacroRepository::LazySet& macros );

  virtual void setMacro(rpp::pp_macro* macro);

  virtual int type() const;

  ///Does not include the names of undef macros
  const Cpp::StringSetRepository::LazySet& macroNameSet() const;

private:
    bool m_finished;
    Cpp::StringSetRepository::LazySet m_macroNameSet;
    mutable std::set<Utils::BasicSetRepository::Index> m_strings;
    mutable KSharedPtr<Cpp::EnvironmentFile> m_environmentFile;
};

#endif
