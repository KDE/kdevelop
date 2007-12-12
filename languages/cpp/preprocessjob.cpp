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

QList<HashedString> convertFromUrls(const QList<KUrl>& urlList) {
  QList<HashedString> ret;
  foreach(const KUrl& url, urlList)
    ret << Cpp::EnvironmentManager::unifyString(url.prettyUrl());
  return ret;
}

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_currentEnvironment(0)
    , m_environmentFile( new Cpp::EnvironmentFile( parent->document(), 0 ) )
    , m_success(true)
    , m_headerSectionEnded(false)
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

    m_environmentFile->setIncludePaths( convertFromUrls(parentJob()->masterJob()->includePaths()) );
    if(CppLanguageSupport::self()->environmentManager()->isSimplifiedMatching())
        //Make sure that proxy-contexts and content-contexts never have the same identity, even if they have the same content.
        m_environmentFile->setIdentityOffset(1);
    
    {
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());

        if( CppLanguageSupport::self()->environmentManager()->isSimplifiedMatching() ) {
            HashedString u = parentJob()->document();
            m_contentEnvironmentFile = new Cpp::EnvironmentFile(  u, 0 );
//            m_contentEnvironmentFile->setFlags(IdentifiedFile::Content);
        }
    }

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(QThread::currentThread()));

    bool readFromDisk = !parentJob()->contentsAvailableFromEditor();
    parentJob()->setReadFromDisk(readFromDisk);

    QString contents;

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

        QByteArray fileData = file.readAll();
        contents = QString::fromUtf8( fileData.constData() );
    //        Q_ASSERT( !contents.isEmpty() );
        file.close();
        
        m_environmentFile->setModificationRevision( KDevelop::ModificationRevision(fileInfo.lastModified()) );
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

        contents = parentJob()->contentsFromEditor(true);
        m_environmentFile->setModificationRevision( KDevelop::ModificationRevision( fileInfo.lastModified(), parentJob()->revisionToken() ) );
    }

    if(m_contentEnvironmentFile) { //Copy some information from the environment-file to its content-part
        m_contentEnvironmentFile->setModificationRevision(m_environmentFile->modificationRevision());

        ///@todo think about this. Actually a m_contentEnvironmentFile may be accumulated using different include-paths(many different includes are added into it).
        m_contentEnvironmentFile->setIncludePaths(m_environmentFile->includePaths());
    }

    kDebug( 9007 ) << "===-- PREPROCESSING --===> "
    << parentJob()->document().str()
    << "<== readFromDisk:" << readFromDisk
    << "size:" << contents.length()
    << endl;

    if (checkAbort())
        return;

    ///@todo care about ownership of the macros when copying them around. They are all owned by the here opened macro-block.
    parentJob()->parseSession()->macros = new rpp::MacroBlock(0);

    rpp::pp preprocessor(this);

    //Eventually initialize the environment with the parent-environment to get its macros
    m_currentEnvironment = new CppPreprocessEnvironment( &preprocessor, m_environmentFile );

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
        parentJob()->setUpdatingContext( KDevelop::TopDUContextPointer( KDevelop::DUChain::self()->chainForDocument(parentJob()->document(), m_currentEnvironment, m_contentEnvironmentFile ? KDevelop::TopDUContext::ProxyContextFlag : KDevelop::TopDUContext::AnyFlag) ) );
        if( m_contentEnvironmentFile && parentJob()->updatingContext() ) {
            //Must be true, because we explicity passed the flag to chaonForDocument
            Q_ASSERT((parentJob()->updatingContext()->flags() & KDevelop::TopDUContext::ProxyContextFlag));
        }
    }

    preprocessor.setEnvironment( m_currentEnvironment );
    preprocessor.environment()->enterBlock(parentJob()->parseSession()->macros);

    ///@todo preprocessor should work in utf8 too, for performance-reasons
    QString result = preprocessor.processFile(parentJob()->document().str(), rpp::pp::Data, contents);

    foreach (KDevelop::Problem p, preprocessor.problems()) {
      p.setLocationStack(parentJob()->includeStack());
      p.setSource(KDevelop::Problem::Preprocessor);
      KDevelop::DUChain::problemEncountered(p);
    }

    parentJob()->parseSession()->setContents( result.toUtf8(), m_currentEnvironment->takeLocationTable() );
    parentJob()->parseSession()->setUrl( parentJob()->document() );
    parentJob()->setEnvironmentFile( m_environmentFile.data() );

    m_currentEnvironment->finish();

    if(!m_headerSectionEnded) {
        kDebug(9007) << parentJob()->document().str() << ": header-section was note ended";
        headerSectionEnded();
    }
    
    if( m_contentEnvironmentFile ) {
        m_environmentFile->merge(*m_contentEnvironmentFile);
        parentJob()->setContentEnvironmentFile(m_contentEnvironmentFile.data());
    }
    
    if( PreprocessJob* parentPreprocessor = parentJob()->parentPreprocessor() ) {
        //If we are included from another preprocessor, give it back the modified macros,
        parentPreprocessor->m_currentEnvironment->swapMacros( m_currentEnvironment );
        //Merge include-file-set, defined macros, used macros, and string-set
        parentPreprocessor->m_currentEnvironment->environmentFile()->merge(*m_environmentFile);
    }else{
/*        kDebug(9007) << "Macros:";
        for( rpp::Environment::EnvironmentMap::const_iterator it = m_currentEnvironment->environment().begin(); it != m_currentEnvironment->environment().end(); ++it ) {
            kDebug(9007) << (*it)->name.str() << "                  from: " << (*it)->file << ":" << (*it)->sourceLine;
        }*/
    }
    kDebug(9007) << "PreprocessJob: finished" << parentJob()->document().str();

    m_currentEnvironment = 0;
}

void PreprocessJob::headerSectionEnded(rpp::Stream& stream)
{
    if(headerSectionEnded())
        stream.toEnd();
}

bool PreprocessJob::headerSectionEnded() {
    bool ret = false;
    m_headerSectionEnded = true;
    kDebug(9007) << parentJob()->document().str() << "PreprocessJob::headerSectionEnded, " << parentJob()->includedFiles().count() << " included in header-section";
    
    if( m_contentEnvironmentFile ) {
        HashedString u = parentJob()->document();

        ///Find a matching content-context
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock()); //Write-lock because of setFlags below
        KDevelop::TopDUContext* content = KDevelop::DUChain::self()->chainForDocument(u, m_currentEnvironment, KDevelop::TopDUContext::NoFlags);
        if(content) {
            Q_ASSERT(!(content->flags() & KDevelop::TopDUContext::ProxyContextFlag));
            //We have found a content-context that we can use
            parentJob()->setContentContext(KDevelop::TopDUContextPointer(content));

            if( content->parsingEnvironmentFile()->modificationRevision() == KDevelop::EditorIntegrator::modificationRevision(parentJob()->document()) ) {
                //We can completely re-use the specialized context
                m_contentEnvironmentFile = dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data());

                ret = true;
                parentJob()->setUseContentContext(true);
                Q_ASSERT(m_contentEnvironmentFile);
            } else {
                kDebug(9007) << "updating content-context";
                //We will re-use the specialized context, but it needs updating. So we keep processing here.
            }
            //content->parsingEnvironmentFile()->setFlags(content); //It may happen here that a proxy-context is transformed into a normal one
        } else {
            //We need to process the content ourselves
            kDebug(9007) << "could not find a matching content-context";
        }

        m_currentEnvironment->finish();
        
        m_currentEnvironment->setEnvironmentFile(m_contentEnvironmentFile);
    }
    return ret;
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

    QPair<KUrl, KUrl> included = parentJob()->cpp()->findInclude(parentJob()->includePaths(), localPath, fileName, type, from );
    KUrl includedFile = included.first;
    if (includedFile.isValid()) {
        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": found include-file" << fileName << ":" << includedFile;

        KDevelop::TopDUContext* includedContext;

        {
            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            includedContext = KDevelop::DUChain::self()->chainForDocument(includedFile, m_currentEnvironment, m_contentEnvironmentFile ? KDevelop::TopDUContext::ProxyContextFlag : KDevelop::TopDUContext::AnyFlag);
        }

        if( includedContext ) {
            kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": took included file from the du-chain" << fileName;

            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            parentJob()->addIncludedFile(includedContext, sourceLine);
            KDevelop::ParsingEnvironmentFilePointer file = includedContext->parsingEnvironmentFile();
            Cpp::EnvironmentFile* environmentFile = dynamic_cast<Cpp::EnvironmentFile*> (file.data());
            if( environmentFile ) {
                m_currentEnvironment->merge( environmentFile->definedMacros() );
                m_currentEnvironment->environmentFile()->merge( *environmentFile );
            } else {
                kDebug(9007) << "preprocessjob: included file" << includedFile << "found in du-chain, but it has no parse-environment information, or it was not parsed by c++ support";
            }
        } else {
            kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": no fitting entry in du-chain, parsing";
            
            /// Why bother the threadweaver? We need the preprocessed text NOW so we simply parse the
            /// included file right here. Parallel parsing cannot be used here, because we need the
            /// macros before we can continue.

            // Create a slave-job that will take over our macros.
            // It will itself take our macros modify them, copy them back,
            // and merge information into our m_environmentFile

            ///The second parameter is zero because we are in a background-thread and we here
            ///cannot create a slave of the foreground cpp-support-part.
            CPPParseJob slaveJob(includedFile, 0, this);

            slaveJob.setIncludedFromPath(included.second);

            includeStack.append(DocumentCursor(includedFile.prettyUrl(), KTextEditor::Cursor(sourceLine, 0)));
            slaveJob.setIncludeStack(includeStack);

            slaveJob.parseForeground();

            // Add the included file.
            Q_ASSERT(slaveJob.duChain());
            parentJob()->addIncludedFile(slaveJob.duChain(), sourceLine);
        }
        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": file included";
    } else {
        kDebug(9007) << "PreprocessJob" << parentJob()->document().str() << ": include not found:" << fileName;
    }

        /*} else {
            kWarning(9007) << "Language support disappeared!!" ;
        }
    } else {
        kWarning(9007) << "Parent job disappeared!!" ;
    }*/

    return 0;
}

bool PreprocessJob::checkAbort()
{
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

