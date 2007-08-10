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

#include "cppparsejob.h"

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include "Thread.h"

#include <ilanguage.h>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include "parser/parsesession.h"
// #include "parser/binder.h"
#include "parser/parser.h"
#include "parser/control.h"
#include <duchain.h>
#include <duchainlock.h>
#include "dumpchain.h"
#include <parsejob.h>
#include "cppeditorintegrator.h"
#include "declarationbuilder.h"
#include "usebuilder.h"
#include <topducontext.h>
#include "preprocessjob.h"
#include "environmentmanager.h"

using namespace KDevelop;

CPPParseJob::CPPParseJob( const KUrl &url,
                    CppLanguageSupport *parent, PreprocessJob* parentPreprocessor )
        : KDevelop::ParseJob( url, parent ),
        m_parentPreprocessor( parentPreprocessor ),
        m_session( new ParseSession ),
        m_AST( 0 ),
        m_duContext( 0 ),
        m_readFromDisk( false ),
        m_includePathsComputed( false )
{
    if( !m_parentPreprocessor ) {
        addJob(m_preprocessJob = new PreprocessJob(this));
        addJob(m_parseJob = new CPPInternalParseJob(this));
        // Higher priority means it will be preferred over other waiting preprocess jobs
        m_parseJob->setPriority(1);
    } else {
        m_preprocessJob = 0;
        m_parseJob = 0;
        //The preprocessor will call parseForeground() to preprocess & parse instantly
    }

    kDebug( 9007 ) << k_funcinfo << "Created job" << this << "pp" << m_preprocessJob << "parse" << parseJob();
}

void CPPParseJob::parseForeground() {
    //Create the sub-jobs and directly execute them.
    Q_ASSERT( !m_preprocessJob && !m_parseJob );
    m_preprocessJob = new PreprocessJob(this);
    m_parseJob = new CPPInternalParseJob(this);
    m_preprocessJob->run();
    m_parseJob->run();
}

KUrl CPPParseJob::includedFromPath() const {
    return m_includedFromPath;
}

void CPPParseJob::setIncludedFromPath( const KUrl& path ) {
    m_includedFromPath = path;
}

PreprocessJob* CPPParseJob::parentPreprocessor() const {
    return m_parentPreprocessor;
}

const KUrl::List& CPPParseJob::includePaths() const {
    if( masterJob() == this ) {
        if( !m_includePathsComputed ) {
            m_includePathsComputed = true;
            m_includePaths = cpp()->findIncludePaths(document());
        }
        return m_includePaths;
    } else {
        return masterJob()->includePaths();
    }
}

/*
CPPParseJob::CPPParseJob( KDevelop::Document *document,
                    CppLanguageSupport *parent,
                    const KTextEditor::Range& textRangeToParse )
        : KDevelop::ParseJob( document, parent ),
        m_session( new ParseSession ),
        m_AST( 0 ),
        m_model( 0 ),
        m_duContext( 0 ),
        m_readFromDisk( false ),
        m_textRangeToParse(textRangeToParse)
{
    PreprocessJob* ppj;
    addJob(ppj = new PreprocessJob(this));
    addJob(m_parseJob = new ::ParseJob(this));

    //kDebug(9007) << k_funcinfo << "Created job" << this << "pp" << ppj << "parse" << parseJob();
}
*/

CPPParseJob::~CPPParseJob()
{}

TranslationUnitAST *CPPParseJob::AST() const
{
    Q_ASSERT ( isFinished () && m_AST );
    return m_AST;
}

TopDUContext* CPPParseJob::duChain() const
{
    return m_duContext;
}

CPPParseJob* CPPParseJob::masterJob() {
    if( parentPreprocessor() )
        return static_cast<CPPParseJob*>(parentPreprocessor()->parent())->masterJob();
    else
        return this;
}

const CPPParseJob* CPPParseJob::masterJob() const {
    if( parentPreprocessor() )
        return static_cast<CPPParseJob*>(parentPreprocessor()->parent())->masterJob();
    else
        return this;
}

void CPPParseJob::setUpdatingContext( const TopDUContextPointer& context ) {
    m_updatingContext = context;
}

TopDUContextPointer CPPParseJob::updatingContext() const {
    return m_updatingContext;
}

CppLanguageSupport * CPPParseJob::cpp() const
{
    Q_ASSERT( parent() || parentPreprocessor() );
    if( parent() )
        return static_cast<CppLanguageSupport*>(const_cast<QObject*>(parent()));
    else
        return static_cast<CPPParseJob*>(parentPreprocessor()->parent())->cpp();
}

void CPPParseJob::addIncludedFile(KDevelop::TopDUContext* duChain) {
    m_includedFiles.push_back(duChain);
}

void CPPParseJob::setEnvironmentFile( Cpp::EnvironmentFile* file ) {
    m_environmentFile = KSharedPtr<Cpp::EnvironmentFile>(file);
}

Cpp::EnvironmentFile* CPPParseJob::environmentFile() {
    return m_environmentFile.data();
}

const IncludeFileList& CPPParseJob::includedFiles() const {
    return m_includedFiles;
}

CPPParseJob * CPPInternalParseJob::parentJob() const
{
    Q_ASSERT(parent());
    return static_cast<CPPParseJob*>(const_cast<QObject*>(parent()));
}

void CPPParseJob::setAST(TranslationUnitAST * ast)
{
    m_AST = ast;
}

void CPPParseJob::setDUChain(TopDUContext * duChain)
{
    m_duContext = duChain;
}

const KTextEditor::Range& CPPParseJob::textRangeToParse() const
{
    return m_textRangeToParse;
}

CPPInternalParseJob::CPPInternalParseJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_priority(0)
{
}

void CPPInternalParseJob::run()
{
    kDebug( 9007 ) << "===-- PARSING --===> "
    << parentJob()->document().fileName()
    << endl;
    Q_ASSERT(parentJob()->environmentFile());

    if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(thread()));

    Parser parser( new Control() );

    TranslationUnitAST* ast = parser.parse( parentJob()->parseSession() );

    if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    if ( ast )
    {
//         ast->language = parentJob()->cpp();
        ast->session = parentJob()->parseSession();
    }

    parentJob()->setAST(ast);

    if ( ast )
    {
//         CodeModel* model = new CodeModel;
//         Binder binder( model, parentJob()->parseSession() );
//         binder.run( parentJob()->document(), ast );

        if (parentJob()->abortRequested())
            return parentJob()->abortJob();

//         parentJob()->setCodeModel(model);

        QList<DUContext*> chains;
        QList<DUContext*> temporaryChains;

        if( parentJob()->parentPreprocessor() ) {
            //Temporarily insert the parent's chains here, so forward-declarations can be resolved and types can be used, as it should be.
            //In theory, we would need to insert the whole until-now parsed part of the parent, but that's not possible because preprocessing and parsing are
            //separate steps. So we simply assume that the includes are at the top of the file.
            foreach ( TopDUContext* context, parentJob()->parentPreprocessor()->parentJob()->includedFiles() ) {
                if( !parentJob()->includedFiles().contains(context) ) {
                    chains << context;
                    temporaryChains << context;
                }
            }
        }

        foreach ( TopDUContext* context, parentJob()->includedFiles() )
            chains << context;

        TopDUContext* topContext;

        // Control the lifetime of the editor integrator (so that locking works)
        {
            kDebug( 9007 ) << "building duchain";
            CppEditorIntegrator editor(parentJob()->parseSession());
            editor.setCurrentUrl(parentJob()->document());

            // Translate the cursors we generate with edits that have happened since retrieval of the document source.
            if (editor.smart() && parentJob()->revisionToken() != -1)
              editor.smart()->useRevision(parentJob()->revisionToken());

            DeclarationBuilder declarationBuilder(&editor);
            topContext = declarationBuilder.buildDeclarations(Cpp::EnvironmentFilePointer(parentJob()->environmentFile()), ast, &chains, parentJob()->updatingContext());

            if (parentJob()->abortRequested())
                return parentJob()->abortJob();

            // We save some time here by not running the use compiler if the file is not loaded... (it's only needed
            // for navigation in that case)
            // FIXME make configurable
            if (!parentJob()->wasReadFromDisk()) {
                UseBuilder useBuilder(&editor);
                useBuilder.buildUses(ast);

                if (parentJob()->abortRequested())
                    return parentJob()->abortJob();
            }

            if( parentJob()->parentPreprocessor() ) {
                DUChainWriteLocker l(DUChain::lock());
                //Remove include-files we imported temporarily, because they are on the same level in reality
                foreach ( DUContext* context, temporaryChains )
                    topContext->removeImportedParentContext(context);
            }

            parentJob()->setDUChain(topContext);

            kDebug( 9007 ) << "duchain is built";
            if ( parentJob()->cpp()->codeHighlighting() && editor.smart() )
            {
                QMutexLocker lock(editor.smart()->smartMutex());
                parentJob()->cpp()->codeHighlighting()->highlightDUChain( topContext );
            }

            if (editor.smart())
              editor.smart()->clearRevision();
        }

        // Debug output...

        /*if (topContext->smartRange()) {
            DUChainReadLocker lock(DUChain::lock());
            kDebug( 9007 ) << "================== duchain ==================";
            if (topContext->smartRange()) {
                DumpChain dump;
                dump.dump(ast, parentJob()->parseSession());
                dump.dump(topContext);
            }
        }*/
    }

    //DumpTree dumpTree;
    //dumpTree.dump( m_AST );

    kDebug( 9007 ) << "===-- Parsing finished --===>" << parentJob()->document().fileName();
}


void CPPParseJob::requestDependancies()
{
}

ParseSession * CPPParseJob::parseSession() const
{
    return m_session;
}

void CPPParseJob::setReadFromDisk(bool readFromDisk)
{
    m_readFromDisk = readFromDisk;
}

bool CPPParseJob::wasReadFromDisk() const
{
    return m_readFromDisk;
}

CPPInternalParseJob * CPPParseJob::parseJob() const
{
    return m_parseJob;
}

int CPPInternalParseJob::priority() const
{
    return m_priority;
}

void CPPInternalParseJob::setPriority(int priority)
{
    m_priority = priority;
}

#include "cppparsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
