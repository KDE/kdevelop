/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "usescollector.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/indexedtype.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <backgroundparser/parsejob.h>
#include <backgroundparser/backgroundparser.h>
#include "../classmemberdeclaration.h"
#include "../abstractfunctiondeclaration.h"
#include "../functiondefinition.h"

using namespace KDevelop;

///@todo make this language-neutral
static Identifier destructorForName(Identifier name) {
  QString str = name.identifier().str();
  if(str.startsWith("~"))
    return Identifier(str);
  return Identifier("~"+str);
}

template<class ImportanceChecker>
void collectImporters(ImportanceChecker& checker, ParsingEnvironmentFile* current, QSet<ParsingEnvironmentFile*>& visited, QSet<ParsingEnvironmentFile*>& collected) {
  if(visited.contains(current))
    return;
  
  visited.insert(current);
  if(checker(current))
    collected.insert(current);
  
  foreach(ParsingEnvironmentFilePointer importer, current->importers())
    if(importer.data())
      collectImporters(checker, importer.data(), visited, collected);
    else
      kDebug() << "missing environment-file, strange";
}

///The returned set does not include the file itself
///@parm visited should be empty on each call, used to prevent endless recursion
void allImportedFiles(ParsingEnvironmentFilePointer file, QSet<IndexedString>& set, QSet<ParsingEnvironmentFilePointer>& visited) {
  foreach(ParsingEnvironmentFilePointer import, file->imports()) {
    if(!visited.contains(import)) {
      visited.insert(import);
      set.insert(import->url());
      allImportedFiles(import, set, visited);
    }
  }
}

void UsesCollector::setCollectConstructors(bool process) {
  m_collectConstructors = process;
}

void UsesCollector::setProcessDeclarations(bool process) {
  m_processDeclarations = process;
}

void UsesCollector::setCollectOverloads(bool collect) {
  m_collectOverloads = collect;
}

void UsesCollector::setCollectDefinitions(bool collect) {
  m_collectDefinitions = collect;
}

QList<IndexedDeclaration> UsesCollector::declarations() {
  return m_declarations;
}

bool UsesCollector::isReady() const {
  return m_waitForUpdate.size() == m_updateReady.size();
}

bool UsesCollector::shouldRespectFile(IndexedString document) {
  return (bool)ICore::self()->projectController()->findProjectForUrl(document.toUrl()) || (bool)ICore::self()->documentController()->documentForUrl(document.toUrl());
}

struct ImportanceChecker {
  ImportanceChecker(UsesCollector& collector) : m_collector(collector) {
  }
  bool operator ()(ParsingEnvironmentFile* file) {
    return m_collector.shouldRespectFile(file->url());
  }
  UsesCollector& m_collector;
};

void UsesCollector::startCollecting() {
    DUChainReadLocker lock(DUChain::lock());

    if(Declaration* decl = m_declaration.data()) {
        
        if(m_collectDefinitions) {
          if(FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(decl)) {
          //Jump from definition to declaration
            Declaration* declaration = def->declaration();
            if(declaration)
              decl = declaration;
          }
        }
      
        ///Collect all overloads into "decls"
        QList<Declaration*> decls;
        
        if(m_collectOverloads && decl->context()->owner() && decl->context()->type() == DUContext::Class) {
          //First find the overridden base, and then all overriders of that base.
          while(Declaration* overridden = DUChainUtils::getOverridden(decl))
            decl = overridden;
          uint maxAllowedSteps = 10000;
          decls += DUChainUtils::getOverriders( decl->context()->owner(), decl, maxAllowedSteps );
          if(maxAllowedSteps == 10000) {
            ///@todo Fail!
          }
        }
        
        decls << decl;
        
        ///Collect all "parsed versions" or forward-declarations etc. here, into allDeclarations
        QSet<IndexedDeclaration> allDeclarations;

        foreach(Declaration* overload, decls) {
          m_declarations = DUChainUtils::collectAllVersions(overload);
          foreach(IndexedDeclaration d, m_declarations) {
            if(!d.data() || d.data()->id() != overload->id())
              continue;
            allDeclarations.insert(d);
            
            if(m_collectConstructors && d.data() && d.data()->internalContext() && d.data()->internalContext()->type() == DUContext::Class) {
              QList<Declaration*> constructors = d.data()->internalContext()->findLocalDeclarations(d.data()->identifier(), SimpleCursor::invalid(), 0, AbstractType::Ptr(), DUContext::OnlyFunctions);
              foreach(Declaration* constructor, constructors) {
                ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(constructor);
                if(classFun && classFun->isConstructor())
                  allDeclarations.insert(IndexedDeclaration(constructor));
              }
              
              Identifier destructorId = destructorForName(d.data()->identifier());
              
              QList<Declaration*> destructors = d.data()->internalContext()->findLocalDeclarations(destructorId, SimpleCursor::invalid(), 0, AbstractType::Ptr(), DUContext::OnlyFunctions);
              foreach(Declaration* destructor, destructors) {
                ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(destructor);
                if(classFun && classFun->isDestructor())
                  allDeclarations.insert(IndexedDeclaration(destructor));
              }
            }
          }
        }

        ///Collect definitions for declarations
        if(m_collectDefinitions) {
          foreach(IndexedDeclaration d, allDeclarations) {
            Declaration* definition = FunctionDefinition::definition(d.data());
            if(definition) {
                kDebug() << "adding definition";
              allDeclarations.insert(IndexedDeclaration(definition));
            }
          }
        }

        m_declarations.clear();
        
        ///Step 4: Copy allDeclarations into m_declarations, build top-context list, etc.
        QList<ReferencedTopDUContext> candidateTopContexts;
        foreach(IndexedDeclaration d, allDeclarations) {
          m_declarations << d;
          m_declarationTopContexts.insert(d.indexedTopContext());
          //We only collect declarations with the same type here.. 
          candidateTopContexts << d.indexedTopContext().data();
        }
        
        ImportanceChecker checker(*this);
        
        QSet<ParsingEnvironmentFile*> visited;
        QSet<ParsingEnvironmentFile*> collected;
        
        kDebug() << "count of source candidate top-contexts:" << candidateTopContexts.size();
        
        ///We use ParsingEnvironmentFile to collect all the relevant importers, because loading those is very cheap, compared
        ///to loading a whole TopDUContext.
        if(decl->inSymbolTable()) {
          //The declaration can only be used from other contexts if it is in the symbol table
          foreach(ReferencedTopDUContext top, candidateTopContexts)
            if(top->parsingEnvironmentFile())
              collectImporters(checker, top->parsingEnvironmentFile().data(), visited, collected);
        }
        if(checker(decl->topContext()->parsingEnvironmentFile().data()))
          collected.insert(decl->topContext()->parsingEnvironmentFile().data());
        
        ///We have all importers now. However since we can tell parse-jobs to also update all their importers, we only need to
        ///update the "root" top-contexts that open the whole set with their imports.
        QSet<IndexedString> rootFiles;
        QSet<IndexedString> allFiles;
        foreach(ParsingEnvironmentFile* importer, collected) {
          QSet<IndexedString> allImports;
          QSet<ParsingEnvironmentFilePointer> visited;
          allImportedFiles(ParsingEnvironmentFilePointer(importer), allImports, visited);
          //Remove all files from the "root" set that are imported by this one
          ///@todo more intelligent
          rootFiles -= allImports;
          allFiles += allImports;
          allFiles.insert(importer->url());
          rootFiles.insert(importer->url());
        }
        
        emit maximumProgressSignal(rootFiles.size());
        maximumProgress(rootFiles.size());
        
        //If we used the AllDeclarationsContextsAndUsesRecursive flag here, we would compute way too much. This we only
        //set the minimum-features selectively on the files we encountered.
        foreach(ParsingEnvironmentFile* file, visited)
          m_staticFeaturesManipulated.insert(file->url());
        m_staticFeaturesManipulated.insert(decl->url());
        
        foreach(IndexedString file, m_staticFeaturesManipulated)
          ParseJob::setStaticMinimumFeatures(file, TopDUContext::AllDeclarationsContextsAndUses);
        
        m_waitForUpdate = rootFiles;
        
        foreach(IndexedString file, rootFiles) {
          kDebug() << "updating root file:" << file.str();
          DUChain::self()->updateContextForUrl(file, TopDUContext::AllDeclarationsContextsAndUses, this);
        }
        
    }else{
        emit maximumProgressSignal(0);
        maximumProgress(0);
    }
}

void UsesCollector::maximumProgress(uint max) {
}

UsesCollector::UsesCollector(IndexedDeclaration declaration) : m_declaration(declaration), m_processDeclarations(true), m_collectOverloads(true), m_collectDefinitions(true), m_collectConstructors(true) {
}

UsesCollector::~UsesCollector() {
  ICore::self()->languageController()->backgroundParser()->revertAllRequests(this);
  
  foreach(IndexedString file, m_staticFeaturesManipulated)
    ParseJob::unsetStaticMinimumFeatures(file, TopDUContext::AllDeclarationsContextsAndUses);
}


void UsesCollector::progress(uint processed, uint total) {
}

void UsesCollector::updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext) {
  
  DUChainReadLocker lock(DUChain::lock());
  
  if(m_waitForUpdate.contains(url)) {
    m_updateReady << url;
    m_checked.clear();
    
    
    emit progressSignal(m_updateReady.size(), m_waitForUpdate.size());
    progress(m_updateReady.size(), m_waitForUpdate.size());
  }
  
  if(!topContext) {
    kDebug() << "failed updating" << url.str();
    return;
  }
  
  if(!topContext->parsingEnvironmentFile()) {
    kDebug() << "missing parsingEnvironmentFile";
    return;
  }
  
  if(!m_staticFeaturesManipulated.contains(url))
    return; //Not interesting
  
  if(!(topContext->features() & TopDUContext::AllDeclarationsContextsAndUses) || topContext->parsingEnvironmentFile()->needsUpdate()) {
      ///@todo With simplified environment-matching, the same file may have been imported multiple times,
      ///while only one of  those was updated. We have to check here whether this file is just such an import,
      ///or whether we work on with it.
      ///@todo We will lose files that were edited right after their update here.
//       kDebug() << "context" << topContext->url().str() << "does not have the required features";
      return;
  }
  
    IndexedTopDUContext indexed(topContext.data());
    if(m_checked.contains(indexed))
      return;
    
    if(!topContext.data()) {
      kDebug() << "updated top-context is zero:" << url.str();
      return;
    }
    
    m_checked.insert(indexed);
    
    if(m_declaration.data() && ((m_processDeclarations && m_declarationTopContexts.contains(indexed)) || 
                                    DUChainUtils::contextHasUse(topContext.data(), m_declaration.data()))) {
      if(!m_processed.contains(topContext->url())) {
        m_processed.insert(topContext->url());
        lock.unlock();
        emit processUsesSignal(topContext);
        processUses(topContext);
        lock.lock();
      }
    }else{
      if(!m_declaration.data())
        kDebug() << "declaration has become invalid";
    }
    
    foreach(DUContext::Import imported, topContext->importedParentContexts())
      if(imported.context() && imported.context()->topContext())
        updateReady(imported.context()->url(), ReferencedTopDUContext(imported.context()->topContext()));
}

IndexedDeclaration UsesCollector::declaration() const {
  return m_declaration;
}
