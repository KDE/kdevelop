/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>
    Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#include "util/clangdebug.h"
#include "util/clangtypes.h"

#include "codecompletion/model.h"

#include "documentfinderhelpers.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>
#include <language/interfaces/iastcontainer.h>

#include "codegen/simplerefactoring.h"
#include "codegen/adaptsignatureassistant.h"
#include "duchain/clangindex.h"
#include "duchain/navigationwidget.h"
#include "duchain/macrodefinition.h"
#include "duchain/clangparsingenvironmentfile.h"
#include "duchain/duchainutils.h"

#include <language/assistant/staticassistantsmanager.h>
#include <language/assistant/renameassistant.h>
#include <language/codecompletion/codecompletion.h>
#include <language/highlighting/codehighlighting.h>
#include <language/interfaces/editorcontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/use.h>
#include <language/editor/documentcursor.h>

#include <KActionCollection>

#include <QAction>
#include <QRegExp>

K_PLUGIN_FACTORY_WITH_JSON(KDevClangSupportFactory, "kdevclangsupport.json", registerPlugin<ClangSupport>(); )

using namespace KDevelop;

namespace {

/**
 * Extract the range of the path-spec inside the include-directive in line @p line
 *
 * Example: line = "#include <vector>" => returns {0, 10, 0, 16}
 *
 * @param originalRange This is the range that the resulting range will be based on
 *
 * @return Range pointing to the path-spec of the include or invalid range if there is no #include directive on the line.
 */
KTextEditor::Range rangeForIncludePathSpec(const QString& line, const KTextEditor::Range& originalRange = KTextEditor::Range())
{
    if (!line.contains(QRegExp("^\\s*#include"))) {
        return KTextEditor::Range::invalid();
    }

    KTextEditor::Range range = originalRange;
    int pos = 0;
    for (; pos < line.size(); ++pos) {
        if(line[pos] == '"' || line[pos] == '<') {
            range.setStart({range.start().line(), ++pos});
            break;
        }
    }

    for (; pos < line.size(); ++pos) {
        if(line[pos] == '"' || line[pos] == '>') {
            range.setEnd({range.start().line(), pos});
            break;
        }
    }

    if(range.start() > range.end()) {
        range.setStart(range.end());
    }
    return range;
}

QPair<QString, KTextEditor::Range> lineInDocument(const QUrl &url, const KTextEditor::Cursor& position)
{
    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if (!doc || !doc->textDocument() || !ICore::self()->documentController()->activeTextDocumentView()) {
        return {};
    }

    const int lineNumber = position.line();
    const int lineLength = doc->textDocument()->lineLength(lineNumber);
    KTextEditor::Range range(lineNumber, 0, lineNumber, lineLength);
    QString line = doc->textDocument()->text(range);
    return {line, range};
}

QPair<TopDUContextPointer, KTextEditor::Range> importedContextForPosition(const QUrl &url, const KTextEditor::Cursor& position)
{
    auto pair = lineInDocument(url, position);

    const QString line = pair.first;
    if (line.isEmpty())
        return {{}, KTextEditor::Range::invalid()};

    KTextEditor::Range wordRange = rangeForIncludePathSpec(line, pair.second);
    if (!wordRange.isValid()) {
        return {{}, KTextEditor::Range::invalid()};
    }

    // Since this is called by the editor while editing, use a fast timeout so the editor stays responsive
    DUChainReadLocker lock(nullptr, 100);
    if (!lock.locked()) {
        clangDebug() << "Failed to lock the du-chain in time";
        return {TopDUContextPointer(), KTextEditor::Range::invalid()};
    }

    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
    if (line.isEmpty() || !topContext || !topContext->parsingEnvironmentFile()) {
        return {TopDUContextPointer(), KTextEditor::Range::invalid()};
    }

    if ((topContext->parsingEnvironmentFile() && topContext->parsingEnvironmentFile()->isProxyContext())) {
        clangDebug() << "Strange: standard-context for" << topContext->url().str() << "is a proxy-context";
        return {TopDUContextPointer(), KTextEditor::Range::invalid()};
    }

    // It's an #include, find out which file was included at the given line
    foreach(const DUContext::Import &imported, topContext->importedParentContexts()) {
        auto context = imported.context(nullptr);
        if (context) {
            if(topContext->transformFromLocalRevision(topContext->importPosition(context)).line() == wordRange.start().line()) {
                if (auto importedTop = dynamic_cast<TopDUContext*>(context))
                    return {TopDUContextPointer(importedTop), wordRange};
            }
        }
    }

    // The last resort. Check if the file is already included (maybe recursively from another files).
    // This is needed as clang doesn't visit (clang_getInclusions) those inclusions.
    // TODO: Maybe create an assistant that'll report whether the file is already included?
    auto includeName = line.mid(wordRange.start().column(), wordRange.end().column() - wordRange.start().column());

    if (!includeName.isEmpty()) {
        if (includeName.startsWith('.')) {
            const Path dir = Path(url).parent();
            includeName = Path(dir, includeName).toLocalFile();
        }

        const auto recursiveImports = topContext->recursiveImportIndices();
        auto iterator = recursiveImports.iterator();
        while (iterator) {
            const auto str = (*iterator).url().str();
            if (str == includeName || (str.endsWith(includeName) && str[str.size()-includeName.size()-1]=='/')) {
                return {TopDUContextPointer((*iterator).data()), wordRange};
            }
            ++iterator;
        }
    }

    return {{}, KTextEditor::Range::invalid()};
}

QPair<TopDUContextPointer, Use> macroExpansionForPosition(const QUrl &url, const KTextEditor::Cursor& position)
{
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
    if (topContext) {
        int useAt = topContext->findUseAt(topContext->transformToLocalRevision(position));
        if (useAt >= 0) {
            Use use = topContext->uses()[useAt];
            if (dynamic_cast<MacroDefinition*>(use.usedDeclaration(topContext))) {
                return {TopDUContextPointer(topContext), use};
            }
        }
    }
    return {{}, {}};
}

}

ClangSupport::ClangSupport(QObject* parent, const QVariantList& )
    : IPlugin( "kdevclangsupport", parent )
    , ILanguageSupport()
    , m_highlighting(nullptr)
    , m_refactoring(nullptr)
    , m_index(nullptr)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    setXMLFile( "kdevclangsupport.rc" );

    ClangIntegration::DUChainUtils::registerDUChainItems();

    m_highlighting = new KDevelop::CodeHighlighting(this);
    m_refactoring = new SimpleRefactoring(this);
    m_index.reset(new ClangIndex);

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
    parseLock()->lockForWrite();
    // By locking the parse-mutexes, we make sure that parse jobs get a chance to finish in a good state
    parseLock()->unlock();

    for(const auto& type : DocumentFinderHelpers::mimeTypesList()) {
        KDevelop::IBuddyDocumentFinder::removeFinder(type);
    }

    ClangIntegration::DUChainUtils::unregisterDUChainItems();
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

bool ClangSupport::areBuddies(const QUrl &url1, const QUrl& url2)
{
    return DocumentFinderHelpers::areBuddies(url1, url2);
}

bool ClangSupport::buddyOrder(const QUrl &url1, const QUrl& url2)
{
    return DocumentFinderHelpers::buddyOrder(url1, url2);
}

QVector< QUrl > ClangSupport::getPotentialBuddies(const QUrl &url) const
{
    return DocumentFinderHelpers::getPotentialBuddies(url);
}

void ClangSupport::createActionsForMainWindow (Sublime::MainWindow* /*window*/, QString& _xmlFile, KActionCollection& actions)
{
    _xmlFile = xmlFile();

    QAction* renameDeclarationAction = actions.addAction("code_rename_declaration");
    renameDeclarationAction->setText( i18n("Rename Declaration") );
    renameDeclarationAction->setIcon(QIcon::fromTheme("edit-rename"));
    renameDeclarationAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_R);
    connect(renameDeclarationAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeRenameAction()));
}

KDevelop::ContextMenuExtension ClangSupport::contextMenuExtension(KDevelop::Context* context)
{
    ContextMenuExtension cm;
    EditorContext *ec = dynamic_cast<KDevelop::EditorContext *>(context);

    if (ec && ICore::self()->languageController()->languagesForUrl(ec->url()).contains(this)) {
        // It's a C++ file, let's add our context menu.
        m_refactoring->fillContextMenu(cm, context);
    }
    return cm;
}

KTextEditor::Range ClangSupport::specialLanguageObjectRange(const QUrl &url, const KTextEditor::Cursor& position)
{
    DUChainReadLocker lock;
    const QPair<TopDUContextPointer, Use> macroExpansion = macroExpansionForPosition(url, position);
    if (macroExpansion.first) {
        return macroExpansion.first->transformFromLocalRevision(macroExpansion.second.m_range);
    }

    const QPair<TopDUContextPointer, KTextEditor::Range> import = importedContextForPosition(url, position);
    if(import.first) {
        return import.second;
    }

    return KTextEditor::Range::invalid();
}

QPair<QUrl, KTextEditor::Cursor> ClangSupport::specialLanguageObjectJumpCursor(const QUrl &url, const KTextEditor::Cursor& position)
{
    const QPair<TopDUContextPointer, KTextEditor::Range> import = importedContextForPosition(url, position);
    DUChainReadLocker lock;
    if (import.first) {
        return qMakePair(import.first->url().toUrl(), KTextEditor::Cursor(0,0));
    }

    return {{}, KTextEditor::Cursor::invalid()};
}

QWidget* ClangSupport::specialLanguageObjectNavigationWidget(const QUrl &url, const KTextEditor::Cursor& position)
{
    DUChainReadLocker lock;
    const QPair<TopDUContextPointer, Use> macroExpansion = macroExpansionForPosition(url, position);
    if (macroExpansion.first) {
        Declaration* declaration = macroExpansion.second.usedDeclaration(macroExpansion.first.data());
        const MacroDefinition::Ptr macroDefinition(dynamic_cast<MacroDefinition*>(declaration));
        Q_ASSERT(macroDefinition);
        auto rangeInRevision = macroExpansion.first->transformFromLocalRevision(macroExpansion.second.m_range.start);
        return new ClangNavigationWidget(macroDefinition, DocumentCursor(IndexedString(url), rangeInRevision));
    }

    const QPair<TopDUContextPointer, KTextEditor::Range> import = importedContextForPosition(url, position);

    if (import.first) {
        // Prefer a standardContext, because the included one may have become empty due to
        if (import.first->localDeclarations().count() == 0 && import.first->childContexts().count() == 0) {
            KDevelop::TopDUContext* betterCtx = standardContext(import.first->url().toUrl());
            if (betterCtx && (betterCtx->localDeclarations().count() != 0 || betterCtx->childContexts().count() != 0)) {
                return betterCtx->createNavigationWidget(0, 0, i18n("Emptied by preprocessor<br />"));
            }
        }
        return import.first->createNavigationWidget();
    }
    return nullptr;
}

TopDUContext* ClangSupport::standardContext(const QUrl &url, bool /*proxyContext*/)
{
    ClangParsingEnvironment env;
    return DUChain::self()->chainForDocument(url, &env);
}

#include "clangsupport.moc"
