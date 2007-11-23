/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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
#include <duchainpointer.h>
#include <duchainlock.h>
#include <dumpdotgraph.h>
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
        m_readFromDisk( false ),
        m_includePathsComputed( false ),
        m_useContentContext( false )
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

    kDebug( 9007 ) << "Created job" << this << "pp" << m_preprocessJob << "parse" << parseJob();
}

void CPPParseJob::setUseContentContext(bool b) {
    m_useContentContext = b;
}

bool CPPParseJob::useContentContext() const {
    return m_useContentContext;
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
        m_readFromDisk( false ),
        m_textRangeToParse(textRangeToParse)
{
    PreprocessJob* ppj;
    addJob(ppj = new PreprocessJob(this));
    addJob(m_parseJob = new ::ParseJob(this));

    //kDebug(9007) << "Created job" << this << "pp" << ppj << "parse" << parseJob();
}
*/

CPPParseJob::~CPPParseJob()
{}

TranslationUnitAST *CPPParseJob::AST() const
{
    Q_ASSERT ( isFinished () && m_AST );
    return m_AST;
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

void CPPParseJob::setContentContext( const TopDUContextPointer& context ) {
    m_contentContext = context;
}

TopDUContextPointer CPPParseJob::contentContext() const {
    return m_contentContext;
}

CppLanguageSupport * CPPParseJob::cpp() const
{
    Q_ASSERT( parent() || parentPreprocessor() );
    if( parent() )
        return static_cast<CppLanguageSupport*>(const_cast<QObject*>(parent()));
    else
        return static_cast<CPPParseJob*>(parentPreprocessor()->parent())->cpp();
}

void CPPParseJob::addIncludedFile(KDevelop::TopDUContext* duChain, int sourceLine) {
    m_includedFiles.push_back(LineContextPair(duChain, sourceLine));
}

void CPPParseJob::setEnvironmentFile( Cpp::EnvironmentFile* file ) {
    m_environmentFile = KSharedPtr<Cpp::EnvironmentFile>(file);
}

Cpp::EnvironmentFile* CPPParseJob::environmentFile() {
    return m_environmentFile.data();
}

void CPPParseJob::setContentEnvironmentFile( Cpp::EnvironmentFile* file ) {
    //Q_ASSERT(!file || file->identity().flags() & IdentifiedFile::Content);
    m_contentEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>(file);
}

Cpp::EnvironmentFile* CPPParseJob::contentEnvironmentFile() {
    return m_contentEnvironmentFile.data();
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

const KTextEditor::Range& CPPParseJob::textRangeToParse() const
{
    return m_textRangeToParse;
}

CPPInternalParseJob::CPPInternalParseJob(CPPParseJob * parent)
    : ThreadWeaver::Job(parent)
    , m_priority(0)
{
}

///If @param ctx is a proxy-context, returns the target-context. Else returns ctx. @warning du-chain must be locked
LineContextPair contentFromProxy(LineContextPair ctx) {
    if( ctx.context->flags() & TopDUContext::ProxyContextFlag ) {
        Q_ASSERT(ctx.context->importedParentContexts().count() == 1);
        return LineContextPair( dynamic_cast<TopDUContext*>(ctx.context->importedParentContexts().first().data()), ctx.sourceLine );
    }else{
        return ctx;
    }
}

void CPPInternalParseJob::run()
{
    kDebug( 9007 ) << "===-- PARSING --===> "
    << parentJob()->document().fileName()
    << endl;
    Q_ASSERT(parentJob()->environmentFile());

    if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(QThread::currentThread()));

    TranslationUnitAST* ast = 0L;

    {
      Control control;
      Parser parser(&control);

      ast = parser.parse( parentJob()->parseSession() );

      if (parentJob()->abortRequested())
          return parentJob()->abortJob();

      if ( ast )
      {
  //         ast->language = parentJob()->cpp();
          ast->session = parentJob()->parseSession();
      }

      foreach (const KDevelop::Problem& p, control.problems())
        KDevelop::DUChain::problemEncountered(p);

      parentJob()->setAST(ast);
    }

    if ( ast )
    {
        if (parentJob()->abortRequested())
            return parentJob()->abortJob();

        //If we are building a separate content-context
        TopDUContextPointer updating = parentJob()->updatingContext();
        Cpp::EnvironmentFilePointer environmentFile(parentJob()->environmentFile());

        if( parentJob()->contentEnvironmentFile() )
        {
            if( parentJob()->contentContext() )
                updating = parentJob()->contentContext().data();
            else
                updating = 0; //We should build a separate context, but none is set.

            //Use the EnvironmentFile of the content-context.
            environmentFile = parentJob()->contentEnvironmentFile();
            //Q_ASSERT(environmentFile->identity().flags() & IdentifiedFile::Content);
        }

        //If simplified environment-matching is used, we now only care about the content-context, until we create the proxy-context in the end.
        
        QList<LineContextPair> chains;
        QList<DUContext*> temporaryChains;

        ///Here we count together all files that were included while this parse-run. If we are updating, and have parsed the document completely, we can remove all contexts urls that do not match this.
        QSet<KUrl> encounteredIncludeUrls;

        {
            DUChainReadLocker lock(DUChain::lock());
            ///We directly insert the content-contexts, to create a cleaner structure. Else the tree would be cluttered and redundant.
            foreach ( LineContextPair context, parentJob()->includedFiles() ) {
                chains << contentFromProxy(context);
                encounteredIncludeUrls << context.context->url();
            }
        }

        
        if( parentJob()->parentPreprocessor() ) {
            DUChainReadLocker lock(DUChain::lock());
            //Temporarily insert the parent's chains here, so forward-declarations can be resolved and types can be used, as it should be.
            //In theory, we would need to insert the whole until-now parsed part of the parent, but that's not possible because preprocessing and parsing are
            //separate steps. So we simply assume that the includes are at the top of the file.
            foreach ( LineContextPair topContext, parentJob()->parentPreprocessor()->parentJob()->includedFiles() ) {
                //As above, we work with the content-contexts.
                LineContextPair context = contentFromProxy(topContext);
                DUContextPointer ptr(context.context);
                if( !containsContext(chains, context.context)  && (!parentJob()->contentContext().data() || !updating || !updating->importedParentContexts().contains(ptr)) ) {
                    chains << context;
                    temporaryChains << context.context;
                }
            }
        }

        TopDUContext* topContext;

        // Control the lifetime of the editor integrator (so that locking works)
        {
            
            CppEditorIntegrator editor(parentJob()->parseSession());
            editor.setCurrentUrl(parentJob()->document());

            // Translate the cursors we generate with edits that have happened since retrieval of the document source.
            if (editor.smart() && parentJob()->revisionToken() > 0)
              editor.smart()->useRevision(parentJob()->revisionToken());
            
            if( !parentJob()->useContentContext() )
            {
                ///We need to update or build a context
                kDebug( 9007 ) << "building duchain";

                DeclarationBuilder declarationBuilder(&editor);
                topContext = declarationBuilder.buildDeclarations(environmentFile, ast, &chains, updating, !(bool)parentJob()->contentContext());

                if(updating) {
                    DUChainWriteLocker l(DUChain::lock());
                    //We have updated a context. Now remove all headers that were not encountered while this parse(maybe one was removed)
                    ///@todo What if a header was found in another path? Then it will be removed here.
                    QList<DUContextPointer> imports = topContext->importedParentContexts();
                    foreach(DUContextPointer ctx, imports) {
                        if(ctx.data() && !encounteredIncludeUrls.contains(ctx->url())) {
                            topContext->removeImportedParentContext(ctx.data());
                            kDebug(9007) << "removing not encountered import " << ctx->url();
                        }
                    }
                }
                
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
            }
            else
            {
                DUChainWriteLocker l(DUChain::lock());
                ///We can re-use a readily available content-context
                Q_ASSERT(parentJob()->contentContext());
                kDebug( 9007 ) << "reusing existing content duchain";
                topContext = parentJob()->contentContext().data();

                ///Add all our imports to the re-used context
                foreach( const LineContextPair& import, chains )
                    topContext->addImportedParentContext(import.context, KTextEditor::Cursor(import.sourceLine, 0));
                if( !topContext ) {
                    kDebug( 9007 ) << "Context was deleted while parsing";
                    return;
                }
            }

            if ( parentJob()->cpp()->codeHighlighting() && editor.smart() )
            {
                QMutexLocker lock(editor.smart()->smartMutex());
                parentJob()->cpp()->codeHighlighting()->highlightDUChain( topContext );
            }
            
            if( parentJob()->parentPreprocessor() ) {
                DUChainWriteLocker l(DUChain::lock());
                //Remove include-files we imported temporarily, because they are on the same level in reality
                foreach ( DUContext* context, temporaryChains ) {
                    topContext->removeImportedParentContext(context);
                    removeContext(chains, dynamic_cast<TopDUContext*>(context));
                }
            }

            Q_ASSERT(topContext);

            //If we are using simplified environment-matching, create a proxy-context that represents the parsed content-context environment-wise
            if( parentJob()->contentEnvironmentFile() ) {
                ContextBuilder builder(&editor);
                TopDUContextPointer ptr(topContext);
                topContext = builder.buildProxyContextFromContent(Cpp::EnvironmentFilePointer(parentJob()->environmentFile()), ptr, parentJob()->updatingContext());
            }

            parentJob()->setDuChain(topContext);

            kDebug( 9007 ) << "duchain is built";

            if (editor.smart())
              editor.smart()->clearRevision();
        }

        // Debug output...

        if ( !parentJob()->parentPreprocessor() ) {
/*            DUChainReadLocker lock(DUChain::lock());
            kDebug( 9007 ) << "================== duchain ==================";
            DumpChain dump;
            //dump.dump(ast, parentJob()->parseSession());
            dump.dump(topContext);
            KDevelop::DumpDotGraph dumpGraph;
            kDebug(9007) << "Dot-graph:\n" << dumpGraph.dotGraph(topContext, true);*/
        }
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

const QStack< DocumentCursor > & CPPParseJob::includeStack() const
{
  return m_includeStack;
}

void CPPParseJob::setIncludeStack(const QStack< DocumentCursor > & includeStack)
{
  m_includeStack = includeStack;
}

#include "cppparsejob.moc"

