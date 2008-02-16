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
#include <kactioncollection.h>
#include <kaction.h>
#include <QExtensionFactory>
#include <QtDesigner/QExtensionFactory>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kio/netaccess.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/smartinterface.h>

#include <icore.h>
#include <iproblem.h>
#include <iproject.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <ilanguage.h>
#include <ilanguagecontroller.h>
#include <iprojectcontroller.h>
#include <ibuildsystemmanager.h>
#include <iquickopen.h>
#include <iplugincontroller.h>
#include <projectmodel.h>
#include <backgroundparser.h>
#include <duchain.h>
#include <duchainlock.h>
#include <topducontext.h>
#include <smartconverter.h>
#include <symboltable.h>

#include "preprocessjob.h"
#include "rpp/preprocessor.h"
#include "ast.h"
#include "parsesession.h"
#include "cpphighlighting.h"
#include "cppcodecompletion.h"
#include "cppeditorintegrator.h"
#include "usebuilder.h"
#include "typerepository.h"
#include "cppparsejob.h"
#include "environmentmanager.h"
#include "macroset.h"

#include "includepathresolver.h"
#include "setuphelpers.h"
#include "quickopen.h"

//List of possible headers used for definition/declaration fallback switching
QStringList headerExtensions(QString("h,H,hh,hxx,hpp,tlh,h++").split(','));
QStringList sourceExtensions(QString("c,cc,cpp,c++,cxx,C,m,mm,M,inl,_impl.h").split(','));

QString addDot(QString ext) {
  if(ext.contains('.')) //We need this check because of the _impl.h thing
    return ext;
  else
    return "." + ext;
}

//#define DEBUG

//When this is enabled, the include-path-resolver will always be issued,
//and the returned include-path compared to the one returned by the build-manager.
//Set it to 1 to debug build-managers.
#define DEBUG_INCLUDE_PATHS 1

using namespace KDevelop;

CppLanguageSupport* CppLanguageSupport::m_self = 0;

QList<KUrl> convertToUrls(const QList<HashedString>& stringList) {
  QList<KUrl> ret;
  foreach(const HashedString& str, stringList)
    ret << KUrl(str.str());
  return ret;
}

Declaration* declarationInLine(const KDevelop::SimpleCursor& cursor, DUContext* ctx) {
  foreach(Declaration* decl, ctx->localDeclarations())
    if(decl->range().start.line == cursor.line)
      return decl;
  
  foreach(DUContext* child, ctx->childContexts()){
    Declaration* decl = declarationInLine(cursor, child);
    if(decl)
      return decl;
  }

  return 0;
}

///Tries to find a definition for the given cursor-position and document-url. DUChain must be locked.
Definition* definitionForCursor(const KDevelop::SimpleCursor& cursor, const KUrl& url) {
  QList<TopDUContext*> topContexts = DUChain::self()->chainsForDocument( url );
  foreach(TopDUContext* ctx, topContexts) {
    Declaration* decl = declarationInLine(cursor, ctx);
    if(decl && decl->definition())
      return decl->definition();
  }
  return 0;
}


K_PLUGIN_FACTORY(KDevCppSupportFactory, registerPlugin<CppLanguageSupport>(); )
K_EXPORT_PLUGIN(KDevCppSupportFactory("kdevcppsupport"))

CppLanguageSupport::CppLanguageSupport( QObject* parent, const QVariantList& /*args*/ )
    : KDevelop::IPlugin( KDevCppSupportFactory::componentData(), parent ),
      KDevelop::ILanguageSupport(),
      m_standardMacros(0)
{
    m_self = this;

    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    setXMLFile( "kdevcppsupport.rc" );

    m_highlights = new CppHighlighting( this );
    m_cc = new CppCodeCompletion( this );
    m_standardMacros = new Cpp::MacroRepository::LazySet( &Cpp::EnvironmentManager::m_macroRepository, &Cpp::EnvironmentManager::m_repositoryMutex );
    m_standardIncludePaths = new QStringList;
    m_environmentManager = new Cpp::EnvironmentManager;
    m_environmentManager->setSimplifiedMatching(true); ///@todo Make simplified matching optional. Before that, make it work.
    {
        DUChainWriteLocker l(DUChain::lock());
        DUChain::self()->addParsingEnvironmentManager(m_environmentManager);
    }

    m_includeResolver = new CppTools::IncludePathResolver;
    // Retrieve the standard include paths & macro definitions for this machine.
    // Uses gcc commands to retrieve the information.
    CppTools::setupStandardIncludePaths(*m_standardIncludePaths);
    CppTools::setupStandardMacros(*m_standardMacros);

    connect( core()->documentController(),
             SIGNAL( documentLoaded( KDevelop::IDocument* ) ),
             this, SLOT( documentLoaded( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentClosed( KDevelop::IDocument* ) ),
             this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentStateChanged( KDevelop::IDocument* ) ),
             this, SLOT( documentChanged( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentContentChanged( KDevelop::IDocument* ) ),
             this, SLOT( documentChanged( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentActivated( KDevelop::IDocument* ) ),
             this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
    connect( core()->projectController(),
             SIGNAL( projectOpened( KDevelop::IProject* ) ),
             this, SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( core()->projectController(),
             SIGNAL( projectClosing( KDevelop::IProject* ) ),
             this, SLOT( projectClosing( KDevelop::IProject* ) ) );

    // Initialise singletons, to prevent needing a mutex in their self() methods
    TypeRepository::self();
    SymbolTable::self();

    m_quickOpenDataProvider = new IncludeFileDataProvider();

    IQuickOpen* quickOpen = core()->pluginController()->extensionForPlugin<IQuickOpen>("org.kdevelop.IQuickOpen");

    if( quickOpen )
        quickOpen->registerProvider( IncludeFileDataProvider::scopes(), QStringList(i18n("Files")), m_quickOpenDataProvider );
    else
        kWarning() << "Quickopen not found";

    KActionCollection* actions = actionCollection();

    QAction* switchDefinitionDeclaration = actions->addAction("switch_definition_declaration");
    switchDefinitionDeclaration->setText( i18n("&Switch Definition/Declaration") );
    switchDefinitionDeclaration->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_C );
    connect(switchDefinitionDeclaration, SIGNAL(triggered(bool)), this, SLOT(switchDefinitionDeclaration()));
}

KUrl CppLanguageSupport::sourceOrHeaderCandidate( const KUrl &url )
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
    foreach(QString ext, sourceExtensions)
      candidates << ( base + addDot(ext) );
    
    possibleExts = sourceExtensions;
  }
  // if file is an implementation file, search for header file
  else if ( sourceExtensions.contains( ext ) )
  {
    foreach(QString ext, headerExtensions)
      candidates << ( base + addDot(ext) );
    
    possibleExts = headerExtensions;
  }
  // search for files from the assembled candidate lists, return the first
  // candidate file that actually exists or QString::null if nothing is found.
  QStringList::ConstIterator it;
  for ( it = candidates.begin(); it != candidates.end(); ++it )
  {
    //kDebug( 9007 ) << "Trying " << ( *it ) << endl;
    if ( QFileInfo( *it ).exists() )
    {
      kDebug( 9007 ) << "using: " << *it << endl;
      return * it;
    }
  }
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
  foreach ( KUrl url, projectFileList )
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
    DUContext* ctx = standardCtx->findContext(SimpleCursor(doc->textDocument()->activeView()->cursorPosition()));
    while(ctx && ctx->parentContext() && ctx->parentContext()->type() == DUContext::Other)
      ctx = ctx->parentContext();

    if(ctx && ctx->owner() && ctx->type() == DUContext::Other) {
      if(ctx->owner()->asDefinition()) {
        Declaration* decl = ctx->owner()->asDefinition()->declaration();

        if(decl) {
          KTextEditor::Cursor c = decl->range().textRange().start();
          lock.unlock();
          core()->documentController()->openDocument(KUrl(decl->url().str()), c);
          return;
        }else{
          kDebug(9007) << "Definition has no assigned declaration";
        }
      }else if(ctx->owner()->asDeclaration()) {
        kDebug(9007) << "In internal context of declaration+definition, nowhere to switch";
        return;
      }
    }
    kDebug(9007) << "Could not get definition/declaration from context";
  }else{
    kDebug(9007) << "Got no context for the current document";
  }

  Definition* def = definitionForCursor(SimpleCursor(doc->textDocument()->activeView()->cursorPosition()), doc->url());

  if(def) {
    ///@todo If the cursor is already in the target context, do not move it.
    if(def->internalContext()) {
      KTextEditor::Cursor c = def->internalContext()->range().textRange().start();
      lock.unlock();
      core()->documentController()->openDocument(KUrl(def->url().str()), c);
    }else{
      kWarning(9007) << "Declaration does not have internal context";
      KTextEditor::Cursor c = def->range().textRange().start();
      lock.unlock();
      core()->documentController()->openDocument(KUrl(def->url().str()), c);
    }
    return;
  }else{
    kDebug(9007) << "Found no definition assigned to cursor position";
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
    m_self = 0;

    delete m_quickOpenDataProvider;
    
    // Remove any documents waiting to be parsed from the background paser.
    core()->languageController()->backgroundParser()->clear(this);

    // Remove the corresponding parsing environment from the DUChain.
    {
        DUChainWriteLocker l(DUChain::lock());
        DUChain::self()->removeParsingEnvironmentManager(m_environmentManager);
    }

    delete m_standardMacros;
    delete m_standardIncludePaths;
    delete m_environmentManager;
    delete m_includeResolver;
}

const Cpp::MacroRepository::LazySet& CppLanguageSupport::standardMacros() const {
    return *m_standardMacros;
}

CppLanguageSupport* CppLanguageSupport::self() {
    return m_self;
}

void CppLanguageSupport::documentChanged( KDevelop::IDocument* document )
{
    core()->languageController()->backgroundParser()->addDocument(document->url());
}

KDevelop::ParseJob *CppLanguageSupport::createParseJob( const KUrl &url )
{
    return new CPPParseJob( url, this );
}

void CppLanguageSupport::documentLoaded(KDevelop::IDocument* doc)
{
    kDebug( 9007 ) << "CppLanguageSupport::documentLoaded";

    EditorIntegrator editor;
    editor.setCurrentUrl(doc->url().prettyUrl());

    QList<TopDUContext*> chains = DUChain::self()->chainsForDocument(doc->url());

    foreach (TopDUContext* chain, chains) {
      if ( codeHighlighting() && editor.smart() )
      {
          QMutexLocker lock(editor.smart()->smartMutex());
          codeHighlighting()->highlightDUChain( chain );
      }
    }
    if( chains.isEmpty() )
        core()->languageController()->backgroundParser()->addDocument(doc->url());
}

void CppLanguageSupport::documentClosed(KDevelop::IDocument *)
{
    kDebug( 9007 ) << "CppLanguageSupport::documentClosed";
}

KDevelop::ICodeHighlighting *CppLanguageSupport::codeHighlighting() const
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
}

void CppLanguageSupport::projectClosing(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    //TODO: Anything to do here?!?!
}

KUrl::List CppLanguageSupport::findIncludePaths(const KUrl& source, QList<KDevelop::ProblemPointer>* problems) const
{
  KUrl::List allPaths;
  QSet<KUrl> hasPath;

  if( source.isEmpty() ) {
    foreach( QString path, *m_standardIncludePaths) {
        KUrl u(path);
        if(!hasPath.contains(u))
          allPaths << KUrl(path);
    }
    return allPaths;
  }

    KUrl effectiveBuildDirectory;
    KUrl buildDirectory;
    KUrl projectDirectory;

    bool gotPathsFromManager = false;
    
    foreach (KDevelop::IProject *project, core()->projectController()->projects()) {
        QList<KDevelop::ProjectFileItem*> files = project->filesForUrl(source);
        ProjectFileItem* file = 0;
        if( !files.isEmpty() )
            file = files.first();
        if (!file) {
                kDebug() << "Didn't find file for url:" << source << "in project" << project->name();
            continue;
        }

        KDevelop::IBuildSystemManager* buildManager = project->buildSystemManager();
        if (!buildManager) {
                kDebug() << "didn't get build manager for project:" << project->name();
            // We found the project, but no build manager!!
            continue;
        }

        
        projectDirectory = project->folder();
        effectiveBuildDirectory = buildDirectory = buildManager->buildDirectory(project->projectItem());
        kDebug(9007) << "Got build-directory from project manager:" << effectiveBuildDirectory;

        if(projectDirectory == effectiveBuildDirectory)
            projectDirectory = effectiveBuildDirectory = KUrl();
        
        KUrl::List dirs = buildManager->includeDirectories(file);

        gotPathsFromManager = true;
        
        kDebug(9007) << "Got " << dirs.count() << " include-paths from build-manager";

        foreach( KUrl dir, dirs ) {
            dir.adjustPath(KUrl::AddTrailingSlash);
            if(!hasPath.contains(dir))
              allPaths << dir;
            hasPath.insert(dir);
        }
    }

    if(!gotPathsFromManager)
      kDebug(9007) << "Did not find a build-manager for" << source;
    
    KDevelop::Problem problem;
    
    if( allPaths.isEmpty() || DEBUG_INCLUDE_PATHS ) {
        //Fallback-search using include-path resolver

        if(!effectiveBuildDirectory.isEmpty()) {
            ///@todo remote directories?
            m_includeResolver->setOutOfSourceBuildSystem(projectDirectory.path(), effectiveBuildDirectory.path());
        } else {
            m_includeResolver->resetOutOfSourceBuild();
        }
        CppTools::PathResolutionResult result = m_includeResolver->resolveIncludePath(source.path());
        if (result) {
            bool hadMissingPath = false;
            if( !gotPathsFromManager ) {
                foreach( QString res, result.paths ) {
                    KUrl r(res);
                    r.adjustPath(KUrl::AddTrailingSlash);
                    if(!hasPath.contains(r))
                      allPaths << r;
                    hasPath.insert(r);
                }
            } else {
                //Compare the includes found by the includepathresolver to the ones returned by the project-manager, and complain eaach missing path.
                foreach( QString res, result.paths ) {
                    
                    KUrl r(res);
                    r.adjustPath(KUrl::AddTrailingSlash);
                    
                    KUrl r2(res);
                    r2.adjustPath(KUrl::RemoveTrailingSlash);
                    
                    if( !hasPath.contains(r) && !hasPath.contains(r2) ) {
                        hadMissingPath = true;
                        if(!hasPath.contains(r))
                          allPaths << r;
                        hasPath.insert(r);
                        
                        kDebug(9007) << "Include-path was missing in list returned by build-manager, adding it: " << r.prettyUrl();

                        if( problems ) {
                          KDevelop::Problem p;
                          p.setSource(KDevelop::Problem::Preprocessor);
                          p.setDescription(i18n("Build-manager did not return an include-path" ));
                          p.setExplanation(i18n("The build-manager did not return the include-path %1, which could be resolved by the include-path resolver", r.prettyUrl()));
                          p.setFinalLocation(DocumentRange(source.prettyUrl(), KTextEditor::Cursor(0,0), KTextEditor::Cursor(0,0)));
                          *problems << KDevelop::ProblemPointer( new KDevelop::Problem(p) );
                        }
                    }
                }
                
                if( hadMissingPath ) {
                    QString paths;
                    foreach( const KUrl& u, allPaths )
                        paths += u.prettyUrl() + "\n";
                    
                    kDebug(9007) << "Total list of include-paths:\n" << paths << "\nEnd of list";
                }
            }
        }else{
            kDebug(9007) << "Failed to resolve include-path for \"" << source << "\":" << result.errorMessage << "\n" << result.longErrorMessage << "\n";
            problem.setSource(KDevelop::Problem::Preprocessor);
            problem.setDescription(i18n("Include-path resolver:") + " " + result.errorMessage);
            problem.setExplanation(i18n("Used build directory: \"%1\"\nInclude-path resolver: %2", effectiveBuildDirectory.prettyUrl(), result.longErrorMessage));
            problem.setFinalLocation(DocumentRange(source.prettyUrl(), KTextEditor::Cursor(0,0), KTextEditor::Cursor(0,0)));
        }
    }

    if( allPaths.isEmpty() && problem.source() != KDevelop::Problem::Unknown && problems )
      *problems << KDevelop::ProblemPointer( new KDevelop::Problem(problem) );
    
    if( allPaths.isEmpty() ) {
        ///Last chance: Take a parsed version of the file from the du-chain, and get its include-paths(We will then get the include-path that some time was used to parse the file)
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        TopDUContext* ctx = KDevelop::DUChain::self()->chainForDocument(source);
        if( ctx && ctx->parsingEnvironmentFile() ) {
            Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data());
            Q_ASSERT(envFile);
            allPaths = convertToUrls(envFile->includePaths());
            kDebug(9007) << "Took include-path for" << source << "from a random parsed duchain-version of it";
            foreach(KUrl url, allPaths)
              hasPath.insert(url);
        }
    }

    //Insert the standard-paths at the end
    foreach( QString path, *m_standardIncludePaths) {
      KUrl u(path);
      if(!hasPath.contains(u))
        allPaths << KUrl(path);
      hasPath.insert(u);
    }


    return allPaths;
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
        KUrl searchPath = path;
        searchPath.addPath(addPath);
        QDirIterator dirContent(searchPath.path());

        while(dirContent.hasNext()) {
            QString next = dirContent.next();
            KUrl u(next);
            if(u.fileName() == ".." || u.fileName() == ".")
                continue;

            Cpp::IncludeItem item;
            item.name = u.fileName();
            item.isDirectory = QFileInfo(u.path()).isDir();
            u.setFileName(QString());
            if(item.isDirectory)
                item.name += '/'; //We rely on having a trailing slash behind directories
            item.basePath = u;
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
        QFileInfo info(QDir(localPath.path()), includeName);
        if (info.exists() && info.isReadable()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.absoluteFilePath());
            ret.second = localPath;
            return ret;
        }
    }

    //When a path is skipped, we will start searching exactly after that path
    bool needSkip = !skipPath.isEmpty();

restart:
    foreach( KUrl path, includePaths ) {
        if( needSkip ) {
            if( path == skipPath ) {
                needSkip = false;
                continue;
            }
        }

        QFileInfo info(QDir( path.path() ), includeName);

        if (info.exists() && info.isReadable()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.absoluteFilePath());
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
        foreach( KUrl path, includePaths )
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

Cpp::EnvironmentManager* CppLanguageSupport::environmentManager() const {
    return m_environmentManager;
}

void CppLanguageSupport::documentActivated(KDevelop::IDocument * document)
{
  Q_UNUSED(document)
}

TopDUContext* CppLanguageSupport::standardContext(const KUrl& url, bool allowProxyContext)
{
  DUChainReadLocker lock(DUChain::lock());
  ParsingEnvironment* env = PreprocessJob::createStandardEnvironment();
  KDevelop::TopDUContext* top = KDevelop::DUChain::self()->chainForDocument(url, env);
  delete env;

  if( !top ) {
    kDebug(9007) << "Could not find perfectly matching version of " << url << " for completion";
    top = DUChain::self()->chainForDocument(url);
  }

  if(top && (top->flags() & TopDUContext::ProxyContextFlag) && !allowProxyContext)
  {
    if(!top->importedParentContexts().isEmpty())
    {
      top = dynamic_cast<TopDUContext*>(top->importedParentContexts().first().data());

      if(!top)
        kDebug(9007) << "WARNING: Proxy-context had invalid content-context";

    } else {
      kDebug(9007) << "ERROR: Proxy-context has no content-context";
    }
  }

  return top;
}


#include "cpplanguagesupport.moc"

