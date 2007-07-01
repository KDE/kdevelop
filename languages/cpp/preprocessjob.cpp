/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Adam Treat <treat@kde.org>
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
#include <QByteArray>
#include <QMutexLocker>

#include <kdebug.h>
#include <klocale.h>

#include <backgroundparser.h>
#include <duchain.h>
#include <duchain/parsingenvironment.h>
#include "duchain/duchainlock.h"
#include "duchain/topducontext.h"

#include "Thread.h"

#include <ilanguage.h>

#include "cpplanguagesupport.h"
#include "cppparsejob.h"
#include "problem.h"
#include "parser/parsesession.h"
#include "parser/ast.h"
#include "parser/parsesession.h"
#include "parser/rpp/pp-environment.h"
#include "parser/rpp/pp-engine.h"
#include "parser/rpp/pp-macro.h"
#include "parser/rpp/preprocessor.h"
#include "environmentmanager.h"

class CppPreprocessEnvironment : public rpp::Environment, public KDevelop::ParsingEnvironment {
    public:
        CppPreprocessEnvironment( rpp::pp* preprocessor, KSharedPtr<Cpp::LexedFile> lexedFile ) : Environment(preprocessor), m_lexedFile(lexedFile) {
            //If this is included from another preprocessed file, take the current macro-set from there.
        }

        
        virtual rpp::pp_macro* retrieveMacro(const QString& name) const {
            ///@todo use a global string-repository
            //note all strings that can be affected by macros
            m_lexedFile->addString(HashedString(name));
            rpp::pp_macro* ret = rpp::Environment::retrieveMacro(name);

            if( ret ) //note all used macros
                m_lexedFile->addUsedMacro(*ret);
            
            return ret;
        }

        /**
         * Merges the given set of macros into the environment. Does not modify m_lexedFile
         * */
        void merge( const Cpp::MacroSet& macros ) {
            Cpp::MacroSet::Macros::const_iterator endIt = macros.macros().end();
            for( Cpp::MacroSet::Macros::const_iterator it = macros.macros().begin(); it != endIt; ++it ) {
                rpp::Environment::setMacro(new rpp::pp_macro(*it)); //Do not use our overridden setMacro(..), because addDefinedMacro(..) is not needed(macro-sets should be merged separately)
            }
        }
        
        virtual void setMacro(rpp::pp_macro* macro) {
            //Note defined macros
            m_lexedFile->addDefinedMacro(*macro);
            rpp::Environment::setMacro(macro);
        }

        virtual int type() const {
            return KDevelop::CppParsingEnvironment;
        }
        
    private:
        mutable KSharedPtr<Cpp::LexedFile> m_lexedFile;
};

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_currentEnvironment(0)
    , m_lexedFile( new Cpp::LexedFile( parent->document(), 0 ) ) ///@todo care about lexer-cache
    , m_success(true)
{
}

CPPParseJob * PreprocessJob::parentJob() const
{
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void PreprocessJob::run()
{
    //kDebug() << k_funcinfo << "Started pp job " << this << " parse " << parentJob()->parseJob() << " parent " << parentJob() << endl;

    kDebug() << "PreprocessJob: preprocessing " << parentJob()->document() << endl;
    
    if (checkAbort())
        return;

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(thread()));

    bool readFromDisk = !parentJob()->contentsAvailableFromEditor();
    parentJob()->setReadFromDisk(readFromDisk);

    QString contents;

    if ( readFromDisk )
    {
        QFile file( parentJob()->document().toLocalFile() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            parentJob()->setErrorMessage(i18n( "Could not open file '%1'", parentJob()->document().toLocalFile() ));
            kWarning( 9007 ) << k_funcinfo << "Could not open file " << parentJob()->document() << " (path " << parentJob()->document().toLocalFile() << ")" << endl;
            return ;
        }

        QByteArray fileData = file.readAll();
        contents = QString::fromUtf8( fileData.constData() );
    //        Q_ASSERT( !contents.isEmpty() );
        file.close();
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
    }

    kDebug( 9007 ) << "===-- PREPROCESSING --===> "
    << parentJob()->document().fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << contents.length()
    << endl;

    if (checkAbort())
        return;

    parentJob()->parseSession()->macros = new rpp::MacroBlock(0);

    rpp::pp preprocessor(this);

    //Eventually initialize the environment with the parent-environment to get it's macros
    m_currentEnvironment = new CppPreprocessEnvironment( &preprocessor, m_lexedFile );

    //If we are included from another preprocessor, copy it's macros
    if( parentJob()->parentPreprocessor() )
            m_currentEnvironment->swapMacros( parentJob()->parentPreprocessor()->m_currentEnvironment );
    
    
    preprocessor.setEnvironment( m_currentEnvironment );
    preprocessor.environment()->enterBlock(parentJob()->parseSession()->macros);

    QString result = preprocessor.processFile(contents, rpp::pp::Data);

    parentJob()->parseSession()->setContents( result.toUtf8() );
    parentJob()->setLexedFile( m_lexedFile.data() );

    if( PreprocessJob* parentPreprocessor = parentJob()->parentPreprocessor() ) {
        //If we are included from another preprocessor, give it back the modified macros,
        parentPreprocessor->m_currentEnvironment->swapMacros( m_currentEnvironment );
        //Merge include-file-set, defined macros, used macros, and string-set
        parentPreprocessor->m_lexedFile->merge(*m_lexedFile);
    }
    kDebug() << "PreprocessJob: finished " << parentJob()->document() << endl;
    
    m_currentEnvironment = 0;
}

rpp::Stream* PreprocessJob::sourceNeeded(QString& fileName, IncludeType type, int sourceLine)
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
    kDebug() << "PreprocessJob " << parentJob()->document() << ": searching for include " << fileName << endl;

    KUrl includedFile = parentJob()->cpp()->findInclude(parentJob()->document(), fileName, type);
    if (includedFile.isValid()) {
        kDebug() << "PreprocessJob " << parentJob()->document() << "(" << m_currentEnvironment->environment().size() << " macros)" << ": found include-file " << fileName << ": " << includedFile << endl;

        KDevelop::TopDUContext* includedContext;
        
        {
            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            includedContext = KDevelop::DUChain::self()->chainForDocument(includedFile, m_currentEnvironment);
        }

        if( includedContext ) {
            kDebug() << "PreprocessJob " << parentJob()->document() << ": took included file from the du-chain " << fileName << endl;
            
            KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
            parentJob()->addIncludedFile(includedContext);
            KDevelop::ParsingEnvironmentFilePointer file = includedContext->parsingEnvironmentFile();
            Cpp::LexedFile* lexedFile = dynamic_cast<Cpp::LexedFile*> (file.data());
            if( lexedFile ) {
                m_currentEnvironment->merge( lexedFile->definedMacros() );
                m_lexedFile->merge( *lexedFile );
            } else {
                kDebug() << "preprocessjob: included file " << includedFile << " found in du-chain, but it has no parse-environment information, or it was not parsed by c++ support" << endl;
            }
        } else {
            kDebug() << "PreprocessJob " << parentJob()->document() << ": no fitting entry in du-chain, parsing" << endl;
            /// Why bother the threadweaver? We need the preprocessed text NOW so we simply parse the
            /// included file right here. Parallel parsing cannot be used here, because we need the
            /// macros before we can continue.

            // Create a slave-job that will take over our macros.
            // It will itself take our macros modify them, copy them back,
            // and merge information into our m_lexedFile

            ///The second parameter is zero because we are in a background-thread and we here
            ///cannot create a slave of the foreground cpp-support-part.
            CPPParseJob slaveJob(includedFile, 0, this);
            slaveJob.parseForeground();

            // Add the included file.
            Q_ASSERT(slaveJob.duChain());
            parentJob()->addIncludedFile(slaveJob.duChain());
        }
        kDebug() << "PreprocessJob " << parentJob()->document() << "(" << m_currentEnvironment->environment().size() << " macros)" << ": file included" << endl;
    } else {
        kDebug() << "PreprocessJob " << parentJob()->document() << ": include not found: " << fileName << endl;
    }

        /*} else {
            kWarning() << k_funcinfo << "Language support disappeared!!" << endl;
        }
    } else {
        kWarning() << k_funcinfo << "Parent job disappeared!!" << endl;
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
        kWarning() << k_funcinfo << "Parent job disappeared!!" << endl;
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
