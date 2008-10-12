/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
* Copyright 2007-2008 David Nolden<david.nolden@kdevelop.org>
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

#include <threadweaver/Thread.h>

#include <interfaces/ilanguage.h>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include "parser/parsesession.h"
// #include "parser/binder.h"
#include "parser/parser.h"
#include "parser/control.h"
#include "parser/dumptree.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/dumpdotgraph.h>
#include <language/duchain/dumpchain.h>
#include <language/backgroundparser/parsejob.h>
#include "cppeditorintegrator.h"
#include "declarationbuilder.h"
#include "usebuilder.h"
#include <language/duchain/topducontext.h>
#include "preprocessjob.h"
#include "environmentmanager.h"
#include <unistd.h>
#include <qwaitcondition.h>

//#define DUMP_SMART_RANGES
//#define DUMP_AST
//#define DUMP_DUCHAIN

using namespace KDevelop;

bool importsContext(const QVector<DUContext::Import>& contexts, const DUContext* context) {
  foreach(DUContext::Import listCtx, contexts)
    if(listCtx.context() && listCtx.context()->imports(context))
      return true;
  return false;
}

QList<IndexedString> convertFromUrls(const QList<KUrl>& urlList) {
  QList<IndexedString> ret;
  foreach(const KUrl& url, urlList)
    ret << IndexedString(url.pathOrUrl());
  return ret;
}

bool CPPParseJob::needUpdateEverything() const {
  return m_needUpdateEverything;
}

bool CPPParseJob::wasUpdated(const KDevelop::DUContext* context) const
{
  return m_updated.contains(context);
}
void CPPParseJob::setWasUpdated(const KDevelop::DUContext* context)
{
  m_updated.insert(context);
}

const QSet<const KDevelop::DUContext*>& CPPParseJob::updated() const
{
  return m_updated;
}


void CPPParseJob::setNeedUpdateEverything(bool need) {
  m_needUpdateEverything = need;
}

CPPParseJob::CPPParseJob( const KUrl &url,
                    CppLanguageSupport *parent, PreprocessJob* parentPreprocessor )
        : KDevelop::ParseJob( url, parent ),
        m_needUpdateEverything( false ),
        m_parentPreprocessor( parentPreprocessor ),
        m_session( new ParseSession ),
        m_readFromDisk( false ),
        m_includePathsComputed( false ),
        m_keepDuchain( false ),
        m_parsedIncludes( 0 ),
        m_needsUpdate( true )
{
    if( !m_parentPreprocessor ) {
        addJob(m_preprocessJob = new PreprocessJob(this));
        addJob(m_parseJob = new CPPInternalParseJob(this));
    } else {
        m_preprocessJob = 0;
        m_parseJob = 0;
        //The preprocessor will call parseForeground() to preprocess & parse instantly
    }

    kDebug( 9007 ) << "Created job" << this << "pp" << m_preprocessJob << "parse" << parseJob();
}

void CPPParseJob::setKeepDuchain(bool b) {
    m_keepDuchain = b;
}

bool CPPParseJob::keepDuchain() const {
    return m_keepDuchain;
}

void CPPParseJob::setKeepEverything(bool b) {
    m_keepEverything = b;
}

bool CPPParseJob::keepEverything() const {
    return m_keepEverything;
}

void CPPParseJob::includedFileParsed() {
  ++m_parsedIncludes;
  const int estimateIncludes = 450;
  float _progress = ((float)m_parsedIncludes) / estimateIncludes;
  if(_progress > 0.8)
    _progress = 0.8;

  emit progress(this, _progress, i18n("Parsing included files"));
}

void CPPParseJob::setLocalProgress(float _progress, QString text) {
  emit progress(this, 0.8+_progress*0.2, text);
}

void CPPParseJob::setNeedsUpdate(bool needs) {
  m_needsUpdate = needs;
}

bool CPPParseJob::needsUpdate() const {
  return m_needsUpdate;
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

void CPPParseJob::gotIncludePaths(const KUrl::List& paths) {
  m_includePathsComputed = true;
  m_includePathUrls = paths;
  m_includePaths = convertFromUrls(m_includePathUrls);
  m_waitForIncludePathsMutex.lock(); //This makes sure that the parse thread goes into the waiting state first
  m_waitForIncludePathsMutex.unlock();
  m_waitForIncludePaths.wakeAll();
}


const KUrl::List& CPPParseJob::includePathUrls() const {
  includePaths();
  return masterJob()->m_includePathUrls;
}

const QList<IndexedString>& CPPParseJob::includePaths() const {
    if( masterJob() == this ) {
        if( !m_includePathsComputed ) {
            m_waitForIncludePathsMutex.lock();
            qRegisterMetaType<CPPParseJob*>("CPPParseJob*");
            ///@todo Make sure this doesn't create problems in corner cases
            QMetaObject::invokeMethod(cpp(), "findIncludePathsForJob", Qt::QueuedConnection, Q_ARG(CPPParseJob*, const_cast<CPPParseJob*>(this)));
            //Will be woken once the include-paths are computed
            m_waitForIncludePaths.wait(&m_waitForIncludePathsMutex);
            m_waitForIncludePathsMutex.unlock();
        }
        return m_includePaths;
    } else {
        return masterJob()->includePaths();
    }
}

CPPParseJob::~CPPParseJob()
{
  delete m_session;
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

void CPPParseJob::setUpdatingProxyContext( const ReferencedTopDUContext& context ) {
    m_updatingProxyContext = context;
}

ReferencedTopDUContext CPPParseJob::updatingProxyContext() const {
    return m_updatingProxyContext;
}

void CPPParseJob::setUpdatingContentContext( const ReferencedTopDUContext& context ) {
    m_updatingContentContext = context;
}

ReferencedTopDUContext CPPParseJob::updatingContentContext() const {
    return m_updatingContentContext;
}

CppLanguageSupport * CPPParseJob::cpp() const
{
    Q_ASSERT( parent() || parentPreprocessor() );
    if( parent() )
        return static_cast<CppLanguageSupport*>(const_cast<QObject*>(parent()));
    else
        return static_cast<CPPParseJob*>(parentPreprocessor()->parent())->cpp();
}

void CPPParseJob::addIncludedFile(KDevelop::ReferencedTopDUContext duChain, int sourceLine) {
    m_includedFiles.push_back(LineContextPair(duChain, sourceLine));
}

void CPPParseJob::setProxyEnvironmentFile( Cpp::EnvironmentFile* file ) {
    m_proxyEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>(file);
}

Cpp::EnvironmentFile* CPPParseJob::proxyEnvironmentFile() {
    return m_proxyEnvironmentFile.data();
}

void CPPParseJob::setContentEnvironmentFile( Cpp::EnvironmentFile* file ) {
    //Q_ASSERT(!file || file->identity().flags() & IdentifiedFile::Content);
    m_contentEnvironmentFile = KSharedPtr<Cpp::EnvironmentFile>(file);
}

void CPPParseJob::addPreprocessorProblem(const ProblemPointer problem) {
  m_preprocessorProblems << problem;
}

QList<ProblemPointer> CPPParseJob::preprocessorProblems() const {
  return m_preprocessorProblems;
}

QList<ProblemPointer>* CPPParseJob::preprocessorProblemsPointer() {
  return &m_preprocessorProblems;
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
    if( ctx.context->parsingEnvironmentFile() && ctx.context->parsingEnvironmentFile()->isProxyContext() ) {
        Q_ASSERT(!ctx.context->importedParentContexts().isEmpty());
        return LineContextPair( dynamic_cast<TopDUContext*>(ctx.context->importedParentContexts().first().context()), ctx.sourceLine );
    }else{
        return ctx;
    }
}

void CPPInternalParseJob::run()
{
    if(!parentJob()->needsUpdate()) {
      kDebug( 9007 ) << "===-- ALREADY UP TO DATE --===> " << parentJob()->document().str();
      return;
    }
    if(!parentJob()->contentEnvironmentFile()) {
      kDebug( 9007 ) << "===-- Problem: Preprocessor did not create environment-file, skipping --===> " << parentJob()->document().str();
      return;
    }
    kDebug( 9007 ) << "===-- PARSING --===> "
    << parentJob()->document().str();


    if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    parentJob()->setLocalProgress(0, i18n("Parsing actual file"));

    QMutexLocker lock(parentJob()->cpp()->language()->parseMutex(QThread::currentThread()));

    ReferencedTopDUContext updatingProxyContext = parentJob()->updatingProxyContext().data();
    ReferencedTopDUContext updatingContentContext = parentJob()->updatingContentContext().data();

    ReferencedTopDUContext proxyContext = updatingProxyContext; //Proxy-context if simplified environment-matching is enabled, else zero.
    ReferencedTopDUContext contentContext= updatingContentContext;  //The actual context that contains the data.

    Cpp::EnvironmentFilePointer proxyEnvironmentFile(parentJob()->proxyEnvironmentFile());
    Cpp::EnvironmentFilePointer contentEnvironmentFile(parentJob()->contentEnvironmentFile());

    //Eventually remove old imports
    if(contentContext) {
      /**
      * If we are updating everything, and the context has not been updated yet in this run,
       * remove all contexts previously imported into the content-context,
      * because they may contain badly parsed data, that might later override new parsed data.
      * If simplified environment-matching is disabled, always remove the imports if the file is reparsed,
      * their new versions will be re-added.
      * */
      if(!parentJob()->keepDuchain() &&
        ((!parentJob()->masterJob()->wasUpdated(contentContext) && parentJob()->needUpdateEverything())
          || !proxyContext)) {
          DUChainWriteLocker lock(DUChain::lock());

          foreach(const DUContext::Import& ctx, contentContext->importedParentContexts())
            contentContext->removeImportedParentContext(ctx.context());
          }
    }

    QList<LineContextPair> importedContentChains; //All content-chains imported while this parse-run. Also contains the temporary ones.
    QList<ReferencedTopDUContext> importedTemporaryChains; //All imported content-chains that were imported temporarily from parents.
    QSet<KDevelop::IndexedString> encounteredIncludeUrls; //All imported file-urls that were encountered this run.

    {
        DUChainReadLocker lock(DUChain::lock());
        foreach ( LineContextPair context, parentJob()->includedFiles() ) {
            importedContentChains << contentFromProxy(context);
            encounteredIncludeUrls << context.context->url();
        }
    }

    //Temporarily import contexts imported by parents, because in C++ those are visible from here too
    if( parentJob()->parentPreprocessor() ) {
        DUChainReadLocker lock(DUChain::lock());
        //In theory, we would need to insert the whole until-now parsed part of the parent, but that's not possible because
        //we mix preprocessing and parsing, by triggering the parsing of included files by the preprocessor of the including file
        //So we simply assume that the includes are at the top of the file.
        CPPParseJob* currentJob = parentJob()->parentPreprocessor()->parentJob();

        while(currentJob) {
          foreach ( LineContextPair topContext, currentJob->includedFiles() ) {
              //As above, we work with the content-contexts.
              LineContextPair context = contentFromProxy(topContext);
              DUContextPointer ptr(context.context);
//               if( !importsContext(importedContentChains, context.context)  && (!updatingContentContext ||       !importsContext(updatingContentContext->importedParentContexts(), context.context)) ) {
//                   if(!updatingContentContext || !updatingContentContext->imports(context.context, updatingContentContext->range().end)) {
                    importedContentChains << context;
                    importedContentChains.back().temporary = true;
                    importedTemporaryChains << context.context;
//                   }
//               }
          }
          if(currentJob->parentPreprocessor())
            currentJob = currentJob->parentPreprocessor()->parentJob();
          else
            currentJob = 0;
        }
    }

    ///Now we build/update the content-context

    if(!parentJob()->keepDuchain()) {

      TranslationUnitAST* ast = 0L;

      Control control;
      Parser parser(&control);

      ast = parser.parse( parentJob()->parseSession() );

      if (parentJob()->abortRequested())
          return parentJob()->abortJob();

      if ( ast ) {
          ast->session = parentJob()->parseSession();
#ifdef DUMP_AST
          DumpTree dump;
          dump.dump(ast, parentJob()->parseSession()->token_stream);
#endif
      }

      CppEditorIntegrator editor(parentJob()->parseSession());
      editor.setCurrentUrl(parentJob()->document());

      kDebug( 9007 ) << (contentContext ? "updating" : "building") << "duchain for" << parentJob()->document().str();

      if(contentContext) {

        ///We're updating, wait until no other thread is updating this context, and then mark that we are updating it.
        bool wait = true;

        while(wait) {
          DUChainWriteLocker l(DUChain::lock());
          if( contentContext->flags() & TopDUContext::UpdatingContext )
            wait = true;
          else
            wait = false;

          if(!wait)
            contentContext->setFlags( (TopDUContext::Flags)(contentContext->flags() | TopDUContext::UpdatingContext) );

          if(wait) {
            kDebug(9007) << "waiting while" << parentJob()->document().str() << "is being updated by another thread";
            l.unlock();
            sleep(1);
          }
        }

        DUChainWriteLocker l(DUChain::lock());
        contentContext->clearProblems();
      }
      
      uint oldItemCount = 0;
      if(contentContext) {
        DUChainReadLocker l(DUChain::lock());
        oldItemCount = contentContext->childContexts().size() + contentContext->localDeclarations().size();
      }

      DeclarationBuilder declarationBuilder(&editor);

      if(parentJob()->minimumFeatures() == TopDUContext::VisibleDeclarationsAndContexts && (!contentContext || contentContext->features() == TopDUContext::VisibleDeclarationsAndContexts))
        declarationBuilder.setOnlyComputeVisible(true); //Only visible declarations/contexts need to be built.
        
      
      contentContext = declarationBuilder.buildDeclarations(contentEnvironmentFile, ast, &importedContentChains, contentContext, false);

      contentEnvironmentFile->setTopContext(contentContext.data());
      
      //If publically visible declarations were added/removed, all following parsed files need to be updated
      if(declarationBuilder.changeWasSignificant()) {
        ///@todo The right solution to the whole problem: Do not put any imports into the content-contexts. Instead, Represent the complete import-structure in the proxy-contexts.
        ///      While searching, always use the perspective of the proxy. Even better: Change the context-system so proxy-contexts become completely valid contexts from the outside perspective,
        ///      that import all their imports, and that share all their content except the imports/environment-information with all the other proxy contexts for that file, and with one content-context.
        ///      Problem: What to do with imports that happen within the content-section then?
        //kDebug() << "A significant change was recorded, all following contexts will be updated";
        //parentJob()->masterJob()->setNeedUpdateEverything(true);
      }

      {
        DUChainReadLocker l(DUChain::lock());
        Q_ASSERT(!updatingContentContext || updatingContentContext == contentContext);
        Q_ASSERT(contentContext->parsingEnvironmentFile().data() == contentEnvironmentFile.data());
        
        if(contentContext->childContexts().size() + contentContext->localDeclarations().size() == 0) {
          if(oldItemCount != 0) {
            //To catch some problems
            kDebug(9007) << "All items in" << parentJob()->document().str() << "have been extincted, previous count:" << oldItemCount << "current identity offset:" << contentEnvironmentFile->identityOffset();
          }
        }
      }

      {
        DUChainWriteLocker l(DUChain::lock());

        foreach( const ProblemPointer& problem, parentJob()->preprocessorProblems() ) {
          if(problem->finalLocation().start().line() >= contentEnvironmentFile->contentStartLine()) {
            contentContext->addProblem(problem);
          }
        }

        foreach( KDevelop::ProblemPointer p, control.problems() ) {
          p->setLocationStack(parentJob()->includeStack());
          contentContext->addProblem(p);
        }
      }

      if(contentContext) {
          DUChainWriteLocker l(DUChain::lock());
          QList<TopDUContext*> remove;
          foreach(ReferencedTopDUContext ctx, importedTemporaryChains)
              remove << ctx.data();
          contentContext->removeImportedParentContexts(remove);
      }

      ///When simplified environment-matching is enabled, we will accumulate many different
      ///versions of imports into a single top-context. To reduce that a little, we remove all
      ///with urls we didn't encounter.
      if(updatingContentContext) {
          DUChainWriteLocker l(DUChain::lock());

          //Remove the temporary chains first, so we don't get warnings from them

          QVector<DUContext::Import> imports = contentContext->importedParentContexts();
          foreach(DUContext::Import ctx, imports) {
              if(ctx.context() && !encounteredIncludeUrls.contains(ctx.context()->url()) && contentEnvironmentFile->missingIncludeFiles().set().count() == 0 && (!proxyEnvironmentFile || proxyEnvironmentFile->missingIncludeFiles().set().count() == 0)) {
                  contentContext->removeImportedParentContext(ctx.context());
                  kDebug( 9007 ) << "removing not encountered import " << ctx.context()->url().str();
              }
          }
      }

      if (!parentJob()->abortRequested()) {
        if (contentContext->features() == TopDUContext::AllDeclarationsContextsAndUses || parentJob()->minimumFeatures() == TopDUContext::AllDeclarationsContextsAndUses) {
            parentJob()->setLocalProgress(0.5, i18n("Building uses"));

            UseBuilder useBuilder(&editor);
            useBuilder.buildUses(ast);
        }

        if (!parentJob()->abortRequested() && editor.smart()) {
          editor.smart()->clearRevision();

          if ( parentJob()->cpp()->codeHighlighting() )
          {
              parentJob()->cpp()->codeHighlighting()->highlightDUChain( contentContext );
          }
        }
      }
      
      ///Now mark the context as not being updated. This MUST be done or we will be waiting forever in a loop
      {
          DUChainWriteLocker l(DUChain::lock());
          contentContext->setFeatures((TopDUContext::Features) (contentContext->features() | parentJob()->minimumFeatures()) );
          if(proxyContext)
            proxyContext->setFeatures(contentContext->features());
          contentContext->setFlags( (TopDUContext::Flags)(contentContext->flags() & (~TopDUContext::UpdatingContext)) );
      }

      if (parentJob()->abortRequested())
        return parentJob()->abortJob();

    }else{
      DUChainWriteLocker l(DUChain::lock());
      ///Add all our imports to the re-used context, just to make sure they are there.
      foreach( const LineContextPair& import, importedContentChains )
          if(!import.temporary)
            contentContext->addImportedParentContext(import.context, SimpleCursor(import.sourceLine, 0));
    }

    ///Build/update the proxy-context

    if( proxyEnvironmentFile ) {
        ContextBuilder builder(parentJob()->parseSession());
        proxyContext = builder.buildProxyContextFromContent(proxyEnvironmentFile, TopDUContextPointer(contentContext), TopDUContextPointer(updatingProxyContext));

        DUChainWriteLocker lock(DUChain::lock());

        proxyEnvironmentFile->setTopContext(proxyContext.data());
        
        Q_ASSERT(!updatingProxyContext || updatingProxyContext == proxyContext);

        if(proxyContext->importedParentContexts().isEmpty()) //Failure
          return;

        Q_ASSERT(proxyContext->importedParentContexts()[0].context() == contentContext);

        //Make sure the imported contextsa re added
        foreach ( LineContextPair context, parentJob()->includedFiles() )
          proxyContext->addImportedParentContext(context.context, SimpleCursor(context.sourceLine, 0));

        //Make sure we don't imported any not imported contexts
        foreach(DUContext::Import import, proxyContext->importedParentContexts()) {
          TopDUContext* top = dynamic_cast<TopDUContext*>(import.context());
          Q_ASSERT(top);
          bool shouldBeIncluded = false;
          if(top == contentContext)
            shouldBeIncluded = true;
          foreach(LineContextPair ctx, parentJob()->includedFiles())
            if(ctx.context == contentContext)
              shouldBeIncluded = true;
          if(!shouldBeIncluded)
            proxyContext->removeImportedParentContext(top);
        }
          
        proxyContext->clearProblems();

        //Put the problems into the proxy-context
        foreach( const ProblemPointer& problem, parentJob()->preprocessorProblems() ) {
          if(problem->finalLocation().start().line() < proxyEnvironmentFile->contentStartLine())
            proxyContext->addProblem(problem);
        }

        //Copy all problems into the proxy, because it's used to represent them.
        foreach( const ProblemPointer& problem, contentContext->problems() )
          proxyContext->addProblem(problem);
    }

    ///In the end, mark the contexts as updated.
    if(contentContext)
      parentJob()->masterJob()->setWasUpdated(contentContext);
    if(proxyContext)
      parentJob()->masterJob()->setWasUpdated(proxyContext);

    parentJob()->setDuChain(proxyContext ? proxyContext : contentContext);

    //Indicate progress
    parentJob()->setLocalProgress(1, i18n("Ready"));

    if(parentJob()->masterJob() != parentJob())
      parentJob()->masterJob()->includedFileParsed();

    // Debug output...

    if ( !parentJob()->parentPreprocessor() ) {
            DUChainReadLocker lock(DUChain::lock());
#ifdef DUMP_DUCHAIN
        kDebug( 9007 ) << "================== duchain ==================";
        KDevelop::DumpChain dump;
        dump.dump(contentContext);
#endif
#ifdef DUMP_SMART_RANGES
        if(contentContext->smartRange()) {
          kDebug() << "dumping smart range";
          KDevelop::DumpChain dump;
          kDebug() << dump.dumpRanges(contentContext->smartRange());
        }

#endif

        //KDevelop::DumpDotGraph dumpGraph;
        //kDebug(9007) << "Dot-graph:\n" << dumpGraph.dotGraph(topContext, true);
    }


    //DumpTree dumpTree;

    kDebug( 9007 ) << "===-- Parsing finished --===>" << parentJob()->document().str();

    //Check the import structure
/*    if(parentJob()->masterJob() == parentJob()) {
      DUChainReadLocker l(DUChain::lock());
      foreach(const DUContext* context, parentJob()->updated()) {
        if(context == contentContext || context == proxyContext)
          continue;
        if(static_cast<const TopDUContext*>(context)->flags() & TopDUContext::ProxyContextFlag){
//           Q_ASSERT(proxyContext->imports(context, proxyContext->range().end));
        }else{
          kDebug( 9007 ) << "lost an instance of" << context->url().str() << "with" << context->childContexts().count() << "child-contexts and " << context->localDeclarations().count() << "local declarations";
//          Q_ASSERT(contentContext->imports(context, contentContext->range().end));
        }
      }
    }*/
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

