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

#include <QSet>
#include <QAction>
#include <QTimer>
#include <QMimeDatabase>
#include <QReadWriteLock>
#include <kactioncollection.h>
#include <kaction.h>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <KDesktopFile>
#include <language/codecompletion/codecompletion.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
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

#include <interfaces/contextmenuextension.h>
#include <util/foregroundlock.h>

#include "preprocessjob.h"
#include "rpp/preprocessor.h"
#include "ast.h"
#include "parsesession.h"
#include "cpphighlighting.h"
#include "cppparsejob.h"
#include "codecompletion/model.h"
#include "environmentmanager.h"
#include "cppduchain/navigation/navigationwidget.h"
#include "cppduchain/cppduchain.h"
//#include "codegen/makeimplementationprivate.h"
#include "codegen/adaptsignatureassistant.h"
#include "codegen/unresolvedincludeassistant.h"

#include "quickopen.h"
#include "cppdebughelper.h"
#include "codegen/simplerefactoring.h"
// #include "codegen/cppclasshelper.h"
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

using namespace KDevelop;

CppLanguageSupport* CppLanguageSupport::m_self = 0;

namespace
{
void fillEditIncludeDirectoriesContextMenu(ContextMenuExtension& extension, KDevelop::Context* context)
{
    auto ec = dynamic_cast<KDevelop::EditorContext*>(context);
    if (ec && ec->currentLine().contains(QRegExp("^\\s*#include"))) {
        KDevelop::IAssistantAction::Ptr assistantAction(new Cpp::AddCustomIncludePathAction(IndexedString(ec->url())));
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
Declaration* definitionForCursorDeclaration(const KTextEditor::Cursor& cursor, const KUrl& url) {
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
// K_EXPORT_PLUGIN(KDevCppSupportFactory(KAboutData("kdevcppsupport","kdevcpp", ki18n("C++ Support"), "0.1", ki18n("Support for C++ Language"), KAboutData::License_GPL)))
#else
class KDevCppSupportFactory : public KPluginFactory
{
public:
    static KComponentData componentData() { return KComponentData(); };
};
#endif


static QStringList mimeTypesList()
{
    KDesktopFile desktopFile(QStandardPaths::GenericDataLocation, QString("kservices5/kdevcppsupport.desktop"));
    const KConfigGroup& desktopGroup = desktopFile.desktopGroup();
    QString mimeTypesStr = desktopGroup.readEntry("X-KDevelop-SupportedMimeTypes", "");
    return mimeTypesStr.split(QChar(','), QString::SkipEmptyParts);
}

CppLanguageSupport::CppLanguageSupport( QObject* parent, const QVariantList& /*args*/ )
    : KDevelop::IPlugin( "kdevcppsupport", parent ),
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

//    QAction* pimplAction = actions->addAction("code_private_implementation");
//    pimplAction->setText( i18n("Make Class Implementation Private") );
//    pimplAction->setShortcut(Qt::ALT | Qt::META | Qt::Key_P);
//    connect(pimplAction, SIGNAL(triggered(bool)), &SimpleRefactoring::self(), SLOT(executePrivateImplementationAction()));

    QAction* renameDeclarationAction = actions.addAction("code_rename_declaration");
    renameDeclarationAction->setText( i18n("Rename Declaration") );
    renameDeclarationAction->setIcon(QIcon::fromTheme("edit-rename"));
    actions.setDefaultShortcut(renameDeclarationAction, Qt::CTRL | Qt::ALT | Qt::Key_R);
    connect(renameDeclarationAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeRenameAction()));

    QAction* moveIntoSourceAction = actions.addAction("code_move_definition");
    moveIntoSourceAction->setText( i18n("Move into Source") );
    actions.setDefaultShortcut(moveIntoSourceAction, Qt::CTRL | Qt::ALT | Qt::Key_S);
    connect(moveIntoSourceAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeMoveIntoSourceAction()));
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
#pragma message("waiting for Grantlee port")
//     return new CppClassHelper;
    return 0;
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
 * @returns all extensions which match the given @p mimeTypeName.
 */
QSet<QString> getExtensionsByMimeType(QString mimeTypeName)
{
    QMimeType mime = QMimeDatabase().mimeTypeForName(mimeTypeName);
    if (!mime.isValid()) {
        return QSet<QString>();
    }

    QSet<QString> extensions;
    foreach(const QString& suffix, mime.suffixes()) {
        extensions.insert(suffix);
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

QPair<QPair<QString, KTextEditor::Range>, QString> CppLanguageSupport::cursorIdentifier(const KUrl& url, const KTextEditor::Cursor& position) const {
  KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
  if(!doc || !doc->activeTextView())
    return qMakePair(qMakePair(QString(), KTextEditor::Range::invalid()), QString());

  int lineNumber = position.line();
  int lineLength = doc->textDocument()->lineLength(lineNumber);

  QString line = doc->textDocument()->text(KTextEditor::Range(lineNumber, 0, lineNumber, lineLength));

  if(CppUtils::findEndOfInclude(line) != -1) { //If it is an include, return the complete line
    int start = 0;
    while(start < lineLength && line[start] == ' ')
      ++start;

    return qMakePair( qMakePair(line, KTextEditor::Range(lineNumber, start, lineNumber, lineLength)), QString() );
  }

  // not an include, if at all a Makro, hence clear strings
  line = clearStrings(line);

  int start = position.column();
  int end = position.column();

  while(start > 0 && (line[start].isLetterOrNumber() || line[start] == '_') && (line[start-1].isLetterOrNumber() || line[start-1] == '_'))
    --start;

  while(end <  lineLength && (line[end].isLetterOrNumber() || line[end] == '_'))
    ++end;

  KTextEditor::Range wordRange = KTextEditor::Range(KTextEditor::Cursor(lineNumber, start), KTextEditor::Cursor(lineNumber, end));

  return qMakePair( qMakePair(line.mid(start, end-start), wordRange), line.mid(end) );
}

QPair<TopDUContextPointer, KTextEditor::Range> CppLanguageSupport::importedContextForPosition(const KUrl& url, const KTextEditor::Cursor& position) {
  QPair<QPair<QString, KTextEditor::Range>, QString> found = cursorIdentifier(url, position);
  if(!found.first.second.isValid())
    return qMakePair(TopDUContextPointer(), KTextEditor::Range::invalid());

  QString word(found.first.first);
  KTextEditor::Range wordRange(found.first.second);

  int pos = 0;
  for(; pos < word.size(); ++pos) {
    if(word[pos] == '"' || word[pos] == '<') {
      wordRange.start().setColumn(++pos);
      break;
    }
  }

  for(; pos < word.size(); ++pos) {
    if(word[pos] == '"' || word[pos] == '>') {
      wordRange.end().setColumn(pos);
      break;
    }
  }

  if(wordRange.start() > wordRange.end())
    wordRange.start() = wordRange.end();

  //Since this is called by the editor while editing, use a fast timeout so the editor stays responsive
  DUChainReadLocker lock(DUChain::lock(), 100);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return qMakePair(TopDUContextPointer(), KTextEditor::Range::invalid());
  }

  TopDUContext* ctx = standardContext(url);
  if(word.isEmpty() || !ctx || !ctx->parsingEnvironmentFile())
    return qMakePair(TopDUContextPointer(), KTextEditor::Range::invalid());

  if((ctx->parsingEnvironmentFile() && ctx->parsingEnvironmentFile()->isProxyContext())) {
    kDebug() << "Strange: standard-context for" << ctx->url().str() << "is a proxy-context";
    return qMakePair(TopDUContextPointer(), KTextEditor::Range::invalid());
  }

  Cpp::EnvironmentFilePointer p(dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data()));

  Q_ASSERT(p);

  if(CppUtils::findEndOfInclude(word) != -1) {
    //It's an #include, find out which file was included at the given line
    foreach(const DUContext::Import &imported, ctx->importedParentContexts()) {
      if(imported.context(0)) {
        if(ctx->transformFromLocalRevision(ctx->importPosition(imported.context(0))).line() == wordRange.start().line()) {
          if(TopDUContext* importedTop = dynamic_cast<TopDUContext*>(imported.context(0)))
            return qMakePair(TopDUContextPointer(importedTop), wordRange);
        }
      }
    }
  }
  return qMakePair(TopDUContextPointer(), KTextEditor::Range::invalid());
}

QPair<KTextEditor::Range, const rpp::pp_macro*> CppLanguageSupport::usedMacroForPosition(const KUrl& url, const KTextEditor::Cursor& position) {
  //Extract the word under the cursor

  QPair<QPair<QString, KTextEditor::Range>, QString> found = cursorIdentifier(url, position);
  if(!found.first.second.isValid())
    return qMakePair(KTextEditor::Range::invalid(), (const rpp::pp_macro*)0);

  IndexedString word(found.first.first);
  KTextEditor::Range wordRange(found.first.second);

  //Since this is called by the editor while editing, use a fast timeout so the editor stays responsive
  DUChainReadLocker lock(DUChain::lock(), 100);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return qMakePair(KTextEditor::Range::invalid(), (const rpp::pp_macro*)0);
  }

  TopDUContext* ctx = standardContext(url, true);
  if(word.str().isEmpty() || !ctx || !ctx->parsingEnvironmentFile())
    return qMakePair(KTextEditor::Range::invalid(), (const rpp::pp_macro*)0);

  Cpp::EnvironmentFilePointer p(dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data()));

  Q_ASSERT(p);

  if(!p->usedMacroNames().contains(word) && !p->definedMacroNames().contains(word))
    return qMakePair(KTextEditor::Range::invalid(), (const rpp::pp_macro*)0);

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

  return qMakePair(KTextEditor::Range::invalid(), (const rpp::pp_macro*)0);
}

KTextEditor::Range CppLanguageSupport::specialLanguageObjectRange(const KUrl& url, const KTextEditor::Cursor& position) {

  QPair<TopDUContextPointer, KTextEditor::Range> import = importedContextForPosition(url, position);
  if(import.first)
    return import.second;

  return usedMacroForPosition(url, position).first;
}

QPair<KUrl, KTextEditor::Cursor> CppLanguageSupport::specialLanguageObjectJumpCursor(const KUrl& url, const KTextEditor::Cursor& position) {

  QPair<TopDUContextPointer, KTextEditor::Range> import = importedContextForPosition(url, position);
    if(import.first) {
      DUChainReadLocker lock(DUChain::lock());
      if(import.first)
        return qMakePair(KUrl(import.first->url().str()), KTextEditor::Cursor(0,0));
    }

    QPair<KTextEditor::Range, const rpp::pp_macro*> m = usedMacroForPosition(url, position);

    if(!m.first.isValid())
      return qMakePair(QUrl(), KTextEditor::Cursor::invalid());

    return qMakePair(KUrl(m.second->file.str()), KTextEditor::Cursor(m.second->sourceLine, 0));
}

QWidget* CppLanguageSupport::specialLanguageObjectNavigationWidget(const KUrl& url, const KTextEditor::Cursor& position) {

  QPair<TopDUContextPointer, KTextEditor::Range> import = importedContextForPosition(url, position);
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

    QPair<KTextEditor::Range, const rpp::pp_macro*> m = usedMacroForPosition(url, position);
    if(!m.first.isValid())
      return 0;

    //Evaluate the preprocessed body
    QPair<QPair<QString, KTextEditor::Range>, QString> found = cursorIdentifier(url, position);

    QString text = found.first.first;
    QString preprocessedBody;
    //Check whether tail contains arguments
    QString tail = found.second.trimmed(); ///@todo make this better.
    if(tail.startsWith("(")) {
      //properly support macro expansions when arguments contain newlines
      int foundClosingBrace = findClose( tail, 0 );
      KDevelop::IDocument* doc = core()->documentController()->documentForUrl(url);
      if(doc && doc->activeTextView() && foundClosingBrace < 0) {
        const int lines = doc->textDocument()->lines();
        for (int lineNumber = position.line() + 1; foundClosingBrace < 0 && lineNumber < lines; lineNumber++) {
          tail += doc->textDocument()->line(lineNumber).trimmed();
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
          preprocessedBody = Cpp::preprocess(text, p, position.line()+1);
        }
      }
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
