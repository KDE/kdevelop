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
#include <duchain/parsingenvironment.h>
#include "duchain/duchainlock.h"
#include "duchain/topducontext.h"
#include <editorintegrator.h>

#include "Thread.h"

#include <ilanguage.h>

#include "cpplanguagesupport.h"
#include "cppparsejob.h"
#include "problem.h"
#include "parser/ast.h"
#include "parser/parsesession.h"
#include "parser/rpp/pp-environment.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/pp-macro.h"
#include "parser/rpp/preprocessor.h"
#include "environmentmanager.h"

class CppPreprocessEnvironment : public rpp::Environment, public KDevelop::ParsingEnvironment {
    public:
        CppPreprocessEnvironment( rpp::pp* preprocessor, KSharedPtr<Cpp::EnvironmentFile> environmentFile ) : Environment(preprocessor), m_environmentFile(environmentFile) {
            //If this is included from another preprocessed file, take the current macro-set from there.
            ///NOTE: m_environmentFile may be zero, this must be treated
        }


        virtual rpp::pp_macro* retrieveMacro(const KDevelop::HashedString& name) const {
            ///@todo use a global string-repository
            //note all strings that can be affected by macros
            if( !m_environmentFile )
                return rpp::Environment::retrieveMacro(name);
            
            m_environmentFile->addString(KDevelop::HashedString(name));
            rpp::pp_macro* ret = rpp::Environment::retrieveMacro(name);

            if( ret ) //note all used macros
                m_environmentFile->addUsedMacro(*ret);

            return ret;
        }

        void setEnvironmentFile( const KSharedPtr<Cpp::EnvironmentFile>& environmentFile ) {
            m_environmentFile = environmentFile;
        }

        /**
         * Merges the given set of macros into the environment. Does not modify m_environmentFile
         * */
        void merge( const Cpp::MacroSet& macros ) {
            Cpp::MacroSet::Macros::const_iterator endIt = macros.macros().end();
            for( Cpp::MacroSet::Macros::const_iterator it = macros.macros().begin(); it != endIt; ++it ) {
                ///@todo ownership!
                rpp::Environment::setMacro(new rpp::pp_macro(*it)); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)
            }
        }

        virtual void setMacro(rpp::pp_macro* macro) {
            //Note defined macros
            if( m_environmentFile )
                m_environmentFile->addDefinedMacro(*macro);
                
            rpp::Environment::setMacro(macro);
        }

        virtual int type() const {
            return KDevelop::CppParsingEnvironment;
        }

    private:
        mutable KSharedPtr<Cpp::EnvironmentFile> m_environmentFile;
};

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_currentEnvironment(0)
    , m_environmentFile( new Cpp::EnvironmentFile( parent->document(), 0 ) )
    , m_success(true)
{
    m_environmentFile->setIncludePaths( parentJob()->masterJob()->includePaths() );
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

    kDebug(9007) << "PreprocessJob: preprocessing" << parentJob()->document();

    if (checkAbort())
        return;

    {
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());

        if( CppLanguageSupport::self()->environmentManager()->isSimplifiedMatching() ) {
            KUrl u = parentJob()->document();
            u.addPath(":content");
            m_contentEnvironmentFile = new Cpp::EnvironmentFile(  u, 0 );
        }
    }

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(thread()));

    bool readFromDisk = !parentJob()->contentsAvailableFromEditor();
    parentJob()->setReadFromDisk(readFromDisk);

    QString contents;

    QFileInfo fileInfo( parentJob()->document().toLocalFile() );
    
    if ( readFromDisk )
    {
        QFile file( parentJob()->document().toLocalFile() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            parentJob()->setErrorMessage(i18n( "Could not open file '%1'", parentJob()->document().toLocalFile() ));
            kWarning( 9007 ) << "Could not open file" << parentJob()->document() << "(path" << parentJob()->document().toLocalFile() << ")" ;
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

    kDebug( 9007 ) << "===-- PREPROCESSING --===> "
    << parentJob()->document().fileName()
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
        parentJob()->setUpdatingContext( KDevelop::DUChain::self()->chainForDocument(parentJob()->document(), m_currentEnvironment) );
        if( m_contentEnvironmentFile && parentJob()->updatingContext() && !(parentJob()->updatingContext()->flags() & KDevelop::TopDUContext::ProxyContextFlag) ) {
            parentJob()->setUpdatingContext(0);
            kDebug() << "Warning: Cannot update a non-proxy context with a proxy-context";
        }
    }

    preprocessor.setEnvironment( m_currentEnvironment );
    preprocessor.environment()->enterBlock(parentJob()->parseSession()->macros);

    QString result = preprocessor.processFile(contents, rpp::pp::Data);

    parentJob()->parseSession()->setContents( result.toUtf8() );
    parentJob()->setEnvironmentFile( m_environmentFile.data() );

    if( m_contentEnvironmentFile ) {
        m_environmentFile->merge(*m_contentEnvironmentFile);
        parentJob()->setContentEnvironmentFile(m_contentEnvironmentFile.data());
    }
    
    if( PreprocessJob* parentPreprocessor = parentJob()->parentPreprocessor() ) {
        //If we are included from another preprocessor, give it back the modified macros,
        parentPreprocessor->m_currentEnvironment->swapMacros( m_currentEnvironment );
        //Merge include-file-set, defined macros, used macros, and string-set
        parentPreprocessor->m_environmentFile->merge(*m_environmentFile);
    }else{
/*        kDebug(9007) << "Macros:";
        for( rpp::Environment::EnvironmentMap::const_iterator it = m_currentEnvironment->environment().begin(); it != m_currentEnvironment->environment().end(); ++it ) {
            kDebug(9007) << (*it)->name.str() << "                  from: " << (*it)->file << ":" << (*it)->sourceLine;
        }*/
    }
    kDebug(9007) << "PreprocessJob: finished" << parentJob()->document();

    m_currentEnvironment = 0;
}

void PreprocessJob::headerSectionEnded(rpp::Stream& stream)
{
    if( m_contentEnvironmentFile ) {
        KUrl u = parentJob()->document();
        u.addPath(":content");

        ///Find a matching content-context
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        KDevelop::TopDUContext* content = KDevelop::DUChain::self()->chainForDocument(parentJob()->document(), m_currentEnvironment);
        if(content) {
            //We have found a content-context that we can use
            parentJob()->setContentContext(content);

            if( content->parsingEnvironmentFile()->modificationRevision() == KDevelop::EditorIntegrator::modificationRevision(parentJob()->document()) ) {
                //We can completely re-use the specialized context
                m_contentEnvironmentFile = dynamic_cast<Cpp::EnvironmentFile*>(content->parsingEnvironmentFile().data());
                Q_ASSERT(m_contentEnvironmentFile);
                stream.toEnd();
                parentJob()->setUseContentContext(true);
            } else {
                //We will re-use the specialized context, but it needs updating. So we keep processing here.
            }
        } else {
            //We need to process the content ourselves
        }

        m_currentEnvironment->setEnvironmentFile(m_contentEnvironmentFile);
    }
}


rpp::Stream* PreprocessJob::sourceNeeded(QString& fileName, IncludeType type, int sourceLine, bool skipCurrentPath)
{
    Q_UNUSED(type)
    Q_UNUSED(sourceLine)

    
    if (checkAbort())
        return 0;

/*    if (CPPParseJob* parent = parentJob()) {
        VALGRIND_CHECK_MEM_IS_DEFINED(parent, sizeof(CPPParseJob));

        if (CppLanguageSupport* lang = parent->cpp()) {
            VALGRIND_CHECK_MEM_IS_DEFINED(lang, sizeof(CppLanguageSupport)); */

//     IncludeFileList::const_iterator it = parentJob()->includedFiles().find(fileName);
//     if (it != parentJob()->includedFiles().constEnd()) {
//         // The file has already been parsed.
//         return 0;
//     }
    kDebug(9007) << "PreprocessJob" << parentJob()->document() << ": searching for include" << fileName;

    KUrl localPath(parentJob()->document());
    localPath.setFileName(QString());
    
    QPair<KUrl, KUrl> included = parentJob()->cpp()->findInclude(parentJob()->includePaths(), localPath, fileName, type, skipCurrentPath ? parentJob()->includedFromPath() : KUrl() );
    KUrl includedFile = included.first;
    if (includedFile.isValid()) {
        kDebug(9007) << "PreprocessJob" << parentJob()->document() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": found include-file" << fileName << ":" << includedFile;

        KDevelop::TopDUContext* includedContext;

        {
            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            includedContext = KDevelop::DUChain::self()->chainForDocument(includedFile, m_currentEnvironment);
        }

        if( includedContext ) {
            kDebug(9007) << "PreprocessJob" << parentJob()->document() << ": took included file from the du-chain" << fileName;

            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            parentJob()->addIncludedFile(includedContext);
            KDevelop::ParsingEnvironmentFilePointer file = includedContext->parsingEnvironmentFile();
            Cpp::EnvironmentFile* environmentFile = dynamic_cast<Cpp::EnvironmentFile*> (file.data());
            if( environmentFile ) {
                m_currentEnvironment->merge( environmentFile->definedMacros() );
                m_environmentFile->merge( *environmentFile );
            } else {
                kDebug(9007) << "preprocessjob: included file" << includedFile << "found in du-chain, but it has no parse-environment information, or it was not parsed by c++ support";
            }
        } else {
            kDebug(9007) << "PreprocessJob" << parentJob()->document() << ": no fitting entry in du-chain, parsing";
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
            slaveJob.parseForeground();

            // Add the included file.
            Q_ASSERT(slaveJob.duChain());
            parentJob()->addIncludedFile(slaveJob.duChain());
        }
        kDebug(9007) << "PreprocessJob" << parentJob()->document() << "(" << m_currentEnvironment->environment().size() << "macros)" << ": file included";
    } else {
        kDebug(9007) << "PreprocessJob" << parentJob()->document() << ": include not found:" << fileName;
    }

        /*} else {
            kWarning() << "Language support disappeared!!" ;
        }
    } else {
        kWarning() << "Parent job disappeared!!" ;
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
        kWarning() << "Parent job disappeared!!" ;
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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
