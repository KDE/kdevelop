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

CppPreprocessEnvironment::CppPreprocessEnvironment( rpp::pp* preprocessor, KSharedPtr<Cpp::EnvironmentFile> environmentFile ) : Environment(preprocessor), m_finished(false), m_macroNameSet(&Cpp::EnvironmentManager::m_stringRepository, &Cpp::EnvironmentManager::m_stringRepositoryMutex), m_environmentFile(environmentFile) {
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


rpp::pp_macro* CppPreprocessEnvironment::retrieveMacro(const KDevelop::HashedString& name) const {
    //note all strings that can be affected by macros
    if( !m_environmentFile )
        return rpp::Environment::retrieveMacro(name);

    {
        QMutexLocker l(&Cpp::EnvironmentManager::m_stringRepositoryMutex);
        Utils::BasicSetRepository::Index idx;
        Cpp::EnvironmentManager::m_stringRepository.getItem(name, &idx);
        m_strings.insert(idx);
    }
    rpp::pp_macro* ret = rpp::Environment::retrieveMacro(name);

    if( ret ) //note all used macros
        m_environmentFile->addUsedMacro(*ret);

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

    Cpp::StringSetRepository::LazySet old = m_macroNameSet;
    m_macroNameSet = env->m_macroNameSet;
    env->m_macroNameSet = old;

    rpp::Environment::swapMacros(parentEnvironment);
}

/**
  * Merges the given set of macros into the environment. Does not modify m_environmentFile
  * */
void CppPreprocessEnvironment::merge( const Cpp::MacroSet& macros ) {
    Cpp::MacroSet::Macros::const_iterator endIt = macros.macros().end();
    for( Cpp::MacroSet::Macros::const_iterator it = macros.macros().begin(); it != endIt; ++it ) {
        ///@todo ownership!
        rpp::Environment::setMacro(new rpp::pp_macro(*it)); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)
        m_macroNameSet.insert((*it).name);
    }
}

void CppPreprocessEnvironment::setMacro(rpp::pp_macro* macro) {
    macro->name = Cpp::EnvironmentManager::unifyString(macro->name);
    //Note defined macros
    if( m_environmentFile )
        m_environmentFile->addDefinedMacro(*macro);

    m_macroNameSet.insert(macro->name);
    rpp::Environment::setMacro(macro);
}

int CppPreprocessEnvironment::type() const {
    return KDevelop::CppParsingEnvironment;
}

const Cpp::StringSetRepository::LazySet& CppPreprocessEnvironment::macroNameSet() const {
    return m_macroNameSet;
}

