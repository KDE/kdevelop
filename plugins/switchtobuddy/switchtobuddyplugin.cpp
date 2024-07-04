/*
    SPDX-FileCopyrightText: 2012 André Stein <andre.stein@rwth-aachen.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "switchtobuddyplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KActionCollection>
#include <QAction>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ibuddydocumentfinder.h>
#include <interfaces/contextmenuextension.h>
#include <language/interfaces/editorcontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <debug.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace KDevelop;

namespace {

KTextEditor::Cursor normalizeCursor(KTextEditor::Cursor c)
{
    c.setColumn(0);
    return c;
}

///Tries to find a definition for the declaration at given cursor-position and document-url. DUChain must be locked.
Declaration* definitionForCursorDeclaration(const KTextEditor::Cursor& cursor, const QUrl& url)
{
    const QList<TopDUContext*> topContexts = DUChain::self()->chainsForDocument(url);
    for (TopDUContext* ctx : topContexts) {
        Declaration* decl = DUChainUtils::declarationInLine(cursor, ctx);
        if (!decl) {
            continue;
        }
        if (auto definition = FunctionDefinition::definition(decl)) {
            return definition;
        }
    }
    return nullptr;
}

QString findSwitchCandidate(const QUrl& docUrl)
{
    QMimeDatabase db;
    IBuddyDocumentFinder* finder = IBuddyDocumentFinder::finderForMimeType(db.mimeTypeForUrl(docUrl).name());
    if (finder) {
        // get the first entry that exists, use that as candidate
        const auto potentialBuddies = finder->potentialBuddies(docUrl);
        for (const QUrl& buddyUrl : potentialBuddies) {
            if (!QFile::exists(buddyUrl.toLocalFile())) {
                continue;
            }

            return buddyUrl.toLocalFile();
        }
    }
    return QString();
}

}

K_PLUGIN_FACTORY_WITH_JSON(SwitchToBuddyPluginFactory, "kdevswitchtobuddy.json", registerPlugin<SwitchToBuddyPlugin>(); )

SwitchToBuddyPlugin::SwitchToBuddyPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("kdevswitchtobuddy"), parent, metaData)
{
    setXMLFile(QStringLiteral("kdevswitchtobuddy.rc"));
}

SwitchToBuddyPlugin::~SwitchToBuddyPlugin()
{
}

ContextMenuExtension SwitchToBuddyPlugin::contextMenuExtension(Context* context, QWidget* parent)
{
    auto* ctx = dynamic_cast<EditorContext*>(context);
    if (!ctx) {
        return ContextMenuExtension();
    }

    QUrl currentUrl = ctx->url();
    IBuddyDocumentFinder* buddyFinder = IBuddyDocumentFinder::finderForMimeType(QMimeDatabase().mimeTypeForUrl(currentUrl).name());
    if (!buddyFinder)
        return ContextMenuExtension();

    // Get all potential buddies for the current document and add a switch-to action
    // for each buddy who really exists in the file system. Note: if no buddies could be calculated
    // no extension actions are generated.
    const QVector<QUrl>& potentialBuddies = buddyFinder->potentialBuddies(currentUrl);

    ContextMenuExtension extension;

    for (const QUrl& url : potentialBuddies) {
        if (!QFile::exists(url.toLocalFile())) {
            continue;
        }

        auto* action = new QAction(i18nc("@action:inmenu", "Switch to '%1'", url.fileName()), parent);
        const QString surl = url.toLocalFile();
        connect(action, &QAction::triggered, this, [this, surl](){ switchToBuddy(surl); }, Qt::QueuedConnection);
        extension.addAction(ContextMenuExtension::NavigationGroup, action);
    }

    return extension;
}

void SwitchToBuddyPlugin::createActionsForMainWindow(Sublime::MainWindow* /*window*/, QString& xmlFile, KActionCollection& actions)
{
    xmlFile = this->xmlFile();

    QAction* switchDefinitionDeclaration = actions.addAction(QStringLiteral("switch_definition_declaration"));
    switchDefinitionDeclaration->setText(i18nc("@action", "&Switch Definition/Declaration"));
    actions.setDefaultShortcut(switchDefinitionDeclaration, Qt::CTRL | Qt::SHIFT | Qt::Key_C);
    connect(switchDefinitionDeclaration, &QAction::triggered, this, &SwitchToBuddyPlugin::switchDefinitionDeclaration);

    QAction* switchHeaderSource = actions.addAction(QStringLiteral("switch_header_source"));
    switchHeaderSource->setText(i18nc("@action", "Switch Header/Source"));
    actions.setDefaultShortcut(switchHeaderSource, Qt::CTRL | Qt::Key_Slash);
    connect(switchHeaderSource, &QAction::triggered, this, &SwitchToBuddyPlugin::switchHeaderSource);
}

void SwitchToBuddyPlugin::switchHeaderSource()
{
    qCDebug(PLUGIN_SWITCHTOBUDDY) << "switching header/source";

    auto doc = ICore::self()->documentController()->activeDocument();
    if (!doc)
        return;

    QString buddyUrl = findSwitchCandidate(doc->url());
    if (!buddyUrl.isEmpty())
        switchToBuddy(buddyUrl);
}

void SwitchToBuddyPlugin::switchToBuddy(const QString& url)
{
    KDevelop::ICore::self()->documentController()->openDocument(QUrl::fromLocalFile(url));
}

void SwitchToBuddyPlugin::switchDefinitionDeclaration()
{
    qCDebug(PLUGIN_SWITCHTOBUDDY) << "switching definition/declaration";

    QUrl docUrl;
    KTextEditor::Cursor cursor;

    ///Step 1: Find the current top-level context of type DUContext::Other(the highest code-context).
    ///-- If it belongs to a function-declaration or definition, it can be retrieved through owner(), and we are in a definition.
    ///-- If no such context could be found, search for a declaration on the same line as the cursor, and switch to the according definition
    {
        auto view = ICore::self()->documentController()->activeTextDocumentView();
        if (!view) {
            qCDebug(PLUGIN_SWITCHTOBUDDY) << "No active document";
            return;
        }

        docUrl = view->document()->url();
        cursor = view->cursorPosition();
    }

    QString switchCandidate = findSwitchCandidate(docUrl);
    if(!switchCandidate.isEmpty()) {

        DUChainReadLocker lock;

        //If the file has not been parsed yet, update it
        TopDUContext* ctx = DUChainUtils::standardContextForUrl(docUrl);
        //At least 'VisibleDeclarationsAndContexts' is required so we can do a switch
        if (!ctx || (ctx->parsingEnvironmentFile() && !ctx->parsingEnvironmentFile()->featuresSatisfied(TopDUContext::AllDeclarationsContextsAndUses))) {
            lock.unlock();
            qCDebug(PLUGIN_SWITCHTOBUDDY) << "Parsing switch-candidate before switching" << switchCandidate;
            ReferencedTopDUContext updatedContext = DUChain::self()->waitForUpdate(IndexedString(switchCandidate), TopDUContext::AllDeclarationsContextsAndUses);
            if (!updatedContext) {
                qCDebug(PLUGIN_SWITCHTOBUDDY) << "Failed to update document:" << switchCandidate;
                return;
            }
        }
    }

    qCDebug(PLUGIN_SWITCHTOBUDDY) << "Document:" << docUrl;

    DUChainReadLocker lock;

    TopDUContext* standardCtx = DUChainUtils::standardContextForUrl(docUrl);

    bool wasSignal = false;
    if (standardCtx) {
        Declaration* definition = nullptr;

        DUContext* ctx = standardCtx->findContext(standardCtx->transformToLocalRevision(cursor));
        if (!ctx) {
            ctx = standardCtx;
        }

        while (ctx && ctx->parentContext() && (ctx->parentContext()->type() == DUContext::Other || ctx->parentContext()->type() == DUContext::Function)) {
            ctx = ctx->parentContext();
        }

        if (ctx && ctx->owner() && (ctx->type() == DUContext::Other || ctx->type() == DUContext::Function) && ctx->owner()->isDefinition()) {
            definition = ctx->owner();
            qCDebug(PLUGIN_SWITCHTOBUDDY) << "found definition while traversing:" << definition->toString();
        }

        if (!definition && ctx) {
            definition = DUChainUtils::declarationInLine(cursor, ctx);
        }

        if (auto* cDef = dynamic_cast<ClassFunctionDeclaration*>(definition)) {
            if (cDef->isSignal()) {
                qCDebug(PLUGIN_SWITCHTOBUDDY) << "found definition is a signal, not switching to .moc implementation";
                definition = nullptr;
                wasSignal = true;
            }
        }

        auto* def = dynamic_cast<FunctionDefinition*>(definition);
        if (def && def->declaration()) {
            Declaration* declaration = def->declaration();
            KTextEditor::Range targetRange = declaration->rangeInCurrentRevision();
            const auto url = declaration->url().toUrl();
            qCDebug(PLUGIN_SWITCHTOBUDDY) << "found definition that has declaration: " << definition->toString() << "range" << targetRange << "url" << url;
            lock.unlock();

            auto view = ICore::self()->documentController()->activeTextDocumentView();
            if (view && !targetRange.contains(view->cursorPosition())) {
                const auto pos = normalizeCursor(targetRange.start());
                ICore::self()->documentController()->openDocument(url, KTextEditor::Range(pos, pos));
            } else {
                ICore::self()->documentController()->openDocument(url);
            }
            return;
        } else {
            qCDebug(PLUGIN_SWITCHTOBUDDY) << "Definition has no assigned declaration";
        }

        qCDebug(PLUGIN_SWITCHTOBUDDY) << "Could not get definition/declaration from context";
    } else {
        qCDebug(PLUGIN_SWITCHTOBUDDY) << "Got no context for the current document";
    }

    Declaration* def = nullptr;
    if (!wasSignal) {
        def = definitionForCursorDeclaration(cursor, docUrl);
    }

    if (def) {
        const auto url = def->url().toUrl();
        KTextEditor::Range targetRange = def->rangeInCurrentRevision();

        if (def->internalContext()) {
            targetRange.end() = def->internalContext()->rangeInCurrentRevision().end();
        } else {
            qCDebug(PLUGIN_SWITCHTOBUDDY) << "Declaration does not have internal context";
        }
        lock.unlock();

        auto view = ICore::self()->documentController()->activeTextDocumentView();
        if (view && !targetRange.contains(view->cursorPosition())) {
            KTextEditor::Cursor pos(normalizeCursor(targetRange.start()));
            ICore::self()->documentController()->openDocument(url, KTextEditor::Range(pos, pos));
        } else {
            //The cursor is already in the target range, only open the document
            ICore::self()->documentController()->openDocument(url);
        }
        return;
    } else if (!wasSignal) {
        qCWarning(PLUGIN_SWITCHTOBUDDY) << "Found no definition assigned to cursor position";
    }

    lock.unlock();

    ///- If no definition/declaration could be found to switch to, just switch the document using normal header/source heuristic by file-extension
    if (!switchCandidate.isEmpty()) {
        ICore::self()->documentController()->openDocument(QUrl::fromUserInput(switchCandidate));
    } else {
        qCDebug(PLUGIN_SWITCHTOBUDDY) << "Found no source/header candidate to switch";
    }
}

#include "switchtobuddyplugin.moc"
#include "moc_switchtobuddyplugin.cpp"
