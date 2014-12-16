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

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kio/netaccess.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <KDesktopFile>
#include <language/codecompletion/codecompletion.h>

#include <interfaces/icore.h>
#include <language/duchain/problem.h>
#include <interfaces/iproject.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isourceformattercontroller.h>
#include <interfaces/isourceformatter.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <language/interfaces/iquickopen.h>
#include <interfaces/iplugincontroller.h>
#include <language/interfaces/editorcontext.h>
#include <project/projectmodel.h>
#include <language/assistant/renameassistant.h>
#include <language/assistant/staticassistantsmanager.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/stringhelpers.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/functiondefinition.h>
#include <language/codegen/coderepresentation.h>

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
#include <interfaces/foregroundlock.h>
//#include "codegen/makeimplementationprivate.h"
#include "codegen/adaptsignatureassistant.h"
#include "codegen/unresolvedincludeassistant.h"

#include "includepathresolver.h"
#include "setuphelpers.h"
#include "quickopen.h"
#include "cppdebughelper.h"
#include "codegen/simplerefactoring.h"
#include "codegen/cppclasshelper.h"
#include "includepathcomputer.h"

//#include <valgrind/callgrind.h>


// #define CALLGRIND_TRACE_UI_LOCKUP

// defined through cmake
// #define DEBUG_UI_LOCKUP

#define LOCKUP_INTERVAL 300

#ifdef CALLGRIND_TRACE_UI_LOCKUP
#define DEBUG_UI_LOCKUP
#define LOCKUP_INTERVAL 5
#endif
#include "cpputils.h"

KTextEditor::Cursor normalizeCursor(KTextEditor::Cursor c) {
  c.setColumn(0);
  return c;
}


using namespace KDevelop;

CppLanguageSupport* CppLanguageSupport::m_self = 0;

namespace
{
void fillEditIncludeDirectoriesContextMenu(ContextMenuExtension& extension, KDevelop::Context* context)
{
    auto ec = dynamic_cast<KDevelop::EditorContext*>(context);
    if (ec && ec->currentLine().contains(QRegExp("^\\s*#include"))) {
        KDevelop::IAssistantAction::Ptr assistantAction;
        if (auto project = ICore::self()->projectController()->findProjectForUrl(ec->url())) {
            assistantAction.attach(new Cpp::OpenProjectConfigurationAction(project));
        } else {
            assistantAction.attach(new Cpp::AddCustomIncludePathAction(IndexedString(ec->url()), QString()));
        }
        auto action = assistantAction->toKAction();
        action->setText(i18n("Edit include directories"));
        extension.addAction(extension.ExtensionGroup, action);
    }
}
}

KDevelop::ContextMenuExtension CppLanguageSupport::contextMenuExtension(KDevelop::Context* context)
{
  ContextMenuExtension cm;
  EditorContext *ec = dynamic_cast<KDevelop::EditorContext *>(context);

  if (ec && ICore::self()->languageController()->languagesForUrl(ec->url()).contains(language())) {
    // It's a C++ file, let's add our context menu.
    m_refactoring->fillContextMenu(cm, context);
    fillEditIncludeDirectoriesContextMenu(cm, context);
  }
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

// For unit-tests that compile cpplanguagesupport.cpp into their executable
// don't create the factories as that means 2 instances of the factory
#ifndef BUILD_TESTS
K_PLUGIN_FACTORY(KDevCppSupportFactory, registerPlugin<CppLanguageSupport>(); )
K_EXPORT_PLUGIN(KDevCppSupportFactory(KAboutData("kdevcppsupport","kdevcpp", ki18n("C++ Support"), "0.1", ki18n("Support for C++ Language"), KAboutData::License_GPL)))
#else
class KDevCppSupportFactory : public KPluginFactory
{
public:
    static KComponentData componentData() { return KComponentData(); };
};
#endif


static QStringList mimeTypesList()
{
    KDesktopFile desktopFile("services", QString("kdevcppsupport.desktop"));
    const KConfigGroup& desktopGroup = desktopFile.desktopGroup();
    QString mimeTypesStr = desktopGroup.readEntry("X-KDevelop-SupportedMimeTypes", "");
    return mimeTypesStr.split(QChar(','), QString::SkipEmptyParts);
}

CppLanguageSupport::CppLanguageSupport( QObject* parent, const QVariantList& /*args*/ )
    : KDevelop::IPlugin( KDevCppSupportFactory::componentData(), parent ),
      KDevelop::ILanguageSupport(),
      m_mimeTypes(mimeTypesList())
{
    m_self = this;

    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    setXMLFile( "kdevcppsupport.rc" );

    m_highlights = new CppHighlighting( this );
    m_refactoring = new SimpleRefactoring(this);
    m_cc = new KDevelop::CodeCompletion( this, new Cpp::CodeCompletionModel(0), name() );

    Cpp::EnvironmentManager::init();
    Cpp::EnvironmentManager::self()->setSimplifiedMatching(true);
    Cpp::EnvironmentManager::self()->setMatchingLevel(Cpp::EnvironmentManager::Disabled);
//     Cpp::EnvironmentManager::self()->setMatchingLevel(Cpp::EnvironmentManager::Naive);
//     Cpp::EnvironmentManager::self()->setMatchingLevel(Cpp::EnvironmentManager::Full);

    CppUtils::standardMacros();

    m_quickOpenDataProvider = new IncludeFileDataProvider();

    IQuickOpen* quickOpen = core()->pluginController()->extensionForPlugin<IQuickOpen>("org.kdevelop.IQuickOpen");

    if( quickOpen )
        quickOpen->registerProvider( IncludeFileDataProvider::scopes(), QStringList(i18n("Files")), m_quickOpenDataProvider );
    // else we are in NoUi mode (duchainify, unit tests, ...) and hence cannot find the Quickopen plugin

#ifdef DEBUG_UI_LOCKUP
    new UIBlockTester(LOCKUP_INTERVAL, this);
#endif

    core()->languageController()->staticAssistantsManager()->registerAssistant(StaticAssistant::Ptr(new RenameAssistant(this)));
    core()->languageController()->staticAssistantsManager()->registerAssistant(StaticAssistant::Ptr(new Cpp::AdaptSignatureAssistant(this)));

    foreach(QString mimeType, m_mimeTypes){
        KDevelop::IBuddyDocumentFinder::addFinder(mimeType,this);
    }
}

void CppLanguageSupport::createActionsForMainWindow (Sublime::MainWindow* /*window*/, QString& _xmlFile, KActionCollection& actions)
{
    _xmlFile = xmlFile();

    KAction* switchDefinitionDeclaration = actions.addAction("switch_definition_declaration");
    switchDefinitionDeclaration->setText( i18n("&Switch Definition/Declaration") );
    switchDefinitionDeclaration->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_C );
    connect(switchDefinitionDeclaration, SIGNAL(triggered(bool)), this, SLOT(switchDefinitionDeclaration()));

//    KAction* pimplAction = actions->addAction("code_private_implementation");
//    pimplAction->setText( i18n("Make Class Implementation Private") );
//    pimplAction->setShortcut(Qt::ALT | Qt::META | Qt::Key_P);
//    connect(pimplAction, SIGNAL(triggered(bool)), &SimpleRefactoring::self(), SLOT(executePrivateImplementationAction()));

    KAction* renameDeclarationAction = actions.addAction("code_rename_declaration");
    renameDeclarationAction->setText( i18n("Rename Declaration") );
    renameDeclarationAction->setIcon(KIcon("edit-rename"));
    renameDeclarationAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_R);
    connect(renameDeclarationAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeRenameAction()));

    KAction* moveIntoSourceAction = actions.addAction("code_move_definition");
    moveIntoSourceAction->setText( i18n("Move into Source") );
    moveIntoSourceAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_S);
    connect(moveIntoSourceAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeMoveIntoSourceAction()));
}

void CppLanguageSupport::switchDefinitionDeclaration()
{
  kDebug(9007) << "switching definition/declaration";

  KUrl docUrl;
  SimpleCursor cursor;
  
  ///Step 1: Find the current top-level context of type DUContext::Other(the highest code-context).
  ///-- If it belongs to a function-declaration or definition, it can be retrieved through owner(), and we are in a definition.
  ///-- If no such context could be found, search for a declaration on the same line as the cursor, and switch to the according definition
  
  {
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    if(!doc || !doc->textDocument() || !doc->textDocument()->activeView()) {
      kDebug(9007) << "No active document";
      return;
    }
    
    docUrl = doc->textDocument()->url();
    cursor = SimpleCursor(doc->textDocument()->activeView()->cursorPosition()); 
  }
  
  const QString switchCandidate = CppUtils::sourceOrHeaderCandidate(docUrl.toLocalFile());
  
  if(!switchCandidate.isEmpty())
  {
    
    DUChainReadLocker lock;

    //If the file has not been parsed yet, update it
    TopDUContext* ctx = standardContext(docUrl);
    //At least 'VisibleDeclarationsAndContexts' is required so we can do a switch
    if(!ctx || (ctx->parsingEnvironmentFile() && !ctx->parsingEnvironmentFile()->featuresSatisfied(TopDUContext::VisibleDeclarationsAndContexts)))
    {
      lock.unlock();
      kDebug(9007) << "Parsing switch-candidate before switching" << switchCandidate;
      ReferencedTopDUContext updatedContext = DUChain::self()->waitForUpdate(IndexedString(switchCandidate), TopDUContext::VisibleDeclarationsAndContexts);
      if (!updatedContext) {
        kDebug(9007) << "Failed to update document:" << switchCandidate;
        return;
      }
    }
  }
  
  kDebug(9007) << "Document:" << docUrl;

  DUChainReadLocker lock(DUChain::lock());

  TopDUContext* standardCtx = standardContext(docUrl);

  bool wasSignal = false;
  if(standardCtx) {
    Declaration* definition = 0;

    DUContext* ctx = standardCtx->findContext(standardCtx->transformToLocalRevision(cursor));
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

    if(ClassFunctionDeclaration* cDef = dynamic_cast<ClassFunctionDeclaration*>(definition)) {
      if (cDef->isSignal()) {
        kDebug() << "found definition is a signal, not switching to .moc implementation";
        definition = 0;
        wasSignal = true;
      }
    }

    FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(definition);
    if(def && def->declaration()) {
      Declaration* declaration = def->declaration();
      KTextEditor::Range targetRange = declaration->rangeInCurrentRevision().textRange();
      KUrl url(declaration->url().str());
      kDebug() << "found definition that has declaration: " << definition->toString() << "range" << targetRange << "url" << url;
      lock.unlock();

      KDevelop::IDocument* document = core()->documentController()->documentForUrl(url);
      
      if(!document || 
          (document && document->textDocument() && document->textDocument()->activeView() && !targetRange.contains(document->textDocument()->activeView()->cursorPosition()))) {
        KTextEditor::Cursor pos(normalizeCursor(targetRange.start()));
        core()->documentController()->openDocument(url, KTextEditor::Range(pos, pos));
      }else if(document)
        core()->documentController()->openDocument(url);
      return;
    }else{
      kDebug(9007) << "Definition has no assigned declaration";
    }

    kDebug(9007) << "Could not get definition/declaration from context";
  }else{
    kDebug(9007) << "Got no context for the current document";
  }

  Declaration* def = 0;
  if (!wasSignal) {
     def = definitionForCursorDeclaration(cursor, docUrl);
  }

  if(def) {
    KUrl url(def->url().str());
    KTextEditor::Range targetRange = def->rangeInCurrentRevision().textRange();

    if(def->internalContext()) {
      targetRange.end() = def->internalContext()->rangeInCurrentRevision().end.textCursor();
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
  }else if (!wasSignal) {
    kWarning(9007) << "Found no definition assigned to cursor position";
  }

  lock.unlock();
  ///- If no definition/declaration could be found to switch to, just switch the document using normal header/source heuristic by file-extension

  if(!switchCandidate.isEmpty()) {
    core()->documentController()->openDocument(KUrl(switchCandidate));
  }else{
    kDebug(9007) << "Found no source/header candidate to switch";
  }
}

CppLanguageSupport::~CppLanguageSupport()
{
    ILanguage* lang = language();
    if (lang) {
        TemporarilyReleaseForegroundLock release;
        lang->parseLock()->lockForWrite();
        m_self = 0; //By locking the parse-mutexes, we make sure that parse- and preprocess-jobs get a chance to finish in a good state
        lang->parseLock()->unlock();
    }

    delete m_quickOpenDataProvider;

    // Remove any documents waiting to be parsed from the background parser.
    core()->languageController()->backgroundParser()->clear(this);

#ifdef DEBUG_UI_LOCKUP
    delete m_blockTester;
#endif

    foreach(QString mimeType, m_mimeTypes){
        KDevelop::IBuddyDocumentFinder::removeFinder(mimeType);
    }
}

CppLanguageSupport* CppLanguageSupport::self() {
    return m_self;
}

KDevelop::ParseJob *CppLanguageSupport::createParseJob( const IndexedString &url )
{
    return new CPPParseJob( url, this );
}

KDevelop::ICodeHighlighting *CppLanguageSupport::codeHighlighting() const
{
    return m_highlights;
}

BasicRefactoring* CppLanguageSupport::refactoring() const
{
    return m_refactoring;
}

ICreateClassHelper* CppLanguageSupport::createClassHelper() const
{
    return new CppClassHelper;
}


void CppLanguageSupport::findIncludePathsForJob(CPPParseJob* job)
{
  IncludePathComputer* comp = new IncludePathComputer(job->document().str());
  comp->computeForeground();
  job->gotIncludePaths(comp);
}

QString CppLanguageSupport::name() const
{
    return "C++";
}

KDevelop::ILanguage *CppLanguageSupport::language()
{
    return core()->languageController()->language(name());
}

TopDUContext* CppLanguageSupport::standardContext(const KUrl& url, bool proxyContext)
{
  DUChainReadLocker lock(DUChain::lock());
  const ParsingEnvironment* env = PreprocessJob::standardEnvironment();
  KDevelop::TopDUContext* top;
  top = KDevelop::DUChain::self()->chainForDocument(url, env, Cpp::EnvironmentManager::self()->isSimplifiedMatching() || proxyContext);

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

  if(top && (top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->isProxyContext()) && !proxyContext)
  {
    top = DUChainUtils::contentContextFromProxyContext(top);
    if(!top)
    {
      kDebug(9007) << "WARNING: Proxy-context had invalid content-context";
    }
  }

  return top;
}

/**
 * Anonymous namespace for IBuddyDocumentFinder related functions.
 */
namespace {

/**
 * @returns all extensions which match the given @p mimeType.
 */
QSet<QString> getExtensionsByMimeType(QString mimeType)
{
    KMimeType::Ptr ptr = KMimeType::mimeType(mimeType);

    if (!ptr) {
      return QSet<QString>();
    }

    QSet<QString> extensions;
    foreach(const QString& pattern, ptr->patterns()) {
      if (pattern.startsWith("*.")) {
        extensions << pattern.mid(2);
      }
    }

    return extensions;
}

QSet<QString> getHeaderFileExtensions()
{
    return getExtensionsByMimeType("text/x-c++hdr") | getExtensionsByMimeType("text/x-chdr");
}

QSet<QString> getSourceFileExtensions()
{
  return getExtensionsByMimeType("text/x-c++src") | getExtensionsByMimeType("text/x-csrc");
}

enum FileType {
  Unknown, ///< Doesn't belong to C++
  Header,  ///< Is a header file
  Source   ///< Is a C(++) file
};

/**
 * Generates the base path (without extension) and the file type
 * for the specified url.
 *
 * @returns pair of base path and file type which has been found for @p url.
 */
QPair<QString,FileType> basePathAndType(const KUrl& url)
{
    QString path = url.toLocalFile();
    int idxSlash = path.lastIndexOf("/");
    int idxDot = path.lastIndexOf(".");
    FileType fileType = Unknown;
    QString basePath;
    if (idxSlash >= 0 && idxDot >= 0 && idxDot > idxSlash) {
        basePath = path.left(idxDot);
        if (idxDot + 1 < path.length()) {
            QString extension = path.mid(idxDot + 1);
            if (getHeaderFileExtensions().contains(extension)) {
                fileType = Header;
            } else if (getSourceFileExtensions().contains(extension)) {
                fileType = Source;
            }
        }
    } else {
        basePath = path;
    }

    return qMakePair(basePath, fileType);
}

}


/**
 * Behavior: Considers the URLs as buddy documents if the base path (=without extension)
 * is the same, and one extension starts with h/H and the other one with c/C.
 * For example, foo.hpp and foo.C are buddies.
 */
bool CppLanguageSupport::areBuddies(const KUrl& url1, const KUrl& url2)
{
    QPair<QString, FileType> type1 = basePathAndType(url1);
    QPair<QString, FileType> type2 = basePathAndType(url2);
    return(type1.first == type2.first && ((type1.second == Header && type2.second == Source) ||
                                          (type1.second == Source && type2.second == Header)));
}

/**
 * Behavior: places foo.h* / foo.H* left of foo.c* / foo.C*
 */
bool CppLanguageSupport::buddyOrder(const KUrl& url1, const KUrl& url2)
{
    QPair<QString, FileType> type1 = basePathAndType(url1);
    QPair<QString, FileType> type2 = basePathAndType(url2);
    // Precondition is that the two URLs are buddies, so don't check it
    return(type1.second == Header && type2.second == Source);
}

QVector< KUrl > CppLanguageSupport::getPotentialBuddies(const KUrl& url) const
{
    QPair<QString, FileType> type = basePathAndType(url);
    // Don't do anything for types we don't know
    if (type.second == Unknown) {
      return QVector< KUrl >();
    }

    // Depending on the buddy's file type we either generate source extensions (for headers)
    // or header extensions (for sources)
    const QSet<QString>& extensions = ( type.second == Header ? getSourceFileExtensions() : getHeaderFileExtensions() );
    QVector< KUrl > buddies;
    foreach(const QString& extension, extensions) {
      buddies.append(KUrl(type.first + "." + extension));
    }

    return buddies;
}

QPair<QPair<QString, SimpleRange>, QString> CppLanguageSupport::cursorIdentifier(const KUrl& url, const SimpleCursor& position) const {
  KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
  if(!doc || !doc->textDocument() || !doc->textDocument()->activeView())
    return qMakePair(qMakePair(QString(), SimpleRange::invalid()), QString());

  int lineNumber = position.line;
  int lineLength = doc->textDocument()->lineLength(lineNumber);

  QString line = doc->textDocument()->text(KTextEditor::Range(lineNumber, 0, lineNumber, lineLength));

  if(CppUtils::findEndOfInclude(line) != -1) { //If it is an include, return the complete line
    int start = 0;
    while(start < lineLength && line[start] == ' ')
      ++start;

    return qMakePair( qMakePair(line, SimpleRange(lineNumber, start, lineNumber, lineLength)), QString() );
  }

  // not an include, if at all a Makro, hence clear strings
  line = clearStrings(line);

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

  int pos = 0;
  for(; pos < word.size(); ++pos) {
    if(word[pos] == '"' || word[pos] == '<') {
      wordRange.start.column = ++pos;
      break;
    }
  }

  for(; pos < word.size(); ++pos) {
    if(word[pos] == '"' || word[pos] == '>') {
      wordRange.end.column = pos;
      break;
    }
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

  if(CppUtils::findEndOfInclude(word) != -1) {
    //It's an #include, find out which file was included at the given line
    foreach(const DUContext::Import &imported, ctx->importedParentContexts()) {
      if(imported.context(0)) {
        if(ctx->transformFromLocalRevision(ctx->importPosition(imported.context(0))).line == wordRange.start.line) {
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
    if(m.second->function_like) {
     if(tail.endsWith('\\'))
       tail.truncate(tail.length() - 1);
      //properly support macro expansions when arguments contain newlines
      int foundClosingBrace = findClose( tail, 0 );
      KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
      if(doc && doc->textDocument() && doc->textDocument()->activeView() && foundClosingBrace < 0) {
        const int lines = doc->textDocument()->lines();
        for (int lineNumber = position.line + 1; foundClosingBrace < 0 && lineNumber < lines; lineNumber++) {
          tail += doc->textDocument()->line(lineNumber).trimmed();
          if(tail.endsWith('\\'))
            tail.truncate(tail.length() - 1);
          foundClosingBrace = findClose( tail, 0 );
        }
      }
      text += tail.left(foundClosingBrace + 1);
    }

    {
      DUChainReadLocker lock(DUChain::lock());
      TopDUContext* ctx = standardContext(url, true);
      if(ctx) {
        Cpp::EnvironmentFile* p(dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data()));
        if(p) {
          kDebug() << "preprocessing" << text;
          preprocessedBody = Cpp::preprocess(text, p, position.line+1);
        }
      }
    }

    KMimeType::Ptr mime = KMimeType::mimeType("text/x-c++hdr");
    ISourceFormatter* i = core()->sourceFormatterController()->formatterForMimeType(mime);
    if(i){
      SourceFormatterStyle style = core()->sourceFormatterController()->styleForMimeType(mime);
      preprocessedBody = i->formatSourceWithStyle(style, preprocessedBody, KUrl(), mime);
    }

    return new Cpp::NavigationWidget(*m.second, preprocessedBody);
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

UIBlockTester::UIBlockTester( uint milliseconds, QObject* parent )
  : QObject(parent)
  , m_thread( *this )
  , m_msecs( milliseconds )
{
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
