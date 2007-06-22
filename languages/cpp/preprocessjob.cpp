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
#include "lexercache.h"
#include <duchain.h>

class CppPreprocessEnvironment : public rpp::Environment {
    public:
        CppPreprocessEnvironment( rpp::pp* preprocessor, KSharedPtr<Cpp::CachedLexedFile> lexedFile ) : Environment(preprocessor), m_lexedFile(lexedFile) {
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

        
        virtual void setMacro(rpp::pp_macro* macro) {
            //Note defined macros
            m_lexedFile->addDefinedMacro(*macro);
            rpp::Environment::setMacro(macro);
        }

    private:
        mutable KSharedPtr<Cpp::CachedLexedFile> m_lexedFile;
};

PreprocessJob::PreprocessJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_currentEnvironment(0)
    , m_cachedLexedFile( new Cpp::CachedLexedFile( parent->document(), 0 ) ) ///@todo care about lexer-cache
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
    m_currentEnvironment = new CppPreprocessEnvironment( &preprocessor, m_cachedLexedFile );

    //If we are included from another preprocessor, copy it's macros
    if( parentJob()->parentPreprocessor() )
            m_currentEnvironment->takeMacros( parentJob()->parentPreprocessor()->m_currentEnvironment );
    
    
    preprocessor.setEnvironment( m_currentEnvironment );
    preprocessor.environment()->enterBlock(parentJob()->parseSession()->macros);

    QString result = preprocessor.processFile(contents, rpp::pp::Data);

    parentJob()->parseSession()->setContents( result.toUtf8() );
    parentJob()->parseSession()->cachedLexedFile = m_cachedLexedFile;

    if( PreprocessJob* parentPreprocessor = parentJob()->parentPreprocessor() ) {
        //If we are included from another preprocessor, give it back the modified macros,
        parentPreprocessor->m_currentEnvironment->takeMacros(m_currentEnvironment);
        //Merge include-file-set, defined macros, used macros, and string-set
        parentPreprocessor->m_cachedLexedFile->merge(*m_cachedLexedFile);
    }
    
    m_currentEnvironment = 0;
}

rpp::Stream* PreprocessJob::sourceNeeded(QString& fileName, IncludeType type, int sourceLine)
{
    Q_UNUSED(type)

    // FIXME change to interruptable preprocessor

    if (checkAbort())
        return 0;

    KUrl includedFile;

/*    if (CPPParseJob* parent = parentJob()) {
        VALGRIND_CHECK_MEM_IS_DEFINED(parent, sizeof(CPPParseJob));

        if (CppLanguageSupport* lang = parent->cpp()) {
            VALGRIND_CHECK_MEM_IS_DEFINED(lang, sizeof(CppLanguageSupport)); */

    includedFile = parentJob()->cpp()->findInclude(parentJob()->document(), fileName);
    if (includedFile.isValid()) {
        /**
         * @todo Check if there already is a cached usable instance of the file, if it is use it
         * Else preprocess the file now
         * */

        ///Why bother the threadweaver? We need the preprocessed text NOW so we simply parse the included file right here. Parallel parsing cannot be used here, because we need the macros before we can continue.

        //Create a slave-job that will take over our macros
        CPPParseJob* slaveJob = new CPPParseJob( includedFile, parentJob()->cpp(), this );
        slaveJob->parseForeground();

        //Add the included file
        Q_ASSERT(slaveJob->duChain());
        parentJob()->addIncludedFile( slaveJob->duChain() );
        
        delete slaveJob;
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
