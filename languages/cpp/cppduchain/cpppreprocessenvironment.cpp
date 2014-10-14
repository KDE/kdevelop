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
#include "debug.h"
#include <language/duchain/problem.h>
#include <parser/rpp/macrorepository.h>

bool onlyRecordImportantMacroUses = true;

void CppPreprocessEnvironment::setRecordOnlyImportantString(bool onlyImportant) {
  onlyRecordImportantMacroUses = onlyImportant;
}

CppPreprocessEnvironment::CppPreprocessEnvironment( const QExplicitlySharedDataPointer<Cpp::EnvironmentFile>& environmentFile )
  : Environment()
  , m_identityOffsetRestriction(0)
  , m_identityOffsetRestrictionEnabled(false)
  , m_finished(false)
  , m_environmentFile(environmentFile)
{
    //If this is included from another preprocessed file, take the current macro-set from there.
    ///NOTE: m_environmentFile may be zero, this must be treated
}

CppPreprocessEnvironment::~CppPreprocessEnvironment() {
    finishEnvironment();
}

void CppPreprocessEnvironment::finishEnvironment(bool leaveEnvironmentFile) {
    if(!m_finished) {
        if(m_environmentFile && !leaveEnvironmentFile)
            m_environmentFile->addStrings(m_strings);
        m_finished = true;
        m_strings.clear();
    }
}

void CppPreprocessEnvironment::removeMacro(const KDevelop::IndexedString& macroName) {
  m_macroNameSet.remove(macroName);
  rpp::pp_macro* m = new rpp::pp_macro;
  m->name = macroName;
  m->defined = false;
  rpp::Environment::setMacro(m);
}

void CppPreprocessEnvironment::removeString(const KDevelop::IndexedString& str) {
  m_strings.erase(str.index());
}

rpp::pp_macro* CppPreprocessEnvironment::retrieveMacro(const KDevelop::IndexedString& name, bool isImportant) const {
    //note all strings that can be affected by macros
    if( !m_environmentFile || (onlyRecordImportantMacroUses && !isImportant) )
        return rpp::Environment::retrieveMacro(name, isImportant);

  //qCDebug(CPPDUCHAIN) << "retrieving macro" << name.str();

    rpp::pp_macro* ret = rpp::Environment::retrieveMacro(name, isImportant);

    if( !ret || (!m_environmentFile->definedMacroNames().contains(name) && !m_environmentFile->unDefinedMacroNames().contains(name)) )
        m_strings.insert(name.index());

    if( ret )
        m_environmentFile->usingMacro(*ret);

    return ret;
}

QExplicitlySharedDataPointer<Cpp::EnvironmentFile> CppPreprocessEnvironment::environmentFile() const {
  return m_environmentFile;
}

void CppPreprocessEnvironment::setEnvironmentFile( const QExplicitlySharedDataPointer<Cpp::EnvironmentFile>& environmentFile ) {
    m_environmentFile = environmentFile;
    m_finished = false;
}

void CppPreprocessEnvironment::swapMacros( rpp::Environment* parentEnvironment ) {
    CppPreprocessEnvironment* env = dynamic_cast<CppPreprocessEnvironment*>(parentEnvironment);
    Q_ASSERT(env);

    qSwap(m_macroNameSet, env->m_macroNameSet);

    rpp::Environment::swapMacros(parentEnvironment);
}

/**
  * Merges the given set of macros into the environment. Does not modify m_environmentFile
  * */
void CppPreprocessEnvironment::merge( const Cpp::ReferenceCountedMacroSet& macros ) {
    for( Cpp::ReferenceCountedMacroSet::Iterator it(macros.iterator()); it; ++it ) {
        rpp::Environment::setMacro(const_cast<rpp::pp_macro*>(&it.ref())); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)

        if( !it.ref().isUndef() )
          m_macroNameSet.insert(it.ref().name);
        else
          m_macroNameSet.remove(it.ref().name);
    }
}

void CppPreprocessEnvironment::merge( const Cpp::EnvironmentFile* file, bool mergeEnvironment ) {
    Cpp::ReferenceCountedMacroSet addedMacros = file->definedMacros() - m_environmentFile->definedMacros();

    if(mergeEnvironment)
      m_environmentFile->merge(*file);

    for( Cpp::ReferenceCountedMacroSet::Iterator it(addedMacros.iterator()); it; ++it )
      rpp::Environment::setMacro(const_cast<rpp::pp_macro*>(&it.ref())); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)

    for( Cpp::ReferenceCountedStringSet::Iterator it = file->definedMacroNames().iterator(); it; ++it ) {
      m_macroNameSet.insert(it.ref());
    }

    //We don't have to care about efficiency too much here, unDefinedMacros should be a rather small set
    for( Cpp::ReferenceCountedStringSet::Iterator it = file->unDefinedMacroNames().iterator(); it; ++it ) {
        rpp::pp_macro* m = new rpp::pp_macro(*it);
        m->defined = false;
        m->m_valueHashValid = false;
        rpp::Environment::setMacro(m); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)
        m_macroNameSet.remove(it.ref());
    }
}

void CppPreprocessEnvironment::setMacro(rpp::pp_macro* macro) {
    rpp::pp_macro* hadMacro = retrieveStoredMacro(macro->name);

    if(hadMacro && hadMacro->fixed) {
      if(hadMacro->defineOnOverride && (hadMacro->file.isEmpty() ||
          (macro->file.length() >= hadMacro->file.length() &&
           memcmp(macro->file.c_str() + (macro->file.length() - hadMacro->file.length()),
                         hadMacro->file.c_str(),
                         hadMacro->file.length()) == 0)))
      {
        // We have to define the macro now, as it is being overridden
        rpp::pp_macro* definedMacro = new rpp::pp_macro(*hadMacro);
        definedMacro->defined = true;
        if(!macro->isRepositoryMacro())
          delete macro;
        macro = definedMacro;
      }else{
        // A fixed macro exists, simply ignore the added macro
        if(!macro->isRepositoryMacro())
          delete macro;
        return;
      }
    }

  //qCDebug(CPPDUCHAIN) << "setting macro" << macro->name.str() << "with body" << macro->definition << "is undef:" << macro->isUndef();
    //Note defined macros
    if( m_environmentFile )
      m_environmentFile->addDefinedMacro(*macro, hadMacro);

    if( !macro->isUndef() )
      m_macroNameSet.insert(macro->name);
    else
      m_macroNameSet.remove(macro->name);

    rpp::Environment::setMacro(macro);
}

int CppPreprocessEnvironment::type() const {
    return KDevelop::CppParsingEnvironment;
}

QSet<KDevelop::IndexedString> CppPreprocessEnvironment::macroNameSet() const {
  return m_macroNameSet;
}

void CppPreprocessEnvironment::setIdentityOffsetRestriction(uint value) {
  m_identityOffsetRestriction = value;
  m_identityOffsetRestrictionEnabled = true;
}

void CppPreprocessEnvironment::disableIdentityOffsetRestriction() {
  m_identityOffsetRestrictionEnabled = false;
}

bool CppPreprocessEnvironment::identityOffsetRestrictionEnabled() const {
  return m_identityOffsetRestrictionEnabled;
}

uint CppPreprocessEnvironment::identityOffsetRestriction() const {
  return m_identityOffsetRestriction;
}
