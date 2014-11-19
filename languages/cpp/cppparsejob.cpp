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

#include "cpplanguagesupport.h"
#include "cpphighlighting.h"
#include "includepathcomputer.h"

#include "parser/parser.h"
#include "parser/control.h"
#include "parser/dumptree.h"
#include "parser/memorypool.h"

#include <unistd.h>

#include <QFile>
#include <QByteArray>
#include <QReadWriteLock>
#include <QReadLocker>

#include <KLocalizedString>

#include <ThreadWeaver/Thread>

#include "cppduchain/cpppreprocessenvironment.h"
#include "cppduchain/cppeditorintegrator.h"
#include "cppduchain/declarationbuilder.h"
#include "cppduchain/usebuilder.h"
#include "preprocessjob.h"
#include "environmentmanager.h"
#include "debug.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchaindumper.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/dumpdotgraph.h>
#include <language/interfaces/iastcontainer.h>

#include <language/backgroundparser/parsejob.h>
#include <language/backgroundparser/urlparselock.h>
#include <language/backgroundparser/backgroundparser.h>

#include <interfaces/ilanguage.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <cpppreprocessenvironment.h>
#include <language/checks/dataaccessrepository.h>
#include <language/checks/controlflowgraph.h>
#include <threadweaver/queue.h>
#include <controlflowgraphbuilder.h>
#include <usedecoratorvisitor.h>

//#define DUMP_AST
//#define DUMP_DUCHAIN

using namespace KDevelop;

bool importsContext(const QVector<DUContext::Import>& contexts, const DUContext* context) {
  foreach(const DUContext::Import &listCtx, contexts)
    if(listCtx.context(0) && listCtx.context(0)->imports(context))
      return true;
  return false;
}

QList<IndexedString> convertFromPaths(const Path::List& paths) {
  QList<IndexedString> ret;
  ret.reserve(paths.size());
  foreach(const Path& path, paths) {
    ret << IndexedString(path.pathOrUrl());
  }
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

CPPParseJob::CPPParseJob( const IndexedString& url, ILanguageSupport* languageSupport,
                    PreprocessJob* parentPreprocessor )
        : KDevelop::ParseJob( url, languageSupport ),
        m_needUpdateEverything( false ),
        m_parentPreprocessor( parentPreprocessor ),
        m_session( new ParseSession ),
        m_localPath( Path(url.str()).parent() ),
        m_includePathsComputed( 0 ),
        m_keepDuchain( false ),
        m_parsedIncludes( 0 ),
        m_needsUpdate( true )
{
    if( !m_parentPreprocessor ) {
        addJob(m_preprocessJob = ThreadWeaver::JobPointer(new PreprocessJob(this)));
        addJob(m_parseJob = ThreadWeaver::JobPointer(new CPPInternalParseJob(this)));
    } else {
        m_preprocessJob.clear();
        m_parseJob.clear();
        //The preprocessor will call parseForeground() to preprocess & parse instantly
    }
}

void CPPParseJob::setKeepDuchain(bool b) {
    m_keepDuchain = b;
}

bool CPPParseJob::keepDuchain() const {
    return m_keepDuchain;
}

void CPPParseJob::includedFileParsed() {
  ++m_parsedIncludes;
  const int estimateIncludes = 450;
  float _progress = ((float)m_parsedIncludes) / estimateIncludes;
  if(_progress > 0.8)
    _progress = 0.8f;

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

    m_preprocessJob = ThreadWeaver::JobPointer(new PreprocessJob(this));
    m_parseJob = ThreadWeaver::JobPointer(new CPPInternalParseJob(this));
    m_preprocessJob->blockingExecute();
    m_parseJob->blockingExecute();
}

Path CPPParseJob::includedFromPath() const {
    return m_includedFromPath;
}

void CPPParseJob::setIncludedFromPath( const Path& path ) {
    m_includedFromPath = path;
}

Path CPPParseJob::localPath() const
{
    return m_localPath;
}

PreprocessJob* CPPParseJob::parentPreprocessor() const {
    return m_parentPreprocessor;
}

void CPPParseJob::gotIncludePaths(IncludePathComputer* comp) {
  m_includePathsComputed = comp;
  m_waitForIncludePathsMutex.lock(); //This makes sure that the parse thread goes into the waiting state first
  m_waitForIncludePathsMutex.unlock();
  m_waitForIncludePaths.wakeAll();
}


const Path::List& CPPParseJob::includePathUrls() const {
  indexedIncludePaths();
  return masterJob()->m_includePathUrls;
}

void CPPParseJob::mergeDefines(CppPreprocessEnvironment& env) const
{
  //m_includePathsComputed is filled when includePaths() is called
  masterJob()->indexedIncludePaths();

  if(ICore::self()->shuttingDown())
    return; //If the system is shutting down, include-paths were not computed properly

  Q_ASSERT(masterJob()->m_includePathsComputed);

  QHash<QString, QString> defines = masterJob()->m_includePathsComputed->defines();

  ///@todo Most probably, the same macro-sets will be calculated again and again.
  ///           One ReferenceCountedMacroSet would be enough.

  for(QHash<QString, QString>::const_iterator it = defines.constBegin(); it != defines.constEnd(); ++it)
  {
    rpp::pp_macro* m = new rpp::pp_macro(IndexedString(it.key()));
    m->setDefinitionText( *it );

    //Call rpp::Environment::setMacro directly, so we don't add the macro to the environment-file.
    //It should be only part of the environment.
    env.rpp::Environment::setMacro(m);
  }
}

const QList<IndexedString>& CPPParseJob::indexedIncludePaths() const {
    //If a lock was held here, we would get deadlocks
    if( ICore::self()->shuttingDown() )
      return m_includePaths;

    if( masterJob() == this ) {
        if( !m_includePathsComputed ) {
            Q_ASSERT(!DUChain::lock()->currentThreadHasReadLock() && !DUChain::lock()->currentThreadHasWriteLock());
            m_waitForIncludePathsMutex.lock();
            qRegisterMetaType<CPPParseJob*>("CPPParseJob*");
            QMetaObject::invokeMethod(cpp(), "findIncludePathsForJob", Qt::QueuedConnection, Q_ARG(CPPParseJob*, const_cast<CPPParseJob*>(this)));
            //Will be woken once the include-paths are computed
            while(!m_waitForIncludePaths.wait(&m_waitForIncludePathsMutex, 1000))
            {
              if(ICore::self()->shuttingDown())
              {
                return m_includePaths;
              }
            }
            m_waitForIncludePathsMutex.unlock();
            Q_ASSERT(m_includePathsComputed);
            m_includePathsComputed->computeBackground();
            m_includePathUrls = m_includePathsComputed->result();
            m_includePaths = convertFromPaths(m_includePathUrls);

        }
        return m_includePaths;
    } else {
        return masterJob()->indexedIncludePaths();
    }
}

CPPParseJob::~CPPParseJob()
{
  delete m_includePathsComputed;
}

KDevelop::ModificationRevisionSet CPPParseJob::includePathDependencies() const {
  return KDevelop::ModificationRevisionSet();
}

CPPParseJob* CPPParseJob::masterJob() {
    if( parentPreprocessor() )
        return parentPreprocessor()->parentJob()->masterJob();
    else
        return this;
}

const CPPParseJob* CPPParseJob::masterJob() const {
    if( parentPreprocessor() )
        return parentPreprocessor()->parentJob()->masterJob();
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
  return CppLanguageSupport::self();
}

void CPPParseJob::addIncludedFile(KDevelop::ReferencedTopDUContext duChain, int sourceLine) {
    if(duChain.data()) {
      DUChainReadLocker lock(DUChain::lock());
      m_includedFiles.push_back(LineContextPair(duChain, sourceLine));
    }
}

void CPPParseJob::setProxyEnvironmentFile( Cpp::EnvironmentFile* file ) {
    m_proxyEnvironmentFile = QExplicitlySharedDataPointer<Cpp::EnvironmentFile>(file);
}

Cpp::EnvironmentFile* CPPParseJob::proxyEnvironmentFile() {
    return m_proxyEnvironmentFile.data();
}

void CPPParseJob::setContentEnvironmentFile( Cpp::EnvironmentFile* file ) {
    //Q_ASSERT(!file || file->identity().flags() & IdentifiedFile::Content);
    m_contentEnvironmentFile = QExplicitlySharedDataPointer<Cpp::EnvironmentFile>(file);
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
    return m_parentJob;
}

const KTextEditor::Range& CPPParseJob::textRangeToParse() const
{
    return m_textRangeToParse;
}

CPPInternalParseJob::CPPInternalParseJob(CPPParseJob * parent)
    : m_parentJob(parent)
    , m_initialized(false)
    , m_priority(0)
{
}

///If @param ctx is a proxy-context, returns the target-context. Else returns ctx. @warning du-chain must be locked
LineContextPair contentFromProxy(LineContextPair ctx) {
    if( ctx.context->parsingEnvironmentFile() && ctx.context->parsingEnvironmentFile()->isProxyContext() ) {
        {
          ReferencedTopDUContext ref(ctx.context);
        }
        if(ctx.context->importedParentContexts().isEmpty()) {
          qWarning() << "proxy-context for" << ctx.context->url().str() << "has no imports!" << ctx.context->ownIndex();
          ///@todo Find out how this can happen
//           Q_ASSERT(0);
          return LineContextPair(0, 0);
        }

        Q_ASSERT(!ctx.context->importedParentContexts().isEmpty());
        return LineContextPair( dynamic_cast<TopDUContext*>(ctx.context->importedParentContexts().first().context(0)), ctx.sourceLine );
    }else{
        return ctx;
    }
}

void CPPInternalParseJob::initialize() {
    if(m_initialized)
      return;
    m_initialized = true;

    updatingProxyContext = parentJob()->updatingProxyContext().data();
    updatingContentContext = parentJob()->updatingContentContext().data();

    proxyContext = updatingProxyContext;
    contentContext = updatingContentContext;
/*    if(!contentContext)
      contentContext = ContextBuilder::createEmptyTopContext(parentJob()->document());

    if(!proxyContext && Cpp::EnvironmentManager::self()->isSimplifiedMatching())
      proxyContext = ContextBuilder::createEmptyTopContext(parentJob()->document());*/
}

void CPPInternalParseJob::highlightIfNeeded()
{
    if(!ICore::self()->languageController()->backgroundParser()->trackerForUrl(parentJob()->document()))
      return;

    DUChainReadLocker l(DUChain::lock());
    ReferencedTopDUContext standardContext = DUChainUtils::standardContextForUrl(parentJob()->document().toUrl());

    qCDebug(CPP) << "Highlighting" << parentJob()->document().str();
    //If the document has a smart-range, at least re-do the highlighting
    l.unlock();
    if ( parentJob()->cpp() && parentJob()->cpp()->codeHighlighting() )
      parentJob()->cpp()->codeHighlighting()->highlightDUChain( standardContext.data() );
}

void CPPInternalParseJob::run(ThreadWeaver::JobPointer pointer, ThreadWeaver::Thread* thread)
{
    //Happens during shutdown
    if(ICore::self()->shuttingDown() || !ICore::self()->languageController()->language("C++")->languageSupport() || !parentJob()->cpp())
      return;

    //If we have a parent, the parse-mutex is already locked
    QReadLocker lock(parentJob()->parentPreprocessor() ? 0 : parentJob()->cpp()->language()->parseLock());
    if(!ICore::self()->languageController()->language("C++")->languageSupport() || !parentJob()->cpp())
      return;

    initialize();

    if(updatingContentContext)
      parentJob()->translateDUChainToRevision(updatingContentContext.data());

    if(updatingProxyContext)
      parentJob()->translateDUChainToRevision(updatingProxyContext.data());

    UrlParseLock urlLock(parentJob()->document());

    if(!parentJob()->needsUpdate()) {
      parentJob()->processDelayedImports();
      qCDebug(CPP) << "===-- ALREADY UP TO DATE --===> " << parentJob()->document().str();
      highlightIfNeeded();
      return;
    }
    if(!parentJob()->contentEnvironmentFile()) {
      //May happen when the file could not be opened or similar
      qCDebug(CPP) << "===-- Problem: Preprocessor did not create environment-file, skipping --===> " << parentJob()->document().str();
      return;
    }
    qCDebug(CPP) << "===-- PARSING --===> "
    << parentJob()->document().str();


    if (parentJob()->abortRequested())
        return /*parentJob()->abortJob()*/;

    parentJob()->setLocalProgress(0, i18n("Parsing actual file"));

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
        DUChainWriteLocker lock(DUChain::lock());
      if(!parentJob()->keepDuchain() &&
        ((!parentJob()->masterJob()->wasUpdated(contentContext) && parentJob()->needUpdateEverything())
          || !proxyContext))
            contentContext->clearImportedParentContexts();
          ///@todo Make this possible without a permanent write-lock
          contentContext->updateImportsCache();
    }

    QList<LineContextPair> importedContentChains; //All content-chains imported while this parse-run. Also contains the temporary ones.
    QList<ReferencedTopDUContext> importedTemporaryChains; //All imported content-chains that were imported temporarily from parents.
    QSet<KDevelop::IndexedString> encounteredIncludeUrls; //All imported file-urls that were encountered this run.

    {
        DUChainReadLocker lock(DUChain::lock());
        foreach ( const LineContextPair &context, parentJob()->includedFiles() ) {
            LineContextPair pair = contentFromProxy(context);
            if(!pair.context)
              continue;

            importedContentChains << pair;
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
          foreach ( const LineContextPair &topContext, currentJob->includedFiles() ) {
              //As above, we work with the content-contexts.
              LineContextPair context = contentFromProxy(topContext);
              if(!context.context)
                continue;
              DUContextPointer ptr(context.context);
              if( !importsContext(importedContentChains, context.context)  && (!updatingContentContext ||       !importsContext(updatingContentContext->importedParentContexts(), context.context)) ) {
                if(!updatingContentContext || !updatingContentContext->imports(context.context, updatingContentContext->range().end)) {
                    //This must be conditional, else we might remove needed includes later because we think they were purely temporary
                    importedContentChains << context;
                    importedContentChains.back().temporary = true;
                    importedTemporaryChains << context.context;
                }
              }
          }
          if(currentJob->parentPreprocessor())
            currentJob = currentJob->parentPreprocessor()->parentJob();
          else
            currentJob = 0;
        }
    }

    ///Now we build/update the content-context

    bool doNotChangeDUChain = false;
    bool isOpenInEditor = ICore::self()->languageController()->backgroundParser()->trackerForUrl(parentJob()->document());

    if(!parentJob()->keepDuchain()) {

      TopDUContext::Features newFeatures = parentJob()->minimumFeatures();
      if(contentContext)
        newFeatures = (TopDUContext::Features)(newFeatures | contentContext->features());

      if(newFeatures & TopDUContext::ForceUpdate)
        qCDebug(CPP) << "update enforced";

      ///At some point, we have to give up on features again, else processing will be just too slow.
      ///Simple solution for now: Always go down to the minimum required level.
      if(!contentContext || !isOpenInEditor)
        newFeatures = parentJob()->minimumFeatures();

      bool keepAST = newFeatures & TopDUContext::AST;

      //Remove update-flags like 'Recursive' or 'ForceUpdate', and the AST flag
      newFeatures = (TopDUContext::Features)(newFeatures & TopDUContext::AllDeclarationsContextsAndUses);

      TranslationUnitAST* ast = 0L;

      Control control;
      Parser parser(&control);

      if(newFeatures != TopDUContext::Empty)
      {
        ast = parser.parse( parentJob()->parseSession().data() );

        //This will be set to true if the duchain data should be left untouched
        if((ast->hadMissingCompoundTokens || control.hasProblem(KDevelop::ProblemData::Lexer)) && updatingContentContext) {
          //Make sure we don't update into a completely invalid state where everything is invalidated temporarily.
          DUChainWriteLocker l(DUChain::lock());

          if((updatingContentContext->features() & parentJob()->minimumFeatures()) ==  parentJob()->minimumFeatures() &&
            isOpenInEditor &&
              updatingContentContext->parsingEnvironmentFile()->modificationRevision().modificationTime == ModificationRevision::revisionForFile(updatingContentContext->url()).modificationTime && CppLanguageSupport::self()->codeHighlighting()->hasHighlighting(parentJob()->document())) {
            qCDebug(CPP) << "not processing" << updatingContentContext->url().str() << "because of missing compound tokens";
            ICore::self()->uiController()->showErrorMessage(i18n("Not updating duchain for %1", parentJob()->document().toUrl().fileName()), 1);
            l.unlock();
            doNotChangeDUChain = true;
            ProblemPointer problem(new Problem);
            problem->setSource(ProblemData::Parser);
            problem->setDescription("Not updating the DUChain because of serious document inconsistency");
            control.reportProblem(problem);
          }
        }

        if (parentJob()->abortRequested())
            return /*parentJob()->abortJob()*/;

        if ( ast ) {
            ast->session = parentJob()->parseSession().data();
#ifdef DUMP_AST
            DumpTree dump;
            dump.dump(ast, parentJob()->parseSession()->token_stream);
#endif
        }
      }

      qCDebug(CPP) << (contentContext ? "updating" : "building") << "duchain for" << parentJob()->document().str();

      uint oldItemCount = 0;
      if(contentContext) {
        DUChainWriteLocker l(DUChain::lock());
        contentContext->clearProblems();
        oldItemCount = contentContext->childContexts().size() + contentContext->localDeclarations().size();
      }

      DeclarationBuilder declarationBuilder(parentJob()->parseSession().data());

      if(newFeatures == TopDUContext::VisibleDeclarationsAndContexts) {
        declarationBuilder.setOnlyComputeVisible(true); //Only visible declarations/contexts need to be built.
      }
      else if(newFeatures == TopDUContext::SimplifiedVisibleDeclarationsAndContexts)
      {
        declarationBuilder.setOnlyComputeVisible(true);
        declarationBuilder.setComputeSimplified(true);
        qCDebug(CPP) << "computing simplified";
      }else if(newFeatures == TopDUContext::Empty) {
        qCDebug(CPP) << "computing empty";
        declarationBuilder.setComputeEmpty(true);
      }


      if(keepAST)
        declarationBuilder.setMapAst(true); //Set the property to map the AST & DUChain

      if(!doNotChangeDUChain) {

        if(Cpp::EnvironmentManager::self()->matchingLevel() == Cpp::EnvironmentManager::Disabled) {
            DUChainWriteLocker lock(DUChain::lock());
            if(contentContext)
              contentContext->clearImportedParentContexts();
        }

        contentContext = declarationBuilder.buildDeclarations(contentEnvironmentFile, ast, &importedContentChains, contentContext, false);

        //If publically visible declarations were added/removed, all following parsed files need to be updated
        if(declarationBuilder.changeWasSignificant()) {
          ///@todo The right solution to the whole problem: Do not put any imports into the content-contexts. Instead, Represent the complete import-structure in the proxy-contexts.
          ///      While searching, always use the perspective of the proxy. Even better: Change the context-system so proxy-contexts become completely valid contexts from the outside perspective,
          ///      that import all their imports, and that share all their content except the imports/environment-information with all the other proxy contexts for that file, and with one content-context.
          ///      Main problem: Contained Declarations/DUContexts point at their parent top-context. Which proxy-context should they point at?
          //qCDebug(CPP) << "A significant change was recorded, all following contexts will be updated";
          //parentJob()->masterJob()->setNeedUpdateEverything(true);
        }

        {
          DUChainReadLocker l(DUChain::lock());
          Q_ASSERT(!updatingContentContext || updatingContentContext == contentContext);
          Q_ASSERT(contentContext->parsingEnvironmentFile().data() == contentEnvironmentFile.data());

          if(contentContext->childContexts().size() + contentContext->localDeclarations().size() == 0) {
            if(oldItemCount != 0) {
              //To catch some problems
              qCDebug(CPP) << "All items in" << parentJob()->document().str() << "have been extincted, previous count:" << oldItemCount << "current identity offset:" << contentEnvironmentFile->identityOffset();
            }
          }
        }
      }

      {
        DUChainWriteLocker l(DUChain::lock());

        foreach( const ProblemPointer& problem, parentJob()->preprocessorProblems() ) {
          contentContext->addProblem(problem);
        }

        foreach( KDevelop::ProblemPointer p, control.problems() ) {
          contentContext->addProblem(p);
        }
      }

      if(!doNotChangeDUChain) {
        if(contentContext) {
            DUChainWriteLocker l(DUChain::lock());
            QList<TopDUContext*> remove;
            foreach(const ReferencedTopDUContext &ctx, importedTemporaryChains)
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
            foreach(const DUContext::Import &ctx, imports) {
                if(ctx.context(0) && !encounteredIncludeUrls.contains(ctx.context(0)->url()) && contentEnvironmentFile->missingIncludeFiles().set().count() == 0 && (!proxyEnvironmentFile || proxyEnvironmentFile->missingIncludeFiles().set().count() == 0)) {
                    contentContext->removeImportedParentContext(ctx.context(0));
                    qCDebug(CPP) << "removing not encountered import " << ctx.context(0)->url().str();
                }
            }
        }

        if(contentContext) {
          DUChainWriteLocker l(DUChain::lock());
          contentContext->updateImportsCache();
        }

        if (!parentJob()->abortRequested()) {
          if ((newFeatures & TopDUContext::AllDeclarationsContextsAndUses) == TopDUContext::AllDeclarationsContextsAndUses) {
              parentJob()->setLocalProgress(0.5, i18n("Building uses"));

              UseBuilder useBuilder(parentJob()->parseSession().data());
              useBuilder.setMapAst(keepAST);
              useBuilder.buildUses(ast);
              DUChainWriteLocker l(DUChain::lock());
              foreach(KDevelop::ProblemPointer problem, useBuilder.problems())
                contentContext->addProblem(problem);
          }else{
              //Delete existing uses
              DUChainWriteLocker lock( DUChain::lock() );
              contentContext->deleteUsesRecursively();
          }
        }
      }

      if (!parentJob()->abortRequested() && isOpenInEditor) {
        if ( parentJob()->cpp() && parentJob()->cpp()->codeHighlighting() )
        {
          parentJob()->cpp()->codeHighlighting()->highlightDUChain( contentContext );
        }
      }


      ///Now mark the context as not being updated. This MUST be done or we will be waiting forever in a loop
      {
        DUChainWriteLocker l(DUChain::lock());
        contentContext->setFeatures(newFeatures);
        if(proxyContext)
          proxyContext->setFeatures(newFeatures);

        //Now that the Ast is fully built, add it to the TopDUContext if requested
        if(keepAST)
        {
          qCDebug(CPP) << "AST Is being kept for" << parentJob()->document().toUrl();
          contentContext->setAst(IAstContainer::Ptr( parentJob()->parseSession().data() ));
          parentJob()->parseSession()->setASTNodeParents();
        }

        else
          contentContext->clearAst();
      }

      if (parentJob()->abortRequested())
        return /*parentJob()->abortJob()*/;

    }else{
      {
        DUChainWriteLocker l(DUChain::lock());
        if(proxyContext && contentContext)
          proxyContext->setFeatures(contentContext->features());
      }
      qCDebug(CPP) << "keeping duchain";
      highlightIfNeeded();
    }

    //Even if doNotChangeDUChain is enabled, add new imported contexts.
    //This is very useful so new added includes always work.
    if(parentJob()->keepDuchain() || doNotChangeDUChain) {
      DUChainWriteLocker l(DUChain::lock());
      ///Add all our imports to the re-used context, just to make sure they are there.
      foreach( const LineContextPair& import, importedContentChains )
          if(!import.temporary)
            contentContext->addImportedParentContext(import.context, CursorInRevision(import.sourceLine, 0));
      contentContext->updateImportsCache();
    }

    if(!doNotChangeDUChain) {
      DUChainReadLocker lock(DUChain::lock());
      foreach(const LineContextPair& import, parentJob()->includedFiles()) {
        if(import.temporary)
          continue;
        LineContextPair context = contentFromProxy(import);
        if(!context.context)
          continue;
        Q_ASSERT(context.context);
        if(!contentContext->imports(context.context.data(), CursorInRevision::invalid())) {
          qWarning() << "Context should be imported, but is not:" << contentContext->url().str() << " <- " << context.context->url().str();
        }
      }
    }

    ///Build/update the proxy-context

    if( proxyEnvironmentFile ) {
        ContextBuilder builder(parentJob()->parseSession().data());
        if(Cpp::EnvironmentManager::self()->matchingLevel() == Cpp::EnvironmentManager::Disabled) {
            DUChainWriteLocker lock(DUChain::lock());
            if(updatingProxyContext)
              updatingProxyContext->clearImportedParentContexts();
        }

        proxyContext = builder.buildProxyContextFromContent(proxyEnvironmentFile, TopDUContextPointer(contentContext), TopDUContextPointer(updatingProxyContext));

        DUChainWriteLocker lock(DUChain::lock());

        Q_ASSERT(!updatingProxyContext || updatingProxyContext == proxyContext);

        if(proxyContext->importedParentContexts().isEmpty()) {
          Q_ASSERT(0); //Failure
        }

        Q_ASSERT(DUChainUtils::contentContextFromProxyContext(proxyContext) == contentContext.data());

        //Make sure the imported contextsa re added
        foreach ( const LineContextPair &context, parentJob()->includedFiles() )
          proxyContext->addImportedParentContext(context.context, CursorInRevision(context.sourceLine, 0));

        proxyContext->updateImportsCache();

        proxyContext->clearProblems();

        //Put the problems into the proxy-context
        foreach( const ProblemPointer& problem, parentJob()->preprocessorProblems() ) {
          if(problem->range().start.line < proxyEnvironmentFile->contentStartLine())
            proxyContext->addProblem(problem);
        }

        //Copy all problems into the proxy, because it's used to represent them.
        foreach( const ProblemPointer& problem, contentContext->problems() )
          proxyContext->addProblem(problem);
    }

    if(proxyContext) {
      DUChainReadLocker lock(DUChain::lock());
      Q_ASSERT(!proxyContext->importedParentContexts().isEmpty());
    }

    {
      //Update include-path dependencies
      DUChainWriteLocker lock(DUChain::lock());
      if(proxyEnvironmentFile)
        proxyEnvironmentFile->setIncludePathDependencies(parentJob()->includePathDependencies());

      if(contentEnvironmentFile)
        contentEnvironmentFile->setIncludePathDependencies(parentJob()->includePathDependencies());
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
        qCDebug(CPP) << "================== duchain ==================";
        DUChainDumper dumper;
        dumper.dump(contentContext);
#endif

        //KDevelop::DumpDotGraph dumpGraph;
        //qCDebug(CPP) << "Dot-graph:\n" << dumpGraph.dotGraph(topContext, true);
    }

    qCDebug(CPP) << "===-- Parsing finished --===>" << parentJob()->document().str();

    parentJob()->processDelayedImports();
}

void CPPParseJob::processDelayedImports() {
  QSharedPointer<CPPInternalParseJob> parseJob = this->parseJob();
  if(!m_delayedImports.isEmpty()) {
    foreach(const LineJobPair& job, m_delayedImports)
      job.first->addDelayedImporter(LineContextPair(parseJob->proxyContext ? parseJob->proxyContext : parseJob->contentContext, job.second));
    m_delayedImports.clear();
  }
  if(!m_delayedImporters.isEmpty()) {
    DUChainWriteLocker l(DUChain::lock());
    foreach(const LineContextPair& context, m_delayedImporters) {
      Q_ASSERT(context.context->parsingEnvironmentFile());
      if(context.context->parsingEnvironmentFile()->isProxyContext()) {
        Q_ASSERT(parseJob->proxyContext);
        context.context->addImportedParentContext(parseJob->proxyContext.data(), CursorInRevision(context.sourceLine, 0));
        Cpp::EnvironmentFile* cppEnvFile = dynamic_cast<Cpp::EnvironmentFile*>(context.context->parsingEnvironmentFile().data());
        Q_ASSERT(cppEnvFile);
        cppEnvFile->merge(dynamic_cast<Cpp::EnvironmentFile&>(*parseJob->proxyContext->parsingEnvironmentFile().data()));
        context.context->updateImportsCache();
      }
      Q_ASSERT(parseJob->contentContext);
      LineContextPair content = contentFromProxy(context);
      if(!content.context)
        continue;
      Q_ASSERT(content.context);
      content.context->addImportedParentContext(parseJob->proxyContext.data(), CursorInRevision(content.sourceLine, 0));
      content.context->updateImportsCache();
      Cpp::EnvironmentFile* cppEnvFile = dynamic_cast<Cpp::EnvironmentFile*>(content.context->parsingEnvironmentFile().data());
      Q_ASSERT(cppEnvFile);
      cppEnvFile->merge(dynamic_cast<Cpp::EnvironmentFile&>(*parseJob->contentContext->parsingEnvironmentFile().data()));
    }
  }
}

void CPPParseJob::addDelayedImport(LineJobPair job) {
  m_delayedImports << job;
}

void CPPParseJob::addDelayedImporter(LineContextPair duChain) {
  m_delayedImporters << duChain;
}

void CPPParseJob::requestDependancies()
{
}

ParseSession::Ptr CPPParseJob::parseSession() const
{
    return m_session;
}

QSharedPointer<CPPInternalParseJob> CPPParseJob::parseJob() const
{
    return m_parseJob.dynamicCast<CPPInternalParseJob>();
}

int CPPInternalParseJob::priority() const
{
    return m_priority;
}

void CPPInternalParseJob::setPriority(int priority)
{
    m_priority = priority;
}

TopDUContext::Features CPPParseJob::slaveMinimumFeatures() const
{
    TopDUContext::Features slaveMinimumFeatures = KDevelop::TopDUContext::Empty;
    if (minimumFeatures() & TopDUContext::SimplifiedVisibleDeclarationsAndContexts) {
      slaveMinimumFeatures = TopDUContext::SimplifiedVisibleDeclarationsAndContexts;
    }

    if(minimumFeatures() & TopDUContext::Recursive)
      slaveMinimumFeatures = (TopDUContext::Features)(minimumFeatures() & (~TopDUContext::ForceUpdate));
    else if((minimumFeatures() & TopDUContext::VisibleDeclarationsAndContexts) == TopDUContext::VisibleDeclarationsAndContexts)
      slaveMinimumFeatures = TopDUContext::VisibleDeclarationsAndContexts;

    if((minimumFeatures() & TopDUContext::ForceUpdateRecursive) == TopDUContext::ForceUpdateRecursive)
      slaveMinimumFeatures = (TopDUContext::Features)(slaveMinimumFeatures | TopDUContext::ForceUpdateRecursive);

    //The selected minimum features are required on all imported contexts recursively
    return (TopDUContext::Features)(slaveMinimumFeatures | TopDUContext::Recursive);
}

ControlFlowGraph* CPPParseJob::controlFlowGraph()
{
  ControlFlowGraph* ret = new ControlFlowGraph;
  ControlFlowGraphBuilder flowvisitor(duChain(), m_session.data(), ret);
  flowvisitor.run(m_session->topAstNode());
  return ret;
}

DataAccessRepository* CPPParseJob::dataAccessInformation()
{
  DataAccessRepository* ret = new DataAccessRepository;
  UseDecoratorVisitor visit(m_session.data(), ret);
  visit.run(m_session->topAstNode());
  return ret;
}


