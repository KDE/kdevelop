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

#include "cpppreprocessenvironment.h"
#include <hashedstring.h>
#include <iproblem.h>
#include <parser/rpp/macrorepository.h>

CppPreprocessEnvironment::CppPreprocessEnvironment( rpp::pp* preprocessor, KSharedPtr<Cpp::EnvironmentFile> environmentFile ) : Environment(preprocessor), m_identityOffsetRestriction(0), m_finished(false), m_macroNameSet(&Cpp::EnvironmentManager::stringSetRepository), m_environmentFile(environmentFile) {
    //If this is included from another preprocessed file, take the current macro-set from there.
    ///NOTE: m_environmentFile may be zero, this must be treated
}

CppPreprocessEnvironment::~CppPreprocessEnvironment() {
    finish();
}

void CppPreprocessEnvironment::finish() {
    if(!m_finished) {
        if(m_environmentFile)
            m_environmentFile->addStrings(m_strings);
        m_finished = true;
        m_strings.clear();
    }
}


rpp::pp_macro* CppPreprocessEnvironment::retrieveMacro(const KDevelop::IndexedString& name) const {
    //note all strings that can be affected by macros
    if( !m_environmentFile )
        return rpp::Environment::retrieveMacro(name);

  //kDebug() << "retrieving macro" << name.str();

    rpp::pp_macro* ret = rpp::Environment::retrieveMacro(name);

    if( !ret || (!m_environmentFile->definedMacroNames().contains(name) && !m_environmentFile->unDefinedMacroNames().contains(name)) )
        m_strings.insert(name.index());
    
    if( ret )
        m_environmentFile->usingMacro(*ret);

    return ret;
}

KSharedPtr<Cpp::EnvironmentFile> CppPreprocessEnvironment::environmentFile() const {
  return m_environmentFile;
}

void CppPreprocessEnvironment::setEnvironmentFile( const KSharedPtr<Cpp::EnvironmentFile>& environmentFile ) {
    m_environmentFile = environmentFile;
    m_finished = false;
}

void CppPreprocessEnvironment::swapMacros( Environment* parentEnvironment ) {
    CppPreprocessEnvironment* env = dynamic_cast<CppPreprocessEnvironment*>(parentEnvironment);
    Q_ASSERT(env);

    Cpp::LazyStringSet old = m_macroNameSet;
    m_macroNameSet = env->m_macroNameSet;
    env->m_macroNameSet = old;

    rpp::Environment::swapMacros(parentEnvironment);
}

/**
  * Merges the given set of macros into the environment. Does not modify m_environmentFile
  * */
void CppPreprocessEnvironment::merge( const Cpp::LazyMacroSet& macros ) {
    for( Cpp::MacroSetIterator it(macros.set().iterator()); it; ++it ) {
        rpp::Environment::setMacro(copyConstantMacro(&it.ref())); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)

        if( !it.ref().isUndef() )
          m_macroNameSet.insert(it.ref().name);
        else
          m_macroNameSet.remove(it.ref().name);
    }
}

void CppPreprocessEnvironment::merge( const Cpp::EnvironmentFile* file ) {
    for( Cpp::MacroSetIterator it(file->definedMacros().iterator()); it; ++it ) {
        rpp::Environment::setMacro(copyConstantMacro(&it.ref())); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)

        m_macroNameSet.insert(it.ref().name);
    }
    for( Cpp::StringSetIterator it = file->unDefinedMacroNames().iterator(); it; ++it ) {
        rpp::pp_dynamic_macro m(*it);
        m.defined = false;
        rpp::Environment::setMacro(makeConstant(&m)); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)

        m_macroNameSet.remove(*it);
    }
}

void CppPreprocessEnvironment::setMacro(rpp::pp_macro* macro) {
  //kDebug() << "setting macro" << macro->name.str() << "with body" << macro->definition << "is undef:" << macro->isUndef();
    //Note defined macros
    if( m_environmentFile )
      m_environmentFile->addDefinedMacro(*macro, retrieveStoredMacro(macro->name));

    if( !macro->isUndef() )
      m_macroNameSet.insert(macro->name);
    else
      m_macroNameSet.remove(macro->name);
    
    rpp::Environment::setMacro(macro);
}

int CppPreprocessEnvironment::type() const {
    return KDevelop::CppParsingEnvironment;
}

const Cpp::LazyStringSet& CppPreprocessEnvironment::macroNameSet() const {
    return m_macroNameSet;
}

void CppPreprocessEnvironment::setIdentityOffsetRestriction(uint value) {
  m_identityOffsetRestriction = value;
}

uint CppPreprocessEnvironment::identityOffsetRestriction() const {
  return m_identityOffsetRestriction;
}
