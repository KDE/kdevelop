/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "preprocessjob.h"

#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QMutexLocker>
#include <QReadWriteLock>

#include <kdebug.h>
#include <klocale.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/codegen/coderepresentation.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/problem.h>

#include <threadweaver/thread.h>

#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "cpplanguagesupport.h"
#include "cppparsejob.h"
#include "parser/ast.h"
#include "parser/parsesession.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/pp-macro.h"
#include "parser/rpp/preprocessor.h"
#include "environmentmanager.h"
#include "cpppreprocessenvironment.h"

#include "cppdebughelper.h"
#include "codegen/unresolvedincludeassistant.h"

// #define ifDebug(x) x
#include "cpputils.h"
#include <rpp/pp-location.h>

const uint maxIncludeDepth = 50;

static QString pathsToString(const Path::List& paths)
{
  QString str;
  foreach( const Path& p, paths ) {
    str += p.pathOrUrl() + "\n";
  }
  return str;
}

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : QObject(parent)
    , m_currentEnvironment(0)
    , m_firstEnvironmentFile( new Cpp::EnvironmentFile( parent->document(), 0 ) )
    , m_success(true)
    , m_headerSectionEnded(false)
    , m_pp(0)
{
}

PreprocessJob::~PreprocessJob() {
  delete m_currentEnvironment;
}

KDevelop::ParsingEnvironment* PreprocessJob::createStandardEnvironment() {
    CppPreprocessEnvironment* ret = new CppPreprocessEnvironment(Cpp::EnvironmentFilePointer());
    ret->merge( CppUtils::standardMacros() );
    
    return ret;
}

CPPParseJob * PreprocessJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void PreprocessJob::foundHeaderGuard(rpp::Stream& stream, KDevelop::IndexedString guardName)
{
  Q_UNUSED(stream);
  
  KDevelop::DUChainWriteLocker lock(KDevelop::DUChain::lock());
  
  m_currentEnvironment->environmentFile()->setHeaderGuard(guardName);
  
  //In naive matching mode, we ignore the dependence on header-guards
  if(Cpp::EnvironmentManager::self()->matchingLevel() <= Cpp::EnvironmentManager::Naive)
    m_currentEnvironment->removeString(guardName);
}

void PreprocessJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread)
{
    if(!ICore::self()->languageController()->language("C++")->languageSupport())
      return;
  
    //If we have a parent, that parent already has locked the parse-lock
    QReadLocker lock(parentJob()->parentPreprocessor() ? 0 : parentJob()->cpp()->language()->parseLock());
    
    //It seems like we cannot influence the actual thread priority in thread-weaver, so for now set it here.
    //It must be low so the GUI stays fluid.
    if(QThread::currentThread())
      QThread::currentThread()->setPriority(QThread::LowestPriority);

    //kDebug(9007) << "Started pp job" << this << "parse" << parentJob()->parseJob() << "parent" << parentJob();

    kDebug(9007) << "PreprocessJob: preprocessing" << parentJob()->document().str();
    if( parentJob()->parentPreprocessor() )
      kDebug(9007) << "PARENT:" << parentJob()->parentPreprocessor()->parentJob()->document().toUrl();

    if (checkAbort())
        return;

    {
      KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
      
      if(Cpp::EnvironmentManager::self()->isSimplifiedMatching()) {
        //Make sure that proxy-contexts and content-contexts never have the same identity, even if they have the same content.
        m_firstEnvironmentFile->setIdentityOffset(1); //Mark the first environment-file as the proxy
        m_secondEnvironmentFile = new Cpp::EnvironmentFile( parentJob()->document(), 0 );
      }
    }

    rpp::pp preprocessor(this);
    m_pp = &preprocessor;

    //Eventually initialize the environment with the parent-environment to get its macros
    m_currentEnvironment = new CppPreprocessEnvironment( m_firstEnvironmentFile );
    //If we are included from another preprocessor, copy its macros
    if( parentJob()->parentPreprocessor() ) {
        m_currentEnvironment->swapMacros( parentJob()->parentPreprocessor()->m_currentEnvironment );
    } else {
        //Insert standard-macros
        m_currentEnvironment->merge( CppUtils::standardMacros() );
        parentJob()->mergeDefines(static_cast<CppPreprocessEnvironment&>(*m_currentEnvironment));
    }
    
    const auto& macroNamesAtBeginning = m_currentEnvironment->macroNameSet();
    
    KDevelop::ParsingEnvironmentFilePointer updatingEnvironmentFile;
    
    {
        ///Find a context that can be updated, and eventually break processing right here, if we notice we don't need to update
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        
        //Make sure we only match proxy-contexts for updating
        m_currentEnvironment->setIdentityOffsetRestriction(m_firstEnvironmentFile->identityOffset());
        
        updatingEnvironmentFile = KDevelop::ParsingEnvironmentFilePointer( KDevelop::DUChain::self()->environmentFileForDocument(parentJob()->document(), m_currentEnvironment, (bool)m_secondEnvironmentFile) );
        
        if(parentJob()->masterJob() == parentJob() && updatingEnvironmentFile) {
          //Check whether we need to run at all, or whether the file is already up to date
          bool fileFeaturesSatisfied = updatingEnvironmentFile->featuresSatisfied(parentJob()->minimumFeatures());
          bool slaveFeaturesSatisfied = updatingEnvironmentFile->featuresSatisfied(parentJob()->slaveMinimumFeatures());
          if(fileFeaturesSatisfied && slaveFeaturesSatisfied) {
            Path localPath = Path(parentJob()->document().str()).parent();
            Cpp::EnvironmentFile* cppEnv = dynamic_cast<Cpp::EnvironmentFile*>(updatingEnvironmentFile.data());
            Q_ASSERT(cppEnv);
            //When possible, we determine whether an update is needed without getting the include-paths, because that's very expensive
            bool needsUpdate = cppEnv->needsUpdate();
              if(!cppEnv->missingIncludeFiles().isEmpty() && !needsUpdate) {
                for(Cpp::ReferenceCountedStringSet::Iterator it = cppEnv->missingIncludeFiles().iterator(); it; ++it)
                  kDebug(9007) << updatingEnvironmentFile->url().str() << "has missing include:" << (*it).str();
                
                readLock.unlock();
                Path::List includePaths = parentJob()->includePathUrls();
                readLock.lock();
                
                needsUpdate = CppUtils::needsUpdate(Cpp::EnvironmentFilePointer(cppEnv), localPath, includePaths);
              }
            
            if(!needsUpdate) {
              parentJob()->setNeedsUpdate(false);
              return;
            }
          } else if (fileFeaturesSatisfied && !slaveFeaturesSatisfied) {
            // This is strange, because importees should not require more features than the importer
            kDebug() << "Slaves require more features than file itself" << parentJob()->slaveMinimumFeatures() << updatingEnvironmentFile->features() << parentJob()->document().toUrl();
          }
        }
    }
    
    //We do this down here, so we eventually can prevent determining the include-paths if nothing needs to be updated
    m_firstEnvironmentFile->setIncludePaths( parentJob()->masterJob()->indexedIncludePaths() );
    
    if(m_secondEnvironmentFile)
      m_secondEnvironmentFile->setIncludePaths(m_firstEnvironmentFile->includePaths());

    if (checkAbort() || !readContents())
        return;

    {
        ///Find a context that can be updated
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        
        KDevelop::ReferencedTopDUContext updating;
        if(updatingEnvironmentFile)
          updating = updatingEnvironmentFile->topContext();

        if(m_secondEnvironmentFile)
          parentJob()->setUpdatingProxyContext( updating ); //The content-context to be updated will be searched later
        else
          parentJob()->setUpdatingContentContext( updating );
      
        if( updating ) {
          //We don't need to change anything, because the EnvironmentFile will be replaced with a new one
          m_updatingEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>( dynamic_cast<Cpp::EnvironmentFile*>(updating->parsingEnvironmentFile().data()) );
        }
        if( m_secondEnvironmentFile && parentJob()->updatingProxyContext() ) {
            // Must be true, because we explicitly passed the flag to chainForDocument
            Q_ASSERT((parentJob()->updatingProxyContext()->parsingEnvironmentFile()->isProxyContext()));
        }
    }
    
    preprocessor.setEnvironment( m_currentEnvironment );

    // Process files in include paths as #include "file"
    // this is analogous to -include command line option of clang/cc
    for (auto i: parentJob()->masterJob()->indexedIncludePaths()) {
        auto include = i.toUrl().toLocalFile();
        QFileInfo info(include);
        if (info.isFile() && CppUtils::isHeader(include)) {
            sourceNeeded(include, IncludeLocal, -1, false);
        }
    }
    PreprocessedContents result = preprocessor.processFile(parentJob()->document().str(), m_contents);

    if(Cpp::EnvironmentManager::self()->matchingLevel() <= Cpp::EnvironmentManager::Naive && !m_headerSectionEnded && !m_firstEnvironmentFile->headerGuard().isEmpty()) {
      if(macroNamesAtBeginning.contains(m_firstEnvironmentFile->headerGuard())) {
        //Remove the header-guard, and re-preprocess, since we don't do real environment-management(We don't allow empty versions)
        // We also have to clear the location-table here, because it already contains 'wrong' contents from the previous preprocessing
        kDebug() << "Re-processing header with the header-guard disabled";
        delete m_currentEnvironment->takeLocationTable();
        m_currentEnvironment->removeMacro(m_firstEnvironmentFile->headerGuard());
        result = preprocessor.processFile(parentJob()->document().str(), m_contents);
      }
    }
    
    if(!m_headerSectionEnded) {
      ifDebug( kDebug(9007) << parentJob()->document().str() << ": header-section was not ended"; )
      headerSectionEndedInternal(0);
    }
    
    m_currentEnvironment->finishEnvironment(m_currentEnvironment->environmentFile() == m_updatingEnvironmentFile);
    
    foreach (KDevelop::ProblemPointer p, preprocessor.problems()) {
      p->setSource(KDevelop::ProblemData::Preprocessor);
      parentJob()->addPreprocessorProblem(p);
    }

    parentJob()->parseSession()->setContents( result, m_currentEnvironment->takeLocationTable() );
    parentJob()->parseSession()->setUrl( parentJob()->document() );

    
    if(m_secondEnvironmentFile)
      parentJob()->setProxyEnvironmentFile( m_firstEnvironmentFile.data() );
    else
      parentJob()->setContentEnvironmentFile( m_firstEnvironmentFile.data() );
    
    if(m_secondEnvironmentFile) {//Copy some information from the environment-file to its content-part
        KDevelop::DUChainWriteLocker readLock(KDevelop::DUChain::lock());
        m_secondEnvironmentFile->setModificationRevision(m_firstEnvironmentFile->modificationRevision());
        if(m_firstEnvironmentFile->headerGuard().isEmpty())
          m_firstEnvironmentFile->setHeaderGuard(m_secondEnvironmentFile->headerGuard());
        else
          m_secondEnvironmentFile->setHeaderGuard(m_firstEnvironmentFile->headerGuard());
    }
    
    if( m_secondEnvironmentFile ) {
        //kDebug(9008) << parentJob()->document().str() << "Merging content-environment file into header environment-file";
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        m_firstEnvironmentFile->merge(*m_secondEnvironmentFile);
        parentJob()->setContentEnvironmentFile(m_secondEnvironmentFile.data());
    }
    
    if( PreprocessJob* parentPreprocessor = parentJob()->parentPreprocessor() ) {
        //If we are included from another preprocessor, give it back the modified macros,
        parentPreprocessor->m_currentEnvironment->swapMacros( m_currentEnvironment );
        //Merge include-file-set, defined macros, used macros, and string-set
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        parentPreprocessor->m_currentEnvironment->environmentFile()->merge(*m_firstEnvironmentFile);
    }else{
/*        kDebug(9007) << "Macros:";
        for( rpp::Environment::EnvironmentMap::const_iterator it = m_currentEnvironment->environment().begin(); it != m_currentEnvironment->environment().end(); ++it ) {
            kDebug(9007) << (*it)->name.str() << "                  from: " << (*it)->file << ":" << (*it)->sourceLine;
        }*/
    }
    ifDebug( kDebug(9007) << "PreprocessJob: finished" << parentJob()->document().str(); )

    m_pp = 0;
    m_currentEnvironment = 0; //Was given to the pp-engine, and will be destroyed by that
}

void PreprocessJob::headerSectionEnded(rpp::Stream& stream)
{
  headerSectionEndedInternal(&stream);
}

void PreprocessJob::headerSectionEndedInternal(rpp::Stream* stream)
{
    bool closeStream = false;
    m_headerSectionEnded = true;

    ifDebug( kDebug(9007) << parentJob()->document().str() << "PreprocessJob::headerSectionEnded, " << parentJob()->includedFiles().count() << " included in header-section" << "upcoming identity-offset:" << m_pp->branchingHash()*19; )
    
    if( m_secondEnvironmentFile ) {
        m_secondEnvironmentFile->setIdentityOffset(m_pp->branchingHash()*19);

        if( stream ) {
          m_secondEnvironmentFile->setContentStartLine(stream->originalInputPosition().line);
          m_firstEnvironmentFile->setContentStartLine(stream->originalInputPosition().line);
        }

        ///Only allow content-contexts that have the same branching hash,
        ///because else they were differently influenced earlier by macros in the header-section
        ///Example: A file that has completely different content depending on an #ifdef

        m_currentEnvironment->setIdentityOffsetRestriction(m_secondEnvironmentFile->identityOffset());
        
        IndexedString u = parentJob()->document();

        ///Find a matching content-context
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());

        KDevelop::ReferencedTopDUContext content;

        if(m_updatingEnvironmentFile)
          content = KDevelop::ReferencedTopDUContext(DUChainUtils::contentContextFromProxyContext(m_updatingEnvironmentFile->topContext()));
        else
          content = KDevelop::DUChain::self()->chainForDocument(u, m_currentEnvironment, false);

        m_currentEnvironment->disableIdentityOffsetRestriction();

        if(content && content->parsingEnvironmentFile()->isProxyContext())
        {
          kWarning() << "Got proxy-context as content-context for file" << content->url().str() << ", not updating";
          content = KDevelop::ReferencedTopDUContext();
        }
        
        if(content) {
            //We have found a content-context that we can use
            parentJob()->setUpdatingContentContext(content);

            Q_ASSERT(!content->parsingEnvironmentFile()->isProxyContext());
            
            Cpp::EnvironmentFilePointer contentEnvironment(dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data()));
            Q_ASSERT(m_updatingEnvironmentFile || contentEnvironment->identityOffset() == m_secondEnvironmentFile->identityOffset());            

            ///@todo think whether localPath is needed
            Path localPath = Path(parentJob()->document().str()).parent();
            
            if(contentEnvironment->matchEnvironment(m_currentEnvironment) && !CppUtils::needsUpdate(contentEnvironment, localPath, parentJob()->includePathUrls()) && (!parentJob()->masterJob()->needUpdateEverything() || parentJob()->masterJob()->wasUpdated(content)) && (content->parsingEnvironmentFile()->featuresSatisfied(parentJob()->minimumFeatures()) && content->parsingEnvironmentFile()->featuresSatisfied(parentJob()->slaveMinimumFeatures())) 
              && Cpp::EnvironmentManager::self()->matchingLevel() != Cpp::EnvironmentManager::Disabled) {
              ///@todo We never keep the duchain while updating now in disabled environment matching mode.
              ///           We don't need it there, and changes in imports may be simply ignored when the keeping is enabled.
              ///           However when full environment management is enabled this is needed, as the same content may be shared for multiple proxy contexts.
                //We can completely re-use the specialized context:
                m_secondEnvironmentFile = dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data());
                m_updatingEnvironmentFile = m_secondEnvironmentFile;
                
                //Merge the macros etc. into the current environment
                m_currentEnvironment->merge( m_secondEnvironmentFile.data() );

                ifDebug( kDebug(9007) << "closing data-stream, body does not need to be processed"; )
                closeStream = true;
                parentJob()->setKeepDuchain(true); //We truncate all following content, so we don't want to update the du-chain.
                Q_ASSERT(m_secondEnvironmentFile);
            } else {
                ifDebug( kDebug(9007) << "updating content-context"; )
                m_updatingEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>(dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data()));
                //We will re-use the specialized context, but it needs updating. So we keep processing here.
                //We don't need to change m_updatingEnvironmentFile, because we will create a new one.
            }
        } else {
            //We need to process the content ourselves
            ifDebug( kDebug(9007) << "could not find a matching content-context"; )
        }

        m_currentEnvironment->finishEnvironment();

        m_currentEnvironment->setEnvironmentFile(m_secondEnvironmentFile);
    }

    if( stream ) {
      if( closeStream )
        stream->toEnd();
    }
}

rpp::Stream* PreprocessJob::sourceNeeded(QString& _fileName, IncludeType type, int sourceLine, bool skipCurrentPath)
{
    Q_UNUSED(type)
    if(0){
      uint currentDepth = 0;
      CPPParseJob* job = parentJob();
      while(job->parentPreprocessor()) {
        ++currentDepth;
        job = job->parentPreprocessor()->parentJob();
      }
      if(currentDepth > maxIncludeDepth) {
        kDebug(9007) << "maximum depth reached while including" << _fileName << "into" << parentJob()->document().str();
        return 0;
      }
    }
    
    KUrl fileNameUrl(_fileName);
    
    TopDUContext::Features slaveMinimumFeatures = parentJob()->slaveMinimumFeatures();
    QString fileName = fileNameUrl.pathOrUrl();
    
    if (checkAbort())
        return 0;

    ifDebug( kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": searching for include" << fileName; )

    Path from;
    if (skipCurrentPath)
      from = parentJob()->includedFromPath();

    QPair<Path, Path> included;
    QFileInfo info(fileName);
    if (info.isFile()) {
        //this is an auto included file i.e. there is no #include directive in the source file for it
        included.first = Path(fileNameUrl.toLocalFile());
        included.second = Path(fileNameUrl.directory());
    } else {
        included = CppUtils::findInclude(parentJob()->includePathUrls(), parentJob()->localPath(), fileName, type, from);
    }

    Path includedFile = included.first;
    if (includedFile.isValid()) {
        const IndexedString indexedFile = includedFile.toIndexed();
        
        {
          //Prevent recursion that may cause a crash
          PreprocessJob* current = this;
          while(current) {
            if(current->parentJob()->document() == indexedFile) {
              KDevelop::ProblemPointer p(new Problem()); ///@todo create special include-problem
              p->setSource(KDevelop::ProblemData::Preprocessor);
              p->setDescription(i18n("File was included recursively from within itself: %1", fileName ));
              p->setFinalLocation(DocumentRange(parentJob()->document(), SimpleRange(sourceLine,0, sourceLine+1,0)));
              parentJob()->addPreprocessorProblem(p);
              return 0;
            }
            current = current->parentJob()->parentPreprocessor();
          }
        }
      
        ifDebug( kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": found include-file" << fileName << ":" << includedFile; )

        KDevelop::ReferencedTopDUContext includedContext;
        bool updateNeeded = false;
        bool updateForbidden = false;

        {
            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            includedContext = KDevelop::DUChain::self()->chainForDocument(indexedFile, m_currentEnvironment, (bool)m_secondEnvironmentFile);
            
            //Check if the same file _is_ one of the parents, and if it is, import it later on
            if(Cpp::EnvironmentManager::self()->matchingLevel() <= Cpp::EnvironmentManager::Naive) {
              
              CPPParseJob* job = parentJob();
              while(job->parentPreprocessor()) {
                job = job->parentPreprocessor()->parentJob();
                if(job->document() == indexedFile) {
                  parentJob()->addDelayedImport(CPPParseJob::LineJobPair(job, sourceLine));
                  return 0;
                }
              }
              
//               if(!includedContext) {
//                 //Check if there is a parsed version that is disabled by its header-guard right now, and enventually use that one.
//                 QList<ParsingEnvironmentFilePointer> allVersions = DUChain::self()->allEnvironmentFiles(indexedFile);
//                 foreach(ParsingEnvironmentFilePointer version, allVersions) {
//                   Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(version.data());
//                   
//                   if(envFile && (envFile->isProxyContext() || !m_secondEnvironmentFile) && !envFile->headerGuard().isEmpty()) {
//                     if(m_currentEnvironment->macroNameSet().contains(envFile->headerGuard())) {
//                       includedContext = envFile->topContext();
//                       
//                       break;
//                     }
//                   }
//                 }
//               }
            }
            
            if(includedContext) {
              Cpp::EnvironmentFilePointer includedEnvironment(dynamic_cast<Cpp::EnvironmentFile*>(includedContext->parsingEnvironmentFile().data()));
              if( includedEnvironment ) {
                updateNeeded = CppUtils::needsUpdate(includedEnvironment, parentJob()->localPath(), parentJob()->includePathUrls());
                //The ForceUpdateRecursive flag is removed before checking for satisfied features, so we can prevent double-updating through "wasUpdated()" below (see *1)
                updateNeeded |= !includedEnvironment->featuresSatisfied((TopDUContext::Features)(slaveMinimumFeatures & (~TopDUContext::ForceUpdateRecursive)));
                //(*1) Do not update again if ForceUpdate is given and the context was already updated during this run
                updateNeeded |= (slaveMinimumFeatures & TopDUContext::ForceUpdate) && !parentJob()->masterJob()->wasUpdated(includedContext.data());
                
                #if 0
                //If header-guards should be ignored, unguard the file
                if(Cpp::EnvironmentManager::self()->ignoreGuardsForImporting() &&
                  !includedEnvironment->headerGuard().isEmpty() && m_currentEnvironment->macroNameSet().contains(includedEnvironment->headerGuard()))
                {
                  m_currentEnvironment->removeMacro(includedEnvironment->headerGuard());
                }
                #endif
                
                if(!includedEnvironment->headerGuard().isEmpty() && m_currentEnvironment->macroNameSet().contains(includedEnvironment->headerGuard())) {
                  updateForbidden = true;
                  kDebug() << "forbidding update of" << includedFile;
                  updateNeeded = false;         
                }
              }
            }
        }

        if( includedContext && (updateForbidden || (!updateNeeded && (!parentJob()->masterJob()->needUpdateEverything() || parentJob()->masterJob()->wasUpdated(includedContext)))) ) {
            ifDebug( kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": took included file from the du-chain" << fileName; )

            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            parentJob()->addIncludedFile(includedContext, sourceLine);
            KDevelop::ParsingEnvironmentFilePointer file = includedContext->parsingEnvironmentFile();
            Cpp::EnvironmentFile* environmentFile = dynamic_cast<Cpp::EnvironmentFile*> (file.data());
            if( environmentFile ) {
                m_currentEnvironment->merge( environmentFile, true );
                ifDebug( kDebug() << "PreprocessJob" << parentJob()->document().str() << "Merging included file into environment-file"; )
            } else {
                ifDebug( kDebug(9007) << "preprocessjob: included file" << includedFile << "found in du-chain, but it has no parse-environment information, or it was not parsed by c++ support"; )
            }
        } else {
            if(updateNeeded)
              kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": need to update" << includedFile;
            else if(parentJob()->masterJob()->needUpdateEverything() && includedContext)
              kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": needUpateEverything, updating" << includedFile;
            else
              kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": no fitting entry for" << includedFile << "in du-chain, parsing";

/*            if( updateNeeded && !parentJob()->masterJob()->needUpdateEverything() ) {
              //When a new include-file was found, that can influence not found declarations in all following encountered contexts, so they all need updating.
              kDebug(9007) << "Marking every following encountered context to be updated";
              parentJob()->masterJob()->setNeedUpdateEverything( true ); //@todo make this a bit more intelligent, instead of updating everything that follows
            }*/
            /// Why bother the threadweaver? We need the preprocessed text NOW so we simply parse the
            /// included file right here. Parallel parsing cannot be used here, because we need the
            /// macros before we can continue.

            // Create a slave-job that will take over our macros.
            // It will itself take our macros modify them, copy them back,
            // and merge information into our m_firstEnvironmentFile

            ///The second parameter is zero because we are in a background-thread and we here
            ///cannot create a slave of the foreground cpp-support-part.
            CPPParseJob* slaveJob = new CPPParseJob(indexedFile, parentJob()->cpp(), this);
            
            slaveJob->setMinimumFeatures(slaveMinimumFeatures);

            slaveJob->setIncludedFromPath(included.second);

            slaveJob->parseForeground();

            // Add the included file.
            if(slaveJob->duChain())
              parentJob()->addIncludedFile(slaveJob->duChain(), sourceLine);
            else
              kDebug(9007) << "parse-job for" << includedFile << "did not return a top-context";
            delete slaveJob;
        }
        ifDebug( kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": file included"; )
    
        {
          DUChainReadLocker lock(DUChain::lock());
          if( m_updatingEnvironmentFile && m_updatingEnvironmentFile->missingIncludeFiles().contains(IndexedString(fileName)) ) {
            //We are finding a file that was not in the include-path last time
            //All following contexts need to be updated, because they may contain references to missing declarations
            parentJob()->masterJob()->setNeedUpdateEverything( true );
            kDebug(9007) << "Marking every following encountered context to be updated";
          }
        }
    
    } else {
        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": include not found:" << fileName;
        Cpp::MissingIncludePathProblem::Ptr p(new Cpp::MissingIncludePathProblem); ///@todo create special include-problem
        p->setSource(KDevelop::ProblemData::Preprocessor);
        p->setDescription(i18n("Included file was not found: %1", fileName ));
        p->setExplanation(i18n("Searched include path:\n%1", pathsToString(parentJob()->includePathUrls())));
        p->setFinalLocation(DocumentRange(parentJob()->document(), SimpleRange(sourceLine,0, sourceLine+1,0)));
        p->setSolutionAssistant(KSharedPtr<KDevelop::IAssistant>(new Cpp::MissingIncludePathAssistant(parentJob()->masterJob()->document(), _fileName)));
        parentJob()->addPreprocessorProblem(KSharedPtr<Problem>::staticCast(p));

        ///@todo respect all the specialties like starting search at a specific path
        ///Before doing that, model findInclude(..) exactly after the standard
        m_firstEnvironmentFile->addMissingIncludeFile(IndexedString(fileName));
    }
    
    return 0;
}

bool PreprocessJob::checkAbort()
{
  if(ICore::self()->shuttingDown()) {
    kDebug(9007) << "The application is shutting down";
    return true;
  }
  
  if(!ICore::self()->languageController()->language("C++") || !ICore::self()->languageController()->language("C++")->languageSupport()) {
    kDebug(9007) << "Environment-manager disappeared" ;
    return true;
  }
    if (CPPParseJob* parent = parentJob()) {
        if (parent->abortRequested()) {
            parent->abortJob();
            m_success = false;
            setStatus(Status_Failed);
            return true;
        }

    } else {
        // What... the parent job got deleted??
        kWarning(9007) << "Parent job disappeared!!" ;
        m_success = false;
        setStatus(Status_Success);
        return true;
    }

    return false;
}

bool PreprocessJob::readContents()
{
  KDevelop::ProblemPointer p = parentJob()->readContents();
  if(p)
  {
    parentJob()->addPreprocessorProblem(p);
    return false;
  }
  m_firstEnvironmentFile->setModificationRevision( parentJob()->contents().modification );

  m_contents = parentJob()->contents().contents;
    
    ifDebug( kDebug( 9007 ) << "===-- PREPROCESSING --===> "
    << parentJob()->document().str()
    << endl; )
    
    return true;
}

bool PreprocessJob::success() const
{
    return m_success;
}

KDevelop::ParsingEnvironment * PreprocessJob::m_standardEnvironment = 0;

const KDevelop::ParsingEnvironment * PreprocessJob::standardEnvironment()
{
  if(!m_standardEnvironment)
    m_standardEnvironment = createStandardEnvironment();

  return m_standardEnvironment;
}

#include "preprocessjob.moc"

