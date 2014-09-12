/*
 * This file is part of KDevelop
 * Copyright 2012 André Stein <andre.stein@rwth-aachen.de>
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
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

#include "switchtobuddyplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KActionCollection>
#include <KAction>
#include <QFile>
#include <QSignalMapper>
#include <QMimeDatabase>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ibuddydocumentfinder.h>
#include <language/interfaces/editorcontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

using namespace KDevelop;

namespace {

KTextEditor::Cursor normalizeCursor(KTextEditor::Cursor c)
{
    c.setColumn(0);
    return c;
}

///Tries to find a definition for the declaration at given cursor-position and document-url. DUChain must be locked.
Declaration* definitionForCursorDeclaration(const KTextEditor::Cursor& cursor, const KUrl& url)
{
    QList<TopDUContext*> topContexts = DUChain::self()->chainsForDocument(url);
    foreach (TopDUContext* ctx, topContexts) {
        Declaration* decl = DUChainUtils::declarationInLine(cursor, ctx);
        if (!decl) {
            continue;
        }
        if (auto definition = FunctionDefinition::definition(decl)) {
            return definition;
        }
    }
    return 0;
}

QString findSwitchCandidate(const KUrl& docUrl)
{
    IBuddyDocumentFinder* finder = IBuddyDocumentFinder::finderForMimeType(KMimeType::findByUrl(docUrl)->name());
    if (finder) {
        // get the first entry that exists, use that as candidate
        foreach(const KUrl& buddyUrl, finder->getPotentialBuddies(docUrl)) {
            if (!QFile::exists(buddyUrl.toLocalFile())) {
                continue;
            }

            return buddyUrl.toLocalFile();
        }
    }
    return QString();
}

}

K_PLUGIN_FACTORY(SwitchToBuddyPluginFactory, registerPlugin<SwitchToBuddyPlugin>(); )
// K_EXPORT_PLUGIN(SwitchToBuddyPluginFactory(KAboutData(
//                                             "kdevswitchtobuddy"
//                                            ,"kdevswitchtobuddy"
//                                            , ki18n("Switch to Buddy")
//                                            , "0.1"
//                                            , ki18n("Allows switching between buddy documents like implementation and header file.")
//                                            , KAboutData::License_GPL)
//                                            .addAuthor(ki18n("André Stein")
//                                            , ki18n("Author")
//                                            , "andre.stein@rwth-aachen.de"
//                                            , "http://steinsoft.net")
// ))

SwitchToBuddyPlugin::SwitchToBuddyPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( "kdevswitchtobuddy", parent )
    , m_signalMapper(0)
{
    setXMLFile("kdevswitchtobuddy.rc");
}

SwitchToBuddyPlugin::~SwitchToBuddyPlugin()
{
}

ContextMenuExtension SwitchToBuddyPlugin::contextMenuExtension(Context* context)
{
    EditorContext* ctx = dynamic_cast<EditorContext*>(context);
    if (!ctx) {
        return ContextMenuExtension();
    }

    KUrl currentUrl = ctx->url();
    IBuddyDocumentFinder* buddyFinder = IBuddyDocumentFinder::finderForMimeType(QMimeDatabase().mimeTypeForUrl(currentUrl).name());
    if (!buddyFinder)
        return ContextMenuExtension();

    // Get all potential buddies for the current document and add a switch-to action
    // for each buddy who really exists in the file system. Note: if no buddies could be calculated
    // no extension actions are generated.
    const QVector<KUrl>& potentialBuddies = buddyFinder->getPotentialBuddies(currentUrl);

    ContextMenuExtension extension;
    if (m_signalMapper) {
        delete m_signalMapper;
    }
    m_signalMapper = new QSignalMapper(this);

    foreach(const KUrl& url, potentialBuddies) {
        if (!QFile::exists(url.toLocalFile())) {
            continue;
        }

        QAction* action = new QAction(i18n("Switch to '%1'", url.fileName()), this);
        connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()), Qt::QueuedConnection);
        m_signalMapper->setMapping(action, url.toLocalFile());
        connect(m_signalMapper, SIGNAL(mapped(const QString&)),
                this, SLOT(switchToBuddy(const QString&)), Qt::QueuedConnection);
        extension.addAction(ContextMenuExtension::ExtensionGroup, action);
    }

    return extension;
}

void SwitchToBuddyPlugin::createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions)
{
    xmlFile = this->xmlFile();

    QAction* switchDefinitionDeclaration = actions.addAction("switch_definition_declaration");
    switchDefinitionDeclaration->setText( i18n("&Switch Definition/Declaration") );
    actions.setDefaultShortcut( switchDefinitionDeclaration, Qt::CTRL | Qt::SHIFT | Qt::Key_C );
    connect(switchDefinitionDeclaration, SIGNAL(triggered(bool)), this, SLOT(switchDefinitionDeclaration()));
}

void SwitchToBuddyPlugin::switchToBuddy(const QString& url)
{
    KDevelop::ICore::self()->documentController()->openDocument(url);
}

void SwitchToBuddyPlugin::switchDefinitionDeclaration()
{
    kDebug() << "switching definition/declaration";

    KUrl docUrl;
    KTextEditor::Cursor cursor;

    ///Step 1: Find the current top-level context of type DUContext::Other(the highest code-context).
    ///-- If it belongs to a function-declaration or definition, it can be retrieved through owner(), and we are in a definition.
    ///-- If no such context could be found, search for a declaration on the same line as the cursor, and switch to the according definition
    {
        auto view = ICore::self()->documentController()->activeTextDocumentView();
        if (!view) {
            kDebug() << "No active document";
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
            kDebug() << "Parsing switch-candidate before switching" << switchCandidate;
            ReferencedTopDUContext updatedContext = DUChain::self()->waitForUpdate(IndexedString(switchCandidate), TopDUContext::AllDeclarationsContextsAndUses);
            if (!updatedContext) {
                kDebug() << "Failed to update document:" << switchCandidate;
                return;
            }
        }
    }

    kDebug() << "Document:" << docUrl;

    DUChainReadLocker lock;

    TopDUContext* standardCtx = DUChainUtils::standardContextForUrl(docUrl);

    bool wasSignal = false;
    if (standardCtx) {
        Declaration* definition = 0;

        DUContext* ctx = standardCtx->findContext(standardCtx->transformToLocalRevision(cursor));
        if (!ctx) {
            ctx = standardCtx;
        }

        while (ctx && ctx->parentContext() && (ctx->parentContext()->type() == DUContext::Other || ctx->parentContext()->type() == DUContext::Function)) {
            ctx = ctx->parentContext();
        }

        if (ctx && ctx->owner() && (ctx->type() == DUContext::Other || ctx->type() == DUContext::Function) && ctx->owner()->isDefinition()) {
            definition = ctx->owner();
            kDebug() << "found definition while traversing:" << definition->toString();
        }

        if (!definition && ctx) {
            definition = DUChainUtils::declarationInLine(cursor, ctx);
        }

        if (ClassFunctionDeclaration* cDef = dynamic_cast<ClassFunctionDeclaration*>(definition)) {
            if (cDef->isSignal()) {
                kDebug() << "found definition is a signal, not switching to .moc implementation";
                definition = 0;
                wasSignal = true;
            }
        }

        FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(definition);
        if (def && def->declaration()) {
            Declaration* declaration = def->declaration();
            KTextEditor::Range targetRange = declaration->rangeInCurrentRevision();
            const auto url = declaration->url().toUrl();
            kDebug() << "found definition that has declaration: " << definition->toString() << "range" << targetRange << "url" << url;
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
            kDebug() << "Definition has no assigned declaration";
        }

        kDebug() << "Could not get definition/declaration from context";
    } else {
        kDebug() << "Got no context for the current document";
    }

    Declaration* def = 0;
    if (!wasSignal) {
        def = definitionForCursorDeclaration(cursor, docUrl);
    }

    if (def) {
        const auto url = def->url().toUrl();
        KTextEditor::Range targetRange = def->rangeInCurrentRevision();

        if (def->internalContext()) {
            targetRange.end() = def->internalContext()->rangeInCurrentRevision().end();
        } else {
            kDebug() << "Declaration does not have internal context";
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
        kWarning() << "Found no definition assigned to cursor position";
    }

    lock.unlock();

    ///- If no definition/declaration could be found to switch to, just switch the document using normal header/source heuristic by file-extension
    if (!switchCandidate.isEmpty()) {
        ICore::self()->documentController()->openDocument(KUrl(switchCandidate));
    } else {
        kDebug() << "Found no source/header candidate to switch";
    }
}

#include "switchtobuddyplugin.moc"
