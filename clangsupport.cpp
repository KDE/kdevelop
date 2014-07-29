/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "clangsupport.h"

#include "clangparsejob.h"
#include "version.h"

#include "duchain/clangtypes.h"

#include "codecompletion/model.h"

#include "documentfinderhelpers.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>

#include "codegen/simplerefactoring.h"
#include "codegen/adaptsignatureassistant.h"

#include <language/assistant/staticassistantsmanager.h>
#include <language/assistant/renameassistant.h>
#include <language/codecompletion/codecompletion.h>
#include <language/highlighting/codehighlighting.h>
#include <language/interfaces/editorcontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/parsingenvironment.h>

#include <KAction>
#include <KActionCollection>

K_PLUGIN_FACTORY(KDevClangSupportFactory, registerPlugin<ClangSupport>(); )
K_EXPORT_PLUGIN(KDevClangSupportFactory(
    KAboutData("kdevclangsupport", 0, ki18n("Clang Plugin"), "0.1",
    ki18n("Support for C, C++ and Objective-C languages."), KAboutData::License_GPL)))

using namespace KDevelop;

namespace {

/**
 * Extract the range of the path-spec inside the include-directive in line @p line
 *
 * Example: line = "#include <vector>" => returns {0, 10, 0, 16}
 *
 * @param originalRange This is the range that the resulting range will be based on
 *
 * @return Range pointing to the path-spec of the include.)
 */
SimpleRange rangeForIncludePathSpec(const QString& line, const SimpleRange& originalRange = SimpleRange())
{
    SimpleRange range = originalRange;
    int pos = 0;
    for (; pos < line.size(); ++pos) {
        if(line[pos] == '"' || line[pos] == '<') {
            range.start.column = ++pos;
            break;
        }
    }

    for (; pos < line.size(); ++pos) {
        if(line[pos] == '"' || line[pos] == '>') {
            range.end.column = pos;
            break;
        }
    }

    if(range.start > range.end) {
        range.start = range.end;
    }
    return range;
}

QPair<QString, SimpleRange> lineInDocument(const KUrl& url, const SimpleCursor& position)
{
    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if (!doc || !doc->textDocument() || !doc->textDocument()->activeView()) {
        return {};
    }

    const int lineNumber = position.line;
    const int lineLength = doc->textDocument()->lineLength(lineNumber);
    KTextEditor::Range range(lineNumber, 0, lineNumber, lineLength);
    QString line = doc->textDocument()->text(range);
    return {line, range};
}

QPair<TopDUContextPointer, SimpleRange> importedContextForPosition(const KUrl& url, const SimpleCursor& position)
{
  auto pair = lineInDocument(url, position);

    const QString line = pair.first;
    if (line.isEmpty())
        return {{}, SimpleRange::invalid()};

    SimpleRange wordRange = rangeForIncludePathSpec(line, pair.second);

    // Since this is called by the editor while editing, use a fast timeout so the editor stays responsive
    DUChainReadLocker lock(nullptr, 100);
    if (!lock.locked()) {
        kDebug() << "Failed to lock the du-chain in time";
        return {TopDUContextPointer(), SimpleRange::invalid()};
    }

    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
    if (line.isEmpty() || !topContext || !topContext->parsingEnvironmentFile()) {
        return {TopDUContextPointer(), SimpleRange::invalid()};
    }

    if ((topContext->parsingEnvironmentFile() && topContext->parsingEnvironmentFile()->isProxyContext())) {
        kDebug() << "Strange: standard-context for" << topContext->url().str() << "is a proxy-context";
        return {TopDUContextPointer(), SimpleRange::invalid()};
    }

    // It's an #include, find out which file was included at the given line
    foreach(const DUContext::Import &imported, topContext->importedParentContexts()) {
        auto context = imported.context(nullptr);
        if (context) {
            if(topContext->transformFromLocalRevision(topContext->importPosition(context)).line == wordRange.start.line) {
                if (auto importedTop = dynamic_cast<TopDUContext*>(context))
                    return {TopDUContextPointer(importedTop), wordRange};
            }
        }
    }
    return {{}, SimpleRange::invalid()};
}


}

ClangSupport::ClangSupport(QObject* parent, const QVariantList& )
: IPlugin( KDevClangSupportFactory::componentData(), parent )
, ILanguageSupport()
, m_highlighting(new KDevelop::CodeHighlighting(this))
, m_refactoring(new SimpleRefactoring(this))
, m_index(new ClangIndex)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    setXMLFile( "kdevclangsupport.rc" );

    new KDevelop::CodeCompletion( this, new ClangCodeCompletionModel(this), name() );
    for(const auto& type : DocumentFinderHelpers::mimeTypesList()){
        KDevelop::IBuddyDocumentFinder::addFinder(type, this);
    }

    auto assistantsManager = core()->languageController()->staticAssistantsManager();
    assistantsManager->registerAssistant(StaticAssistant::Ptr(new RenameAssistant(this)));
    assistantsManager->registerAssistant(StaticAssistant::Ptr(new AdaptSignatureAssistant(this)));
}

ClangSupport::~ClangSupport()
{
    for(const auto& type : DocumentFinderHelpers::mimeTypesList()) {
        KDevelop::IBuddyDocumentFinder::removeFinder(type);
    }
}

ParseJob* ClangSupport::createParseJob(const IndexedString& url)
{
    return new ClangParseJob(url, this);
}

QString ClangSupport::name() const
{
    return "clang";
}

ICodeHighlighting* ClangSupport::codeHighlighting() const
{
    return m_highlighting;
}

BasicRefactoring* ClangSupport::refactoring() const
{
    return m_refactoring;
}

ClangIndex* ClangSupport::index()
{
    return m_index.data();
}

bool ClangSupport::areBuddies(const KUrl& url1, const KUrl& url2)
{
    return DocumentFinderHelpers::areBuddies(url1, url2);
}

bool ClangSupport::buddyOrder(const KUrl& url1, const KUrl& url2)
{
    return DocumentFinderHelpers::buddyOrder(url1, url2);
}

QVector< KUrl > ClangSupport::getPotentialBuddies(const KUrl& url) const
{
    return DocumentFinderHelpers::getPotentialBuddies(url);
}

void ClangSupport::createActionsForMainWindow (Sublime::MainWindow* /*window*/, QString& _xmlFile, KActionCollection& actions)
{
    _xmlFile = xmlFile();

    KAction* renameDeclarationAction = actions.addAction("code_rename_declaration");
    renameDeclarationAction->setText( i18n("Rename Declaration") );
    renameDeclarationAction->setIcon(KIcon("edit-rename"));
    renameDeclarationAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_R);
    connect(renameDeclarationAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeRenameAction()));
}

KDevelop::ContextMenuExtension ClangSupport::contextMenuExtension(KDevelop::Context* context)
{
  ContextMenuExtension cm;
  EditorContext *ec = dynamic_cast<KDevelop::EditorContext *>(context);

  if (ec && ICore::self()->languageController()->languagesForUrl(ec->url()).contains(language())) {
    // It's a C++ file, let's add our context menu.
    m_refactoring->fillContextMenu(cm, context);
  }
  return cm;
}

SimpleRange ClangSupport::specialLanguageObjectRange(const KUrl& url, const SimpleCursor& position)
{
    const QPair<TopDUContextPointer, SimpleRange> import = importedContextForPosition(url, position);
    if(import.first) {
        return import.second;
    }

    return SimpleRange::invalid();
}

QPair<KUrl, KDevelop::SimpleCursor> ClangSupport::specialLanguageObjectJumpCursor(const KUrl& url, const SimpleCursor& position)
{
    const QPair<TopDUContextPointer, SimpleRange> import = importedContextForPosition(url, position);
    DUChainReadLocker lock;
    if (import.first) {
        return qMakePair(KUrl(import.first->url().str()), SimpleCursor(0,0));
    }

    return {{}, SimpleCursor::invalid()};
}

QWidget* ClangSupport::specialLanguageObjectNavigationWidget(const KUrl& url, const SimpleCursor& position)
{
    const QPair<TopDUContextPointer, SimpleRange> import = importedContextForPosition(url, position);

    DUChainReadLocker lock;
    if (!import.first) {
        return nullptr;
    }

    // Prefer a standardContext, because the included one may have become empty due to
    if (import.first->localDeclarations().count() == 0 && import.first->childContexts().count() == 0) {
        KDevelop::TopDUContext* betterCtx = standardContext(KUrl(import.first->url().str()));
        if (betterCtx && (betterCtx->localDeclarations().count() != 0 || betterCtx->childContexts().count() != 0)) {
            return betterCtx->createNavigationWidget(0, 0, i18n("Emptied by preprocessor<br />"));
        }
    }
    return import.first->createNavigationWidget();
}

#include "clangsupport.moc"
