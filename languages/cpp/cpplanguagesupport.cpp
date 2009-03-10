/*
* KDevelop C++ Language Support
*
* Copyright 2005 Matt Rogers <mattr@kde.org>
* Copyright 2006 Adam Treat <treat@kde.org>
* Copyright 2007-2008 David Nolden<david.nolden.kdevelop@art-master.de>
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
#include "cpplanguagesupport.h"
#include <config.h>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSet>
#include <QApplication>
#include <QAction>
#include <QTimer>
#include <QReadWriteLock>
#include <kactioncollection.h>
#include <kaction.h>
#include <QExtensionFactory>
#include <QtDesigner/QExtensionFactory>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kio/netaccess.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/smartinterface.h>
#include <language/codecompletion/codecompletion.h>

#include <interfaces/icore.h>
#include <language/interfaces/iproblem.h>
#include <interfaces/iproject.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <language/interfaces/iquickopen.h>
#include <interfaces/iplugincontroller.h>
#include <project/projectmodel.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/stringhelpers.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/smartconverter.h>
#include <language/duchain/functiondefinition.h>
#include <interfaces/contextmenuextension.h>

#include "preprocessjob.h"
#include "rpp/preprocessor.h"
#include "ast.h"
#include "parsesession.h"
#include "cpphighlighting.h"
#include "cppparsejob.h"
#include "codecompletion/model.h"
#include "cppeditorintegrator.h"
#include "usebuilder.h"
#include "environmentmanager.h"
#include "cppduchain/navigation/navigationwidget.h"
#include "cppduchain/cppduchain.h"
#include "veritas/testswitch.h"
#include "veritas/stubcontextaction.h"
#include "veritas/uutcontextaction.h"
#include "codegen/cppnewclass.h"

#include "includepathresolver.h"
#include "setuphelpers.h"
#include "quickopen.h"
#include "cppdebughelper.h"
#include "codegen/simplerefactoring.h"
#include "includepathcomputer.h"
#include "codecompletion/missingincludemodel.h"
//#include <valgrind/callgrind.h>


// #define CALLGRIND_TRACE_UI_LOCKUP

#define DEBUG_UI_LOCKUP
#define LOCKUP_INTERVAL 140

#ifdef CALLGRIND_TRACE_UI_LOCKUP
#define DEBUG_UI_LOCKUP
#define LOCKUP_INTERVAL 5
#endif

//List of possible headers used for definition/declaration fallback switching
QStringList headerExtensions(QString("h,H,hh,hxx,hpp,tlh,h++").split(','));
QStringList sourceExtensions(QString("c,cc,cpp,c++,cxx,C,m,mm,M,inl,_impl.h").split(','));

QString addDot(QString ext) {
  if(ext.contains('.')) //We need this check because of the _impl.h thing
    return ext;
  else
    return "." + ext;
}

KTextEditor::Cursor normalizeCursor(KTextEditor::Cursor c) {
  c.setColumn(0);
  return c;
}


using namespace KDevelop;

CppLanguageSupport* CppLanguageSupport::m_self = 0;

KDevelop::ContextMenuExtension CppLanguageSupport::contextMenuExtension(KDevelop::Context* context)
{
    ContextMenuExtension cm;
    m_stubAction->appendTo(cm, context);
    m_uutAction->appendTo(cm, context);
    SimpleRefactoring::self().doContextMenu(cm, context);
    return cm;
}

///Tries to find a definition for the declaration at given cursor-position and document-url. DUChain must be locked.
Declaration* definitionForCursorDeclaration(const KDevelop::SimpleCursor& cursor, const KUrl& url) {
  QList<TopDUContext*> topContexts = DUChain::self()->chainsForDocument( url );
  foreach(TopDUContext* ctx, topContexts) {
    Declaration* decl = DUChainUtils::declarationInLine(cursor, ctx);
    if(decl && FunctionDefinition::definition(decl))
      return FunctionDefinition::definition(decl);
  }
  return 0;
}

QStringList CppLanguageSupport::standardIncludePaths() const {
  return *m_standardIncludePaths;
}

K_PLUGIN_FACTORY(KDevCppSupportFactory, registerPlugin<CppLanguageSupport>(); )
K_EXPORT_PLUGIN(KDevCppSupportFactory(KAboutData("kdevcppsupport","kdevcpp", ki18n("C++ Support"), "0.1", ki18n("Support for C++ Language"), KAboutData::License_GPL)))

CppLanguageSupport::CppLanguageSupport( QObject* parent, const QVariantList& /*args*/ )
    : KDevelop::IPlugin( KDevCppSupportFactory::componentData(), parent ),
      KDevelop::ILanguageSupport(),
      m_standardMacros(0),
      m_standardEnvironment(0)
{
    m_self = this;

    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    setXMLFile( "kdevcppsupport.rc" );

    m_highlights = new CppHighlighting( this );
    m_cc = new KDevelop::CodeCompletion( this, new Cpp::CodeCompletionModel(0), name() );
    m_missingIncludeCompletion = new KDevelop::CodeCompletion( this, new Cpp::MissingIncludeCompletionModel(0), name() );
    m_standardMacros = new Cpp::ReferenceCountedMacroSet;
    m_standardIncludePaths = new QStringList;
    
    Cpp::EnvironmentManager::setSimplifiedMatching(true);
    
    Cpp::EnvironmentManager::setMatchingLevel(Cpp::EnvironmentManager::Disabled);
//     Cpp::EnvironmentManager::setMatchingLevel(Cpp::EnvironmentManager::Naive);
//     Cpp::EnvironmentManager::setMatchingLevel(Cpp::EnvironmentManager::Full);

    m_includeResolver = new CppTools::IncludePathResolver;
    // Retrieve the standard include paths & macro definitions for this machine.
    // Uses gcc commands to retrieve the information.
    CppTools::setupStandardIncludePaths(*m_standardIncludePaths);
    CppTools::setupStandardMacros(*m_standardMacros);
    
    m_standardEnvironment = PreprocessJob::createStandardEnvironment();

    connect( core()->projectController(),
             SIGNAL( projectOpened( KDevelop::IProject* ) ),
             this, SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( core()->projectController(),
             SIGNAL( projectClosing( KDevelop::IProject* ) ),
             this, SLOT( projectClosing( KDevelop::IProject* ) ) );

    m_quickOpenDataProvider = new IncludeFileDataProvider();

    IQuickOpen* quickOpen = core()->pluginController()->extensionForPlugin<IQuickOpen>("org.kdevelop.IQuickOpen");

    if( quickOpen )
        quickOpen->registerProvider( IncludeFileDataProvider::scopes(), QStringList(i18n("Files")), m_quickOpenDataProvider );
    else
        kWarning() << "Quickopen not found";

    KActionCollection* actions = actionCollection();

    KAction* switchDefinitionDeclaration = actions->addAction("switch_definition_declaration");
    switchDefinitionDeclaration->setText( i18n("&Switch Definition/Declaration") );
    switchDefinitionDeclaration->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_C );
    connect(switchDefinitionDeclaration, SIGNAL(triggered(bool)), this, SLOT(switchDefinitionDeclaration()));

    KAction* newClassAction = actions->addAction("code_new_class");
    newClassAction->setText( i18n("Create &New Class") );
    connect(newClassAction, SIGNAL(triggered(bool)), this, SLOT(newClassWizard()));

    Veritas::TestSwitch* ts = new Veritas::TestSwitch(this);
    ts->setStandardMacros(m_standardMacros);
    ts->connectAction(actionCollection());

    m_stubAction = new Veritas::StubContextAction(this);
    m_stubAction->setup();
    m_uutAction = new Veritas::UUTContextAction(this);
    m_uutAction->setup();

#ifdef DEBUG_UI_LOCKUP
    m_blockTester = new UIBlockTester(LOCKUP_INTERVAL);
#endif
}

bool CppLanguageSupport::isHeader(const KUrl &url) const {
  QFileInfo fi( url.path() );
  QString path = fi.filePath();
  // extract the exension
  QString ext = fi.suffix();
  if ( ext.isEmpty() )
    return true;
  
  return headerExtensions.contains(ext);
}

KUrl CppLanguageSupport::sourceOrHeaderCandidate( const KUrl &url, bool fast ) const
{
  QString urlPath = url.path(); ///@todo Make this work with real urls

// get the path of the currently active document
  QFileInfo fi( urlPath );
  QString path = fi.filePath();
  // extract the exension
  QString ext = fi.suffix();
  if ( ext.isEmpty() )
    return KUrl();
  // extract the base path (full path without '.' and extension)
  QString base = path.left( path.length() - ext.length() - 1 );
  //kDebug( 9007 ) << "base: " << base << ", ext: " << ext << endl;
  // just the filename without the extension
  QString fileNameWoExt = fi.fileName();
  if ( !ext.isEmpty() )
    fileNameWoExt.replace( "." + ext, "" );
  QStringList possibleExts;
  // depending on the current extension assemble a list of
  // candidate files to look for
  QStringList candidates;
  // special case for template classes created by the new class dialog
  if ( path.endsWith( "_impl.h" ) )
  {
    QString headerpath = path;
    headerpath.replace( "_impl.h", ".h" );
    candidates << headerpath;
    fileNameWoExt.replace( "_impl", "" );
    possibleExts << "h";
  }
  // if file is a header file search for implementation file
  else if ( headerExtensions.contains( ext ) )
  {
    foreach(const QString& ext, sourceExtensions)
      candidates << ( base + addDot(ext) );

    possibleExts = sourceExtensions;
  }
  // if file is an implementation file, search for header file
  else if ( sourceExtensions.contains( ext ) )
  {
    foreach(const QString& ext, headerExtensions)
      candidates << ( base + addDot(ext) );

    possibleExts = headerExtensions;
  }
  // search for files from the assembled candidate lists, return the first
  // candidate file that actually exists or QString::null if nothing is found.
  QStringList::ConstIterator it;
  for ( it = candidates.constBegin(); it != candidates.constEnd(); ++it )
  {
//     kDebug( 9007 ) << "Trying " << ( *it ) << endl;
    if ( QFileInfo( *it ).exists() )
    {
//       kDebug( 9007 ) << "using: " << *it << endl;
      return * it;
    }
  }

  if(fast)
    return KUrl();

  //kDebug( 9007 ) << "Now searching in project files." << endl;
  // Our last resort: search the project file list for matching files
  KUrl::List projectFileList;

  foreach (KDevelop::IProject *project, core()->projectController()->projects()) {
      if (project->inProject(url)) {
        QList<ProjectFileItem*> files = project->files();
        foreach(ProjectFileItem* file, files)
          projectFileList << file->url();
      }
  }
  QFileInfo candidateFileWoExt;
  QString candidateFileWoExtString;
  foreach ( const KUrl& url, projectFileList )
  {
    candidateFileWoExt.setFile(url.path());
    //kDebug( 9007 ) << "candidate file: " << url << endl;
    if( !candidateFileWoExt.suffix().isEmpty() )
      candidateFileWoExtString = candidateFileWoExt.fileName().replace( "." + candidateFileWoExt.suffix(), "" );

    if ( candidateFileWoExtString == fileNameWoExt )
    {
      if ( possibleExts.contains( candidateFileWoExt.suffix() ) || candidateFileWoExt.suffix().isEmpty() )
      {
        //kDebug( 9007 ) << "checking if " << url << " exists" << endl;
        return url;
      }
    }
  }
  return KUrl();
}

///@todo Make this work again with non-class declarations/definitions
void CppLanguageSupport::switchDefinitionDeclaration()
{
  kDebug(9007) << "switching definition/declaration";
  ///Step 1: Find the current top-level context of type DUContext::Other(the highest code-context).
  ///-- If it belongs to a function-declaration or definition, it can be retrieved through owner(), and we are in a definition.
  ///-- If no such context could be found, search for a declaration on the same line as the cursor, and switch to the according definition
  KDevelop::IDocument* doc = core()->documentController()->activeDocument();
  if(!doc || !doc->textDocument() || !doc->textDocument()->activeView()) {
    kDebug(9007) << "No active document";
    return;
  }
  kDebug(9007) << "Document:" << doc->url();

  DUChainReadLocker lock(DUChain::lock());

  TopDUContext* standardCtx = standardContext(doc->url());
  if(standardCtx) {
    Declaration* definition = 0;
    SimpleCursor cursor = SimpleCursor(doc->textDocument()->activeView()->cursorPosition());

    DUContext* ctx = standardCtx->findContext(cursor);
    if(!ctx)
      ctx = standardCtx;

    if(ctx)
      kDebug() << "found context" << ctx->scopeIdentifier();
    else
      kDebug() << "found no context";

    while(ctx && ctx->parentContext() && ctx->parentContext()->type() == DUContext::Other)
      ctx = ctx->parentContext();

    if(ctx && ctx->owner() && ctx->type() == DUContext::Other && ctx->owner()->isDefinition()) {
      definition = ctx->owner();
      kDebug() << "found definition while traversing:" << definition->toString();
    }

    if(!definition && ctx) {
      definition = DUChainUtils::declarationInLine(cursor, ctx);
      if(definition)
        kDebug() << "found definition using declarationInLine:" << definition->toString();
      else
        kDebug() << "not found definition using declarationInLine";
    }

    FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(definition);
    if(def && def->declaration()) {
      Declaration* declaration = def->declaration();
      KTextEditor::Range targetRange = declaration->range().textRange();
      KUrl url(declaration->url().str());
      kDebug() << "found definition that has declaration: " << definition->toString() << "range" << targetRange << "url" << url;
      lock.unlock();

      KDevelop::IDocument* document = core()->documentController()->documentForUrl(url);
      
      if(!document || 
          (document && document->textDocument() && document->textDocument()->activeView() && !targetRange.contains(document->textDocument()->activeView()->cursorPosition()))) {
        KTextEditor::Cursor pos(normalizeCursor(targetRange.start()));
        core()->documentController()->openDocument(url, KTextEditor::Range(pos, pos));
      }
      return;
    }else{
      kDebug(9007) << "Definition has no assigned declaration";
    }

    kDebug(9007) << "Could not get definition/declaration from context";
  }else{
    kDebug(9007) << "Got no context for the current document";
  }

  Declaration* def = definitionForCursorDeclaration(SimpleCursor(doc->textDocument()->activeView()->cursorPosition()), doc->url());

  if(def) {
    KUrl url(def->url().str());
    KTextEditor::Range targetRange = def->range().textRange();

    ///@todo If the cursor is already in the target context, do not move it.
    if(def->internalContext()) {
      targetRange.end() = def->internalContext()->range().end.textCursor();
    }else{
      kDebug(9007) << "Declaration does not have internal context";
    }
    lock.unlock();

    KDevelop::IDocument* document = core()->documentController()->documentForUrl(url);
    
    if(!document || 
        (document && document->textDocument() && (!document->textDocument()->activeView() || !targetRange.contains(document->textDocument()->activeView()->cursorPosition())))) {
      KTextEditor::Cursor pos(normalizeCursor(targetRange.start()));
      core()->documentController()->openDocument(url, KTextEditor::Range(pos, pos));
    }else if(document) {
      //The cursor is already in the target range, only open the document
      core()->documentController()->openDocument(url);
    }
    return;
  }else{
    kWarning(9007) << "Found no definition assigned to cursor position";
  }

  lock.unlock();
  ///- If no definition/declaration could be found to switch to, just switch the document using normal header/source heuristic by file-extension
  KUrl url = sourceOrHeaderCandidate(doc->textDocument()->url());

  if(url.isValid()) {
    core()->documentController()->openDocument(url);
  }else{
    kDebug(9007) << "Found no source/header candidate to switch";
  }
}

CppLanguageSupport::~CppLanguageSupport()
{
    ILanguage* lang = language();
    if (lang) {
        lang->parseLock()->lockForWrite();
        m_self = 0; //By locking the parse-mutexes, we make sure that parse- and preprocess-jobs get a chance to finish in a good state
        lang->parseLock()->unlock();
    }

    delete m_quickOpenDataProvider;

    // Remove any documents waiting to be parsed from the background paser.
    core()->languageController()->backgroundParser()->clear(this);


    delete m_standardMacros;
    delete m_standardIncludePaths;
    delete m_includeResolver;
#ifdef DEBUG_UI_LOCKUP
    delete m_blockTester;
#endif
}

const Cpp::ReferenceCountedMacroSet& CppLanguageSupport::standardMacros() const {
    return *m_standardMacros;
}

const ParsingEnvironment* CppLanguageSupport::standardEnvironment() const {
  return m_standardEnvironment;
}

CppLanguageSupport* CppLanguageSupport::self() {
    return m_self;
}

KDevelop::ParseJob *CppLanguageSupport::createParseJob( const KUrl &url )
{
    return new CPPParseJob( url );
}

const KDevelop::ICodeHighlighting *CppLanguageSupport::codeHighlighting() const
{
    return m_highlights;
}

void CppLanguageSupport::projectOpened(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    // FIXME Add signals slots from the filemanager for:
    //       1. filesAddedToProject
    //       2. filesRemovedFromProject
    //       3. filesChangedInProject

    ///@todo Be more clever about getting include-paths for header-files: They are not directly part of the project, but usually they have an assigned source-file, so use the include-path from that file. Then we won't need to do the ordering.

    //Since there may be additional information like include-paths available now, reparse all open documents
    QList<IDocument*> headers;
    foreach(IDocument* doc, core()->documentController()->openDocuments()) {
        if(project->inProject(doc->url())) {
          bool isSource = false;
          QString path = doc->url().path();

          foreach(const QString& str, sourceExtensions)
            if(path.endsWith(str))
              isSource = true;

          if(isSource) //Add source-files first, because their include-paths may be important for headers
            core()->languageController()->backgroundParser()->addDocument(doc->url());
          else
            headers << doc;
        }
    }

    foreach(IDocument* doc, headers)
      core()->languageController()->backgroundParser()->addDocument(doc->url());
}

void CppLanguageSupport::projectClosing(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    //TODO: Anything to do here?!?!
}

void CppLanguageSupport::findIncludePathsForJob(CPPParseJob* job)
{
  IncludePathComputer* comp = new IncludePathComputer(KUrl(job->document().str()), job->preprocessorProblemsPointer());
  comp->computeForeground();
  job->gotIncludePaths(comp);
}

KUrl::List CppLanguageSupport::findIncludePaths(const KUrl& source, QList<KDevelop::ProblemPointer>* problems) const {
  IncludePathComputer comp(source, problems);
  comp.computeForeground();
  comp.computeBackground();
  return comp.result();
}

QList<Cpp::IncludeItem> CppLanguageSupport::allFilesInIncludePath(const KUrl& source, bool local, const QString& addPath) const {

    QMap<KUrl, bool> hadPaths; //Only process each path once
    QList<Cpp::IncludeItem> ret;

    KUrl::List paths = findIncludePaths(source, 0);

    if(local) {
        KUrl localPath = source;
        localPath.setFileName(QString());
        paths.push_front(localPath);
    }

    int pathNumber = 0;

    foreach(const KUrl& path, paths)
    {
        if(!hadPaths.contains(path)) {
            hadPaths[path] = true;
        }else{
            continue;
        }
        if(!path.isLocalFile()) {
            kDebug(9007) << "include-path " << path << " is not local";
            continue;
        }
        KUrl searchPathUrl = path;
        searchPathUrl.addPath(addPath);
        QString searchPath = searchPathUrl.path();
        QDirIterator dirContent(searchPath);

        while(dirContent.hasNext()) {
             dirContent.next();
            Cpp::IncludeItem item;
            item.name = dirContent.fileName();
            if(item.name.startsWith('.') || item.name.endsWith("~")) //This filters out ".", "..", and hidden files, and backups
              continue;
            if(!dirContent.fileInfo().suffix().isEmpty() && !headerExtensions.contains(dirContent.fileInfo().suffix()))
              continue;
            item.isDirectory = dirContent.fileInfo().isDir();
            item.basePath = searchPath;
            item.pathNumber = pathNumber;

            ret << item;
        }
        ++pathNumber;
    }

    return ret;
}

QPair<KUrl, KUrl> CppLanguageSupport::findInclude(const KUrl::List& includePaths, const KUrl& localPath, const QString& includeName, int includeType, const KUrl& skipPath, bool quiet) const {
    QPair<KUrl, KUrl> ret;
#ifdef DEBUG
    kDebug(9007) << "searching for include-file" << includeName;
    if( !skipPath.isEmpty() )
        kDebug(9007) << "skipping path" << skipPath;
#endif

    if (includeType == rpp::Preprocessor::IncludeLocal && localPath != skipPath) {
      QString check = localPath.path(KUrl::AddTrailingSlash) + includeName;
        QFileInfo info(check);
        if (info.exists() && info.isReadable() && info.isFile()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.absoluteFilePath());
            ret.first.cleanPath();
            ret.second = localPath;
            return ret;
        }
    }

    //When a path is skipped, we will start searching exactly after that path
    bool needSkip = !skipPath.isEmpty();

restart:
    foreach( const KUrl& path, includePaths ) {
        if( needSkip ) {
            if( path == skipPath ) {
                needSkip = false;
                continue;
            }
        }

        QString check = path.path(KUrl::AddTrailingSlash) + includeName;
        QFileInfo info(check);

        if (info.exists() && info.isReadable() && info.isFile()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.absoluteFilePath());
            ret.first.cleanPath();
            ret.second = path.path();
            return ret;
        }
    }

    if( needSkip ) {
        //The path to be skipped was not found, so simply start from the begin, considering any path.
        needSkip = false;
        goto restart;
    }

    if( ret.first.isEmpty() && !quiet ) {
        kDebug(9007) << "FAILED to find include-file" << includeName << "in paths:";
        foreach( const KUrl& path, includePaths )
            kDebug(9007) << path;
    }
    
    return ret;
}

QString CppLanguageSupport::name() const
{
    return "C++";
}

KDevelop::ILanguage *CppLanguageSupport::language()
{
    return core()->languageController()->language(name());
}

TopDUContext* CppLanguageSupport::standardContext(const KUrl& url, bool allowProxyContext)
{
  DUChainReadLocker lock(DUChain::lock());
  const ParsingEnvironment* env = standardEnvironment();
  KDevelop::TopDUContext* top;
  top = KDevelop::DUChain::self()->chainForDocument(url, env, Cpp::EnvironmentManager::isSimplifiedMatching());

  if( !top ) {
    //kDebug(9007) << "Could not find perfectly matching version of " << url << " for completion";
    //Preferably pick a context that is not empty
    QList<TopDUContext*> candidates = DUChain::self()->chainsForDocument(url);
    foreach(TopDUContext* candidate, candidates)
      if(!candidate->localDeclarations().isEmpty() || !candidate->childContexts().isEmpty())
      top = candidate;
    if(!top && !candidates.isEmpty())
      top = candidates[0];
  }

  if(top && (top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->isProxyContext()) && !allowProxyContext)
  {
    if(!top->importedParentContexts().isEmpty())
    {
      top = dynamic_cast<TopDUContext*>(top->importedParentContexts().first().context(0));

      if(!top)
        kDebug(9007) << "WARNING: Proxy-context had invalid content-context";

    } else {
      kDebug(9007) << "ERROR: Proxy-context has no content-context";
    }
  }

  return top;
}

QPair<QPair<QString, SimpleRange>, QString> CppLanguageSupport::cursorIdentifier(const KUrl& url, const SimpleCursor& position) const {
  KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
  if(!doc || !doc->textDocument() || !doc->textDocument()->activeView())
    return qMakePair(qMakePair(QString(), SimpleRange::invalid()), QString());

  int lineNumber = position.line;
  int lineLength = doc->textDocument()->lineLength(lineNumber);

  QString line = doc->textDocument()->text(KTextEditor::Range(lineNumber, 0, lineNumber, lineLength));

  if(line.trimmed().startsWith("#include")) { //If it is an include, return the complete line
    int start = 0;
    while(start < lineLength && line[start] == ' ')
      ++start;

    return qMakePair( qMakePair(line, SimpleRange(lineNumber, start, lineNumber, lineLength)), QString() );
  }

  int start = position.column;
  int end = position.column;

  while(start > 0 && (line[start].isLetterOrNumber() || line[start] == '_') && (line[start-1].isLetterOrNumber() || line[start-1] == '_'))
    --start;

  while(end <  lineLength && (line[end].isLetterOrNumber() || line[end] == '_'))
    ++end;

  SimpleRange wordRange = SimpleRange(SimpleCursor(lineNumber, start), SimpleCursor(lineNumber, end));

  return qMakePair( qMakePair(line.mid(start, end-start), wordRange), line.mid(end) );
}

QPair<TopDUContextPointer, SimpleRange> CppLanguageSupport::importedContextForPosition(const KUrl& url, const SimpleCursor& position) {
  QPair<QPair<QString, SimpleRange>, QString> found = cursorIdentifier(url, position);
  if(!found.first.second.isValid())
    return qMakePair(TopDUContextPointer(), SimpleRange::invalid());

  QString word(found.first.first);
  SimpleRange wordRange(found.first.second);

  for(int pos = wordRange.end.column-wordRange.start.column-1; pos >= 0; --pos) {
    if(word[pos] == '"' || word[pos].isSpace() || word[pos] == '>')
      --wordRange.end.column;
    else
      break;
  }

  for(int pos = 0; pos < word.size(); ++pos) {
    ++wordRange.start.column;
    if(word[pos] == '"' || word[pos] == '<')
      break;
  }

  if(wordRange.start > wordRange.end)
    wordRange.start = wordRange.end;

  //Since this is called by the editor while editing, use a fast timeout so the editor stays responsive
  DUChainReadLocker lock(DUChain::lock(), 100);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return qMakePair(TopDUContextPointer(), SimpleRange::invalid());
  }

  TopDUContext* ctx = standardContext(url);
  if(word.isEmpty() || !ctx || !ctx->parsingEnvironmentFile())
    return qMakePair(TopDUContextPointer(), SimpleRange::invalid());

  if((ctx->parsingEnvironmentFile() && ctx->parsingEnvironmentFile()->isProxyContext())) {
    kDebug() << "Strange: standard-context for" << ctx->url().str() << "is a proxy-context";
    return qMakePair(TopDUContextPointer(), SimpleRange::invalid());
  }

  Cpp::EnvironmentFilePointer p(dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data()));

  Q_ASSERT(p);

  if(word.startsWith("#include")) {
    //It's an #include, find out which file was included at the given line
    foreach(const DUContext::Import &imported, ctx->importedParentContexts()) {
      if(imported.context(0)) {
        if(ctx->importPosition(imported.context(0)).line == wordRange.start.line) {
          if(TopDUContext* importedTop = dynamic_cast<TopDUContext*>(imported.context(0)))
            return qMakePair(TopDUContextPointer(importedTop), wordRange);
        }
      }
    }
  }
  return qMakePair(TopDUContextPointer(), SimpleRange::invalid());
}

QPair<SimpleRange, const rpp::pp_macro*> CppLanguageSupport::usedMacroForPosition(const KUrl& url, const SimpleCursor& position) {
  //Extract the word under the cursor

  QPair<QPair<QString, SimpleRange>, QString> found = cursorIdentifier(url, position);
  if(!found.first.second.isValid())
    return qMakePair(SimpleRange::invalid(), (const rpp::pp_macro*)0);

  IndexedString word(found.first.first);
  SimpleRange wordRange(found.first.second);

  //Since this is called by the editor while editing, use a fast timeout so the editor stays responsive
  DUChainReadLocker lock(DUChain::lock(), 100);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return qMakePair(SimpleRange::invalid(), (const rpp::pp_macro*)0);
  }

  TopDUContext* ctx = standardContext(url, true);
  if(word.str().isEmpty() || !ctx || !ctx->parsingEnvironmentFile())
    return qMakePair(SimpleRange::invalid(), (const rpp::pp_macro*)0);

  Cpp::EnvironmentFilePointer p(dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data()));

  Q_ASSERT(p);

  if(!p->usedMacroNames().contains(word) && !p->definedMacroNames().contains(word))
    return qMakePair(SimpleRange::invalid(), (const rpp::pp_macro*)0);

  //We need to do a flat search through all macros here, which really hurts

  Cpp::ReferenceCountedMacroSet::Iterator it = p->usedMacros().iterator();

  while(it) {
    if(it.ref().name == word && !it.ref().isUndef())
      return qMakePair(wordRange, &it.ref());
    ++it;
  }

  it = p->definedMacros().iterator();
  while(it) {
    if(it.ref().name == word && !it.ref().isUndef())
      return qMakePair(wordRange, &it.ref());
    ++it;
  }

  return qMakePair(SimpleRange::invalid(), (const rpp::pp_macro*)0);
}

SimpleRange CppLanguageSupport::specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position) {

  QPair<TopDUContextPointer, SimpleRange> import = importedContextForPosition(url, position);
  if(import.first)
    return import.second;

  return usedMacroForPosition(url, position).first;
}

QPair<KUrl, KDevelop::SimpleCursor> CppLanguageSupport::specialLanguageObjectJumpCursor(const KUrl& url, const SimpleCursor& position) {

  QPair<TopDUContextPointer, SimpleRange> import = importedContextForPosition(url, position);
    if(import.first) {
      DUChainReadLocker lock(DUChain::lock());
      if(import.first)
        return qMakePair(KUrl(import.first->url().str()), SimpleCursor(0,0));
    }

    QPair<SimpleRange, const rpp::pp_macro*> m = usedMacroForPosition(url, position);

    if(!m.first.isValid())
      return qMakePair(KUrl(), SimpleCursor::invalid());

    return qMakePair(KUrl(m.second->file.str()), SimpleCursor(m.second->sourceLine, 0));
}

QWidget* CppLanguageSupport::specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position) {

  QPair<TopDUContextPointer, SimpleRange> import = importedContextForPosition(url, position);
    if(import.first) {
      DUChainReadLocker lock(DUChain::lock());
      if(import.first) {
        //Prefer a standardContext, because the included one may have become empty due to
        if(import.first->localDeclarations().count() == 0 && import.first->childContexts().count() == 0) {

          KDevelop::TopDUContext* betterCtx = standardContext(KUrl(import.first->url().str()));

          if(betterCtx && (betterCtx->localDeclarations().count() != 0 || betterCtx->childContexts().count() != 0))
            return betterCtx->createNavigationWidget(0, 0, i18n("Emptied by preprocessor<br />"));
        }
        return import.first->createNavigationWidget();
      }
    }

    QPair<SimpleRange, const rpp::pp_macro*> m = usedMacroForPosition(url, position);
    if(!m.first.isValid())
      return 0;

    //Evaluate the preprocessed body
    QPair<QPair<QString, SimpleRange>, QString> found = cursorIdentifier(url, position);

    QString text = found.first.first;
    QString preprocessedBody;
    //Check whether tail contains arguments
    QString tail = found.second.trimmed(); ///@todo make this better.
    if(tail.startsWith("(")) {
      int i = findClose( tail, 0 );
      if(i != -1) {
        text += tail.left(i+1);
      }
    }

    {
      DUChainReadLocker lock(DUChain::lock());
      TopDUContext* ctx = standardContext(url, true);
      if(ctx) {
        Cpp::EnvironmentFile* p(dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data()));
        if(p) {
          kDebug() << "preprocessing" << text;
          preprocessedBody = Cpp::preprocess(text, p, position.line);
        }
      }
    }

    return new Cpp::NavigationWidget(*m.second, preprocessedBody);
}

bool CppLanguageSupport::needsUpdate(const Cpp::EnvironmentFilePointer& file, const KUrl& localPath, const KUrl::List& includePaths) const
{
  if(file->needsUpdate())
    return true;

  ///@todo somehow this check should also go into EnvironmentManager
  for( Cpp::ReferenceCountedStringSet::Iterator it = file->missingIncludeFiles().iterator(); it; ++it ) {

    ///@todo store IncludeLocal/IncludeGlobal somewhere
    QPair<KUrl, KUrl> included = findInclude( includePaths, localPath, (*it).str(), rpp::Preprocessor::IncludeLocal, KUrl(), true );
    if(!included.first.isEmpty())
      return true;
  }

  return false;
}

void CppLanguageSupport::newClassWizard()
{
  KUrl baseUrl;
  if(core()->documentController()->activeDocument())
    baseUrl = core()->documentController()->activeDocument()->url().upUrl();
  CppNewClass newClassWizard(qApp->activeWindow(), baseUrl);
  newClassWizard.exec();
}

UIBlockTester::UIBlockTesterThread::UIBlockTesterThread( UIBlockTester& parent ) : QThread(), m_parent( parent ), m_stop(false) {
}

 void UIBlockTester::UIBlockTesterThread::run() {
   while(!m_stop) {
           msleep( m_parent.m_msecs / 10 );
           m_parent.m_timeMutex.lock();
           QDateTime t = QDateTime::currentDateTime();
           uint msecs = m_parent.m_lastTime.time().msecsTo( t.time() );
           if( msecs > m_parent.m_msecs ) {
                   m_parent.lockup();
                   m_parent.m_lastTime = t;
           }
           m_parent.m_timeMutex.unlock();
  }
 }

 void UIBlockTester::UIBlockTesterThread::stop() {
         m_stop = true;
 }

 UIBlockTester::UIBlockTester( uint milliseconds ) : m_thread( *this ), m_msecs( milliseconds ) {
         m_timer = new QTimer( this );
         m_timer->start( milliseconds/10 );
         connect( m_timer, SIGNAL(timeout()), this, SLOT(timer()) );
         timer();
         m_thread.start();
 }
 UIBlockTester::~UIBlockTester() {
   m_thread.stop();
  m_thread.wait();
 }

 void UIBlockTester::timer() {
         m_timeMutex.lock();
         m_lastTime = QDateTime::currentDateTime();
         m_timeMutex.unlock();
#ifdef CALLGRIND_TRACE_UI_LOCKUP
         CALLGRIND_STOP_INSTRUMENTATION
#endif
 }

void UIBlockTester::lockup() {
        //std::cout << "UIBlockTester: lockup of the UI for " << m_msecs << endl; ///kdDebug(..) is not thread-safe..
#ifdef CALLGRIND_TRACE_UI_LOCKUP
    CALLGRIND_START_INSTRUMENTATION
#else
    kDebug() << "ui is blocking";
#endif
 }

#include "cpplanguagesupport.moc"
