/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
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

//#include <valgrind/memcheck.h>


#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QMutexLocker>

#include <kdebug.h>
#include <klocale.h>

#include <backgroundparser.h>
#include <duchain.h>
#include "duchain/duchainlock.h"
#include "duchain/topducontext.h"
#include <editorintegrator.h>

#include "Thread.h"

#include <ilanguage.h>

#include "cpplanguagesupport.h"
#include "cppparsejob.h"
#include <iproblem.h>
#include "parser/ast.h"
#include "parser/parsesession.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/pp-macro.h"
#include "parser/rpp/preprocessor.h"
#include "environmentmanager.h"
#include "cpppreprocessenvironment.h"

QString urlsToString(const QList<KUrl>& urlList) {
  QString paths;
  foreach( const KUrl& u, urlList )
      paths += u.prettyUrl() + "\n";

  return paths;
}


PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_currentEnvironment(0)
    , m_firstEnvironmentFile( new Cpp::EnvironmentFile( parent->document(), 0 ) )
    , m_success(true)
    , m_headerSectionEnded(false)
    , m_pp(0)
{
}

KDevelop::ParsingEnvironment* PreprocessJob::createStandardEnvironment() {
    CppPreprocessEnvironment* ret = new CppPreprocessEnvironment(0, Cpp::EnvironmentFilePointer());
    ret->merge( CppLanguageSupport::self()->standardMacros() );
            
    return ret;
}

CPPParseJob * PreprocessJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void PreprocessJob::run()
{
    //kDebug(9007) << "Started pp job" << this << "parse" << parentJob()->parseJob() << "parent" << parentJob();

    kDebug(9007) << "PreprocessJob: preprocessing" << parentJob()->document().str();

    if (checkAbort())
        return;

    m_firstEnvironmentFile->setIncludePaths( parentJob()->masterJob()->includePaths() );

    {
      KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
      
    if(CppLanguageSupport::self()->environmentManager()->isSimplifiedMatching()) {
        //Make sure that proxy-contexts and content-contexts never have the same identity, even if they have the same content.
            m_firstEnvironmentFile->setIdentityOffset(1); //Mark the first environment-file as the proxy
            HashedString u = parentJob()->document();
            m_secondEnvironmentFile = new Cpp::EnvironmentFile(  u, 0 );
            m_secondEnvironmentFile->setIncludePaths(m_firstEnvironmentFile->includePaths());
        }
    }

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(QThread::currentThread()));

    bool readFromDisk = !parentJob()->contentsAvailableFromEditor();
    parentJob()->setReadFromDisk(readFromDisk);

    QByteArray contents;

    QString localFile(KUrl(parentJob()->document().str()).toLocalFile());
  
    QFileInfo fileInfo( localFile );
    
    if ( readFromDisk )
    {
        QFile file( localFile );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            parentJob()->setErrorMessage(i18n( "Could not open file '%1'", localFile ));
            kWarning( 9007 ) << "Could not open file" << parentJob()->document().str() << "(path" << localFile << ")" ;
            return ;
        }

        contents = file.readAll(); ///@todo respect local encoding settings. Currently, the file is expected to be utf-8
        
    //        Q_ASSERT( !contents.isEmpty() );
        file.close();
        
        m_firstEnvironmentFile->setModificationRevision( KDevelop::ModificationRevision(fileInfo.lastModified()) );
    }
    else
    {
        KTextEditor::Range range = KTextEditor::Range::invalid();

        //===--- Incremental Parsing!!! yay :) ---===//
        /*if (KDevelop::Core::activeProject() && parentJob->textRangeToParse().isValid()) {
            //if (Core::activeProject()->persistentHash()->retrieveAST( parentJob->document() ))
                // We have an AST which could be used

            if (DUContext* topContext = DUChain::self()->chainForDocument(parentJob->document())) {
                // We have a definition-use chain for this document already
                // Find a recovery point
                DUContext* specific = topContext->findContextIncluding(parentJob->textRangeToParse());
                // Not worthwhile "incrementally" parsing the top level context (i think...?)
                if (specific != topContext)
            }
        }*/

        contents = parentJob()->contentsFromEditor(true).toUtf8();
        m_firstEnvironmentFile->setModificationRevision( KDevelop::ModificationRevision( fileInfo.lastModified(), parentJob()->revisionToken() ) );
    }

    if(m_secondEnvironmentFile) //Copy some information from the environment-file to its content-part
        m_secondEnvironmentFile->setModificationRevision(m_firstEnvironmentFile->modificationRevision());

    kDebug( 9007 ) << "===-- PREPROCESSING --===> "
    << parentJob()->document().str()
    << "<== readFromDisk:" << readFromDisk
    << "size:" << contents.length()
    << endl;

    if (checkAbort())
        return;

    // Create a new macro block if this is the master preprocess-job
    if( parentJob()->masterJob() == parentJob() )
        parentJob()->parseSession()->macros = new rpp::MacroBlock(0);

    rpp::pp preprocessor(this);
    m_pp = &preprocessor;

    //Eventually initialize the environment with the parent-environment to get its macros
    m_currentEnvironment = new CppPreprocessEnvironment( &preprocessor, m_firstEnvironmentFile );

    //If we are included from another preprocessor, copy its macros
    if( parentJob()->parentPreprocessor() ) {
        m_currentEnvironment->swapMacros( parentJob()->parentPreprocessor()->m_currentEnvironment );
    } else {
        //Insert standard-macros
        KDevelop::ParsingEnvironment* standardEnv = createStandardEnvironment();
        m_currentEnvironment->swapMacros( dynamic_cast<CppPreprocessEnvironment*>(standardEnv) );
        delete standardEnv;
    }

    {
        ///Find a context that can be updated
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
      
        KDevelop::TopDUContextPointer updating( KDevelop::DUChain::self()->chainForDocument(parentJob()->document(), m_currentEnvironment, m_secondEnvironmentFile ? KDevelop::TopDUContext::ProxyContextFlag : KDevelop::TopDUContext::AnyFlag) );

        if(m_secondEnvironmentFile)
          parentJob()->setUpdatingProxyContext( updating ); //The content-context to be updated will be searched later
        else
          parentJob()->setUpdatingContentContext( updating );
      
        if( updating ) {
          m_updatingEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>( dynamic_cast<Cpp::EnvironmentFile*>(updating->parsingEnvironmentFile().data()) );
        }
        if( m_secondEnvironmentFile && parentJob()->updatingProxyContext() ) {
            //Must be true, because we explicity passed the flag to chaonForDocument
            Q_ASSERT((parentJob()->updatingProxyContext()->flags() & KDevelop::TopDUContext::ProxyContextFlag));
        }
    }

    preprocessor.setEnvironment( m_currentEnvironment );

    preprocessor.environment()->enterBlock(parentJob()->masterJob()->parseSession()->macros);

    QByteArray result = preprocessor.processFile(parentJob()->document().str(), rpp::pp::Data, contents);

    m_currentEnvironment->finish();
    
    if(!m_headerSectionEnded) {
        kDebug(9007) << parentJob()->document().str() << ": header-section was not ended";
      headerSectionEndedInternal(0);
    }
    
    foreach (KDevelop::ProblemPointer p, preprocessor.problems()) {
      p->setLocationStack(parentJob()->includeStack());
      p->setSource(KDevelop::Problem::Preprocessor);
      parentJob()->addPreprocessorProblem(p);
    }

    parentJob()->parseSession()->setContents( result, m_currentEnvironment->takeLocationTable() );
    parentJob()->parseSession()->setUrl( parentJob()->document() );

    if(m_secondEnvironmentFile)
      parentJob()->setProxyEnvironmentFile( m_firstEnvironmentFile.data() );
    else
      parentJob()->setContentEnvironmentFile( m_firstEnvironmentFile.data() );

    if( m_secondEnvironmentFile ) {
        //kDebug(9008) << parentJob()->document().str() << "Merging content-environment file into header environment-file";
        m_firstEnvironmentFile->merge(*m_secondEnvironmentFile);
        parentJob()->setContentEnvironmentFile(m_secondEnvironmentFile.data());
    }
    
    if( PreprocessJob* parentPreprocessor = parentJob()->parentPreprocessor() ) {
        //If we are included from another preprocessor, give it back the modified macros,
        parentPreprocessor->m_currentEnvironment->swapMacros( m_currentEnvironment );
        //Merge include-file-set, defined macros, used macros, and string-set
        parentPreprocessor->m_currentEnvironment->environmentFile()->merge(*m_firstEnvironmentFile);
    }else{
/*        kDebug(9007) << "Macros:";
        for( rpp::Environment::EnvironmentMap::const_iterator it = m_currentEnvironment->environment().begin(); it != m_currentEnvironment->environment().end(); ++it ) {
            kDebug(9007) << (*it)->name.str() << "                  from: " << (*it)->file << ":" << (*it)->sourceLine;
        }*/
    }
    kDebug(9007) << "PreprocessJob: finished" << parentJob()->document().str();

    m_currentEnvironment = 0;
    m_pp = 0;
}

void PreprocessJob::headerSectionEnded(rpp::Stream& stream)
{
  headerSectionEndedInternal(&stream);
}

bool PreprocessJob::needsUpdate(const Cpp::EnvironmentFilePointer& file, const KUrl& localPath, const KUrl::List& includePaths)
{
  if(file->modificationRevision() != KDevelop::EditorIntegrator::modificationRevision(file->url()))
    return true;

    ///@todo model the include-logic exactly, and check modification-revisions of all actually included files
  for( Cpp::StringSetRepository::Iterator it = file->missingIncludeFiles().iterator(); it; ++it ) {

    QPair<KUrl, KUrl> included = parentJob()->cpp()->findInclude( includePaths, localPath, (*it).str(), IncludeLocal, KUrl(), true );
    if(!included.first.isEmpty()) {
      return true;
    }
  }
  
  return false;
}

void PreprocessJob::headerSectionEndedInternal(rpp::Stream* stream)
{
    bool closeStream = false;
    m_headerSectionEnded = true;

    kDebug(9007) << parentJob()->document().str() << "PreprocessJob::headerSectionEnded, " << parentJob()->includedFiles().count() << " included in header-section";
    
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
        
        HashedString u = parentJob()->document();

        ///Find a matching content-context
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());

        KDevelop::TopDUContext* content = KDevelop::DUChain::self()->chainForDocument(u, m_currentEnvironment, KDevelop::TopDUContext::NoFlags);

        m_currentEnvironment->setIdentityOffsetRestriction(0);

        if(content) {
            Q_ASSERT(!(content->flags() & KDevelop::TopDUContext::ProxyContextFlag));
            //We have found a content-context that we can use
            parentJob()->setUpdatingContentContext(KDevelop::TopDUContextPointer(content));

            Cpp::EnvironmentFilePointer contentEnvironment(dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data()));

            ///@todo think whether localPath is needed
            KUrl localPath(parentJob()->document().str());
            localPath.setFileName(QString());
                
            if(!needsUpdate(contentEnvironment, localPath, parentJob()->includePathUrls()) && (!parentJob()->masterJob()->needUpdateEverything() || parentJob()->masterJob()->wasUpdated(content)) && (!parentJob()->needUses() || content->hasUses()) ) {
                //We can completely re-use the specialized context:
                m_secondEnvironmentFile = dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data());

                closeStream = true;
                parentJob()->setKeepDuchain(true); //We truncate all following content, so we don't want to update the du-chain.
                Q_ASSERT(m_secondEnvironmentFile);
            } else {
                kDebug(9007) << "updating content-context";
                m_updatingEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>(dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data()));
                //We will re-use the specialized context, but it needs updating. So we keep processing here.
            }
        } else {
            //We need to process the content ourselves
            kDebug(9007) << "could not find a matching content-context";
        }

        m_currentEnvironment->finish();

        m_currentEnvironment->setEnvironmentFile(m_secondEnvironmentFile);
    }

    if( stream ) {
      if( closeStream )
        stream->toEnd();
    }
}

rpp::Stream* PreprocessJob::sourceNeeded(QString& fileName, IncludeType type, int sourceLine, bool skipCurrentPath)
{
    Q_UNUSED(type)
    
    if (checkAbort())
        return 0;

    kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": searching for include" << fileName;

    KUrl localPath(parentJob()->document().str());
    localPath.setFileName(QString());
    QStack<DocumentCursor> includeStack = parentJob()->includeStack();

    KUrl from;
    if (skipCurrentPath)
      from = parentJob()->includedFromPath();

    QPair<KUrl, KUrl> included = parentJob()->cpp()->findInclude( parentJob()->includePathUrls(), localPath, fileName, type, from );
    KUrl includedFile = included.first;
    if (includedFile.isValid()) {
        
        if( m_updatingEnvironmentFile && m_updatingEnvironmentFile->missingIncludeFiles().contains(HashedString(fileName)) ) {
          //We are finding a file that was not in the include-path last time
          //All following contexts need to be updated, because they may contain references to missing declarations
          parentJob()->masterJob()->setNeedUpdateEverything( true );
          kDebug(9007) << "Marking every following encountered context to be updated";
        }

        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": found include-file" << fileName << ":" << includedFile;

        KDevelop::TopDUContext* includedContext;
        bool updateNeeded = false;

        {
            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            includedContext = KDevelop::DUChain::self()->chainForDocument(includedFile, m_currentEnvironment, m_secondEnvironmentFile ? KDevelop::TopDUContext::ProxyContextFlag : KDevelop::TopDUContext::AnyFlag);
            if(includedContext) {
              Cpp::EnvironmentFilePointer includedEnvironment(dynamic_cast<Cpp::EnvironmentFile*>(includedContext->parsingEnvironmentFile().data()));
              if( includedEnvironment )
                updateNeeded = needsUpdate(includedEnvironment, localPath, parentJob()->includePathUrls());
            }
        }

        if( includedContext && !updateNeeded && (!parentJob()->masterJob()->needUpdateEverything() || parentJob()->masterJob()->wasUpdated(includedContext)) ) {
            kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": took included file from the du-chain" << fileName;

            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            parentJob()->addIncludedFile(includedContext, sourceLine);
            KDevelop::ParsingEnvironmentFilePointer file = includedContext->parsingEnvironmentFile();
            Cpp::EnvironmentFile* environmentFile = dynamic_cast<Cpp::EnvironmentFile*> (file.data());
            if( environmentFile ) {
                m_currentEnvironment->merge( environmentFile->definedMacros() );
                kDebug() << "PreprocessJob" << parentJob()->document().str() << "Merging included file into environment-file";
                m_currentEnvironment->environmentFile()->merge( *environmentFile );
            } else {
                kDebug(9007) << "preprocessjob: included file" << includedFile << "found in du-chain, but it has no parse-environment information, or it was not parsed by c++ support";
            }
        } else {
            kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": no fitting entry in du-chain, parsing";

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
            CPPParseJob* slaveJob = new CPPParseJob(includedFile, 0, this);

            slaveJob->setIncludedFromPath(included.second);

            includeStack.append(DocumentCursor(parentJob()->document(), KTextEditor::Cursor(sourceLine, 0)));
            slaveJob->setIncludeStack(includeStack);

            slaveJob->parseForeground();

            // Add the included file.
            Q_ASSERT(slaveJob->duChain());
            parentJob()->addIncludedFile(slaveJob->duChain(), sourceLine);
            delete slaveJob;
        }
        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": file included";
    } else {
        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": include not found:" << fileName;
        KDevelop::ProblemPointer p(new Problem()); ///@todo create special include-problem
        p->setSource(KDevelop::Problem::Preprocessor);
        p->setDescription(i18n("Included file was not found: %1", fileName ));
        p->setExplanation(i18n("Searched include path:\n%1", urlsToString(parentJob()->includePathUrls())));
        p->setFinalLocation(DocumentRange(parentJob()->document().str(), KTextEditor::Cursor(sourceLine,0), KTextEditor::Cursor(sourceLine+1,0)));
        p->setLocationStack(parentJob()->includeStack());
        parentJob()->addPreprocessorProblem(p);

        ///@todo respect all the specialties like starting search at a specific path
        ///Before doing that, model findInclude(..) exactly after the standard
        m_firstEnvironmentFile->addMissingIncludeFile(HashedString(fileName));
    }

    return 0;
}

bool PreprocessJob::checkAbort()
{
  if(!CppLanguageSupport::self()->environmentManager()) {
    kDebug(9007) << "Environment-manager disappeared" ;
    return true;
  }
    if (CPPParseJob* parent = parentJob()) {
        if (parent->abortRequested()) {
            parent->abortJob();
            m_success = false;
            setFinished(true);
            return true;
        }

    } else {
        // What... the parent job got deleted??
        kWarning(9007) << "Parent job disappeared!!" ;
        m_success = false;
        setFinished(true);
        return true;
    }

    return false;
}

bool PreprocessJob::success() const
{
    return m_success;
}

#include "preprocessjob.moc"

