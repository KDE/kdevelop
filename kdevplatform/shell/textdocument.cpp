/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "textdocument.h"

#include <QAction>
#include <QFile>
#include <QMenu>
#include <QPointer>
#include <QWidget>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <vcs/interfaces/icontentawareversioncontrol.h>

#include <language/interfaces/editorcontext.h>
#include <language/backgroundparser/backgroundparser.h>

#include <util/foregroundlock.h>

#include "core.h"
#include "mainwindow.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "documentcontroller.h"
#include "ktexteditorpluginintegration.h"
#include "debug.h"

#include <path.h>
#include <shellutils.h>

namespace KDevelop {

const int MAX_DOC_SETTINGS = 20;

// This sets cursor position and selection on the view to the given
// range. Selection is set only for non-empty ranges
// Factored into a function since its needed in 3 places already
static void selectAndReveal( KTextEditor::View* view, const KTextEditor::Range& range ) {
    Q_ASSERT(view);
    if (range.isValid()) {
        view->setCursorPosition(range.start());
        view->setSelection(range);
    }
}

class TextDocumentPrivate
{
public:
    explicit TextDocumentPrivate(TextDocument *textDocument)
        : q(textDocument)
    {
    }

    ~TextDocumentPrivate()
    {
        // Handle the case we are being deleted while the context menu is not yet hidden.
        // We want to remove all actions we added to it, especially those not owned by the document
        // but by the plugins (i.e. created on-the-fly during ContextMenuExtension::populateMenu
        // with ownership set to our addedContextMenu)
        cleanContextMenu();

        saveSessionConfig();
        delete document;
    }

    void setStatus(KTextEditor::Document* document, bool dirty)
    {
        QIcon statusIcon;

        if (document->isModified())
            if (dirty) {
                state = IDocument::DirtyAndModified;
                statusIcon = QIcon::fromTheme(QStringLiteral("edit-delete"));
            } else {
                state = IDocument::Modified;
                statusIcon = QIcon::fromTheme(QStringLiteral("document-save"));
            }
        else
            if (dirty) {
                state = IDocument::Dirty;
                statusIcon = QIcon::fromTheme(QStringLiteral("document-revert"));
            } else {
                state = IDocument::Clean;
            }

        q->setStatusIcon(statusIcon);
        q->notifyStateChanged();
    }

    inline KConfigGroup katePartSettingsGroup() const
    {
        return KSharedConfig::openConfig()->group(QStringLiteral("KatePart Settings"));
    }

    inline QString docConfigGroupName() const
    {
        return document->url().toDisplayString(QUrl::PreferLocalFile);
    }

    inline KConfigGroup docConfigGroup() const
    {
        return katePartSettingsGroup().group(docConfigGroupName());
    }

    void saveSessionConfig()
    {
        if(document && document->url().isValid()) {
            // make sure only MAX_DOC_SETTINGS entries are stored
            KConfigGroup katePartSettings = katePartSettingsGroup();
            // ordered list of documents
            QStringList documents = katePartSettings.readEntry("documents", QStringList());
            // ensure this document is "new", i.e. at the end of the list
            documents.removeOne(docConfigGroupName());
            documents.append(docConfigGroupName());
            // remove "old" documents + their group
            while(documents.size() >= MAX_DOC_SETTINGS) {
                katePartSettings.group(documents.takeFirst()).deleteGroup();
            }
            // update order
            katePartSettings.writeEntry("documents", documents);

            // actually save session config
            KConfigGroup group = docConfigGroup();
            document->writeSessionConfig(group);
        }
    }

    void loadSessionConfig()
    {
        if (!document || !katePartSettingsGroup().hasGroup(docConfigGroupName())) {
            return;
        }

        document->readSessionConfig(docConfigGroup(), {QStringLiteral("SkipUrl")});
    }

    // Determines whether the current contents of this document in the editor
    // could be retrieved from the VCS if they were dismissed.
    void queryCanRecreateFromVcs(KTextEditor::Document* document) const {
        // Find projects by checking which one contains the file's parent directory,
        // to avoid issues with the cmake manager temporarily removing files from a project
        // during reloading.
        KDevelop::Path path(document->url());
        const auto projects = Core::self()->projectController()->projects();
        auto projectIt = std::find_if(projects.begin(), projects.end(), [&](KDevelop::IProject* project) {
            return project->path().isParentOf(path);
        });
        if (projectIt == projects.end()) {
            return;
        }
        IProject* project = *projectIt;

        IContentAwareVersionControl* iface;
        iface = qobject_cast< KDevelop::IContentAwareVersionControl* >(project->versionControlPlugin());
        if (!iface) {
            return;
        }

        CheckInRepositoryJob* req = iface->isInRepository( document );
        if ( !req ) {
            return;
        }
        QObject::connect(req, &CheckInRepositoryJob::finished,
                            q, &TextDocument::repositoryCheckFinished);
        // Abort the request when the user edits the document
        QObject::connect(q->textDocument(), &KTextEditor::Document::textChanged,
                            req, &CheckInRepositoryJob::abort);
    }

    void modifiedOnDisk(KTextEditor::Document* document, bool /*isModified*/,
                        KTextEditor::Document::ModifiedOnDiskReason reason)
    {
        bool dirty = false;
        switch (reason)
        {
        case KTextEditor::Document::OnDiskUnmodified:
            break;
        case KTextEditor::Document::OnDiskModified:
        case KTextEditor::Document::OnDiskCreated:
        case KTextEditor::Document::OnDiskDeleted:
            dirty = true;
            break;
        }

        // In some cases, the VCS (e.g. git) can know whether the old contents are "valuable", i.e.
        // not retrieveable from the VCS. If that is not the case, then the document can safely be
        // reloaded without displaying a dialog asking the user.
        if ( dirty ) {
            queryCanRecreateFromVcs(document);
        }
        setStatus(document, dirty);
    }

    void cleanContextMenu()
    {
        if (!addedContextMenu) {
            return;
        }

        if (currentContextMenu) {
            const auto actions = addedContextMenu->actions();
            for (QAction* action : actions) {
                currentContextMenu->removeAction(action);
            }
            currentContextMenu.clear();
        }

        // The addedContextMenu owns those actions created on-the-fly for the context menu
        // (other than those actions only shared for the context menu, but also used elsewhere)
        // and thuse deletes then on its own destruction.
        // Some actions potentially could be connected to triggered-signal handlers
        // using Qt::QueuedConnection (at least SwitchToBuddyPlugin does so currently).
        // Deleting them here also would also delete the connection before the handler is triggered.
        // So we delay the menu's and thus their destruction to the next eventloop by default.
        addedContextMenu->deleteLater();
        addedContextMenu = nullptr;
    }

    TextDocument * const q;

    QPointer<KTextEditor::Document> document;
    IDocument::DocumentState state = IDocument::Clean;
    QString encoding;
    bool loaded = false;
    // we want to remove the added stuff when the menu hides
    QMenu* addedContextMenu = nullptr;
    QPointer<QMenu> currentContextMenu;
};

class TextViewPrivate
{
public:
    explicit TextViewPrivate(TextView* q) : q(q) {}

    TextView* const q;
    QPointer<KTextEditor::View> view;
    KTextEditor::Range initialRange;
};

TextDocument::TextDocument(const QUrl &url, ICore* core, const QString& encoding)
    : PartDocument(url, core)
    , d_ptr(new TextDocumentPrivate(this))
{
    Q_D(TextDocument);

    d->encoding = encoding;
}

TextDocument::~TextDocument() = default;

KTextEditor::Document *TextDocument::textDocument() const
{
    Q_D(const TextDocument);

    return d->document;
}

QWidget *TextDocument::createViewWidget(QWidget *parent)
{
    Q_D(TextDocument);

    KTextEditor::View* view = nullptr;

    if (!d->document)
    {
        d->document = Core::self()->partControllerInternal()->createTextPart();

        // Connect to the first text changed signal, it occurs before the completed() signal
        connect(d->document.data(), &KTextEditor::Document::textChanged, this, &TextDocument::slotDocumentLoaded);
        // Also connect to the completed signal, sometimes the first text changed signal is missed because the part loads too quickly (? TODO - confirm this is necessary)
        connect(d->document.data(), QOverload<>::of(&KTextEditor::Document::completed),
                this, &TextDocument::slotDocumentLoaded);

        // force a reparse when a document gets reloaded
        connect(d->document.data(), &KTextEditor::Document::reloaded,
                this, [] (KTextEditor::Document* document) {
            ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString(document->url()),
                    TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate,
                    BackgroundParser::BestPriority, nullptr);
        });

        // Set encoding passed via constructor
        // Needs to be done before openUrl, else katepart won't use the encoding
        // @see KTextEditor::Document::setEncoding
        if (!d->encoding.isEmpty())
            d->document->setEncoding(d->encoding);

        if (!url().isEmpty() && !DocumentController::isEmptyDocumentUrl(url()))
            d->document->openUrl( url() );

        d->setStatus(d->document, false);

        /* It appears, that by default a part will be deleted the
           first view containing it is deleted.  Since we do want
           to have several views, disable that behaviour.  */
        d->document->setAutoDeletePart(false);

        Core::self()->partController()->addPart(d->document, false);

        d->loadSessionConfig();

        connect(d->document.data(), &KTextEditor::Document::modifiedChanged,
                 this, &TextDocument::newDocumentStatus);
        connect(d->document.data(), &KTextEditor::Document::textChanged,
                 this, &TextDocument::textChanged);
        connect(d->document.data(), &KTextEditor::Document::documentUrlChanged,
                 this, &TextDocument::documentUrlChanged);
        connect(d->document.data(), &KTextEditor::Document::documentSavedOrUploaded,
                 this, &TextDocument::documentSaved );

        connect(d->document.data(), &KTextEditor::Document::marksChanged, this, [d] {
            d->saveSessionConfig();
        });

        d->document->setModifiedOnDiskWarning(true);
        connect(
            d->document.data(), &KTextEditor::Document::modifiedOnDisk, this,
            [d](KTextEditor::Document* document, bool isModified, KTextEditor::Document::ModifiedOnDiskReason reason) {
                d->modifiedOnDisk(document, isModified, reason);
            });

        notifyTextDocumentCreated();
    }

    view = d->document->createView(parent, Core::self()->uiControllerInternal()->defaultMainWindow()->kateWrapper()->interface());

    // get rid of some actions regarding the config dialog, we merge that one into the kdevelop menu already
    delete view->actionCollection()->action(QStringLiteral("set_confdlg"));
    delete view->actionCollection()->action(QStringLiteral("editor_options"));

    view->setStatusBarEnabled(Core::self()->partControllerInternal()->showTextEditorStatusBar());

    connect(view, &KTextEditor::View::contextMenuAboutToShow, this, &TextDocument::populateContextMenu);

    view->setAutomaticInvocationEnabled(
        core()->languageController()->completionSettings()->automaticCompletionEnabled());

    return view;
}

KParts::Part *TextDocument::partForView(QWidget *view) const
{
    Q_D(const TextDocument);

    if (d->document && d->document->views().contains(qobject_cast<KTextEditor::View*>(view)))
        return d->document;
    return nullptr;
}



// KDevelop::IDocument implementation

void TextDocument::reload()
{
    Q_D(TextDocument);

    if (!d->document)
        return;

    d->document->documentReload();
}

bool TextDocument::save(DocumentSaveMode mode)
{
    Q_D(TextDocument);

    if (!d->document)
        return true;

    if (mode & Discard)
        return true;

    switch (d->state)
    {
        case IDocument::Clean:
            return true;

        case IDocument::Modified:
            break;

        case IDocument::Dirty:
        case IDocument::DirtyAndModified:
            if (!(mode & Silent))
            {
                int code = KMessageBox::warningTwoActionsCancel(
                    Core::self()->uiController()->activeMainWindow(),
                    i18n("The file \"%1\" is modified on disk.\n\nAre "
                         "you sure you want to overwrite it? (External "
                         "changes will be lost.)",
                         d->document->url().toLocalFile()),
                    i18nc("@title:window", "Document Externally Modified"),
                    KGuiItem(i18nc("@action:button", "Overwrite External Changes"), QStringLiteral("document-save")),
                    KStandardGuiItem::discard());
                if (code == KMessageBox::SecondaryAction) {
                    return true; // Discard
                } else if (code == KMessageBox::Cancel) {
                    return false;
                }
                mode = Silent; // prevent documentSave() from asking essentially the same question again
            }
            break;
    }

    if (!KDevelop::ensureWritable(QList<QUrl>() << url())) {
        return false;
    }

    if (mode & Silent) {
        // Set the KTextEditor equivalent to DocumentSaveMode::Silent.
        d->document->setModifiedOnDiskWarning(false);
    }
    const auto saved = d->document->documentSave();
    if (mode & Silent) {
        // Restore the default value. Unfortunately, a getter to query the previous value is missing.
        d->document->setModifiedOnDiskWarning(true);
    }
    return saved;
}

IDocument::DocumentState TextDocument::state() const
{
    Q_D(const TextDocument);

    return d->state;
}

KTextEditor::Cursor KDevelop::TextDocument::cursorPosition() const
{
    Q_D(const TextDocument);

    if (!d->document) {
        return KTextEditor::Cursor::invalid();
    }

    KTextEditor::View *view = activeTextView();

    if (view)
        return view->cursorPosition();

    return KTextEditor::Cursor::invalid();
}

void TextDocument::setCursorPosition(const KTextEditor::Cursor &cursor)
{
    Q_D(TextDocument);

    if (!cursor.isValid() || !d->document)
        return;

    KTextEditor::View *view = activeTextView();

    // Rodda: Cursor must be accurate here, to the definition of accurate for KTextEditor::Cursor.
    // ie, starting from 0,0

    if (view)
        selectAndReveal(view, {cursor, cursor});
}

KTextEditor::Range TextDocument::textSelection() const
{
    Q_D(const TextDocument);

    if (!d->document) {
        return KTextEditor::Range::invalid();
    }

    KTextEditor::View *view = activeTextView();

    if (view && view->selection()) {
        return view->selectionRange();
    }

    return PartDocument::textSelection();
}

QString TextDocument::text(const KTextEditor::Range &range) const
{
    VERIFY_FOREGROUND_LOCKED
    Q_D(const TextDocument);

    if (!d->document) {
        return QString();
    }

    return d->document->text( range );
}

QString TextDocument::textLine() const
{
    VERIFY_FOREGROUND_LOCKED
    Q_D(const TextDocument);

    if (!d->document) {
        return QString();
    }

    KTextEditor::View *view = activeTextView();

    if (view) {
        return d->document->line( view->cursorPosition().line() );
    }

    return PartDocument::textLine();
}

QString TextDocument::textWord() const
{
    VERIFY_FOREGROUND_LOCKED
    Q_D(const TextDocument);

    if (!d->document) {
        return QString();
    }

    KTextEditor::View *view = activeTextView();

    if (view) {
        KTextEditor::Cursor start = view->cursorPosition();
        qCDebug(SHELL) << "got start position from view:" << start.line() << start.column();
        QString linestr = textLine();
        int startPos = qMax( qMin( start.column(), linestr.length() - 1 ), 0 );
        int endPos = startPos;
        startPos --;
        while (startPos >= 0 &&
               (linestr[startPos].isLetterOrNumber() || linestr[startPos] == QLatin1Char('_') || linestr[startPos] == QLatin1Char('~'))) {
            --startPos;
        }

        while (endPos < linestr.length() &&
               (linestr[endPos].isLetterOrNumber() || linestr[endPos] == QLatin1Char('_') || linestr[endPos] == QLatin1Char('~'))) {
            ++endPos;
        }
        if( startPos != endPos )
        {
            qCDebug(SHELL) << "found word" << startPos << endPos << linestr.mid( startPos+1, endPos - startPos - 1 );
            return linestr.mid( startPos + 1, endPos - startPos - 1 );
        }
    }

    return PartDocument::textWord();
}

void TextDocument::setTextSelection(const KTextEditor::Range &range)
{
    Q_D(TextDocument);

    if (!range.isValid() || !d->document)
        return;

    KTextEditor::View *view = activeTextView();

    if (view) {
        selectAndReveal(view, range);
    }
}

Sublime::View* TextDocument::newView(Sublime::Document* doc)
{
    Q_UNUSED(doc);
    return new TextView(this);
}

}

KDevelop::TextView::TextView(TextDocument * doc)
    : View(doc, View::TakeOwnership)
    , d_ptr(new TextViewPrivate(this))
{
}

KDevelop::TextView::~TextView() = default;

QWidget * KDevelop::TextView::createWidget(QWidget * parent)
{
    Q_D(TextView);

    auto textDocument = qobject_cast<TextDocument*>(document());
    Q_ASSERT(textDocument);
    QWidget* widget = textDocument->createViewWidget(parent);
    d->view = qobject_cast<KTextEditor::View*>(widget);
    Q_ASSERT(d->view);
    connect(d->view.data(), &KTextEditor::View::cursorPositionChanged, this, &KDevelop::TextView::sendStatusChanged);
    return widget;
}

void KDevelop::TextView::setInitialRange(const KTextEditor::Range& range)
{
    Q_D(TextView);

    if (d->view) {
        selectAndReveal(d->view, range);
    } else {
        d->initialRange = range;
    }
}

KTextEditor::Range KDevelop::TextView::initialRange() const
{
    Q_D(const TextView);

    return d->initialRange;
}

void KDevelop::TextView::readSessionConfig(KConfigGroup& config)
{
    Q_D(TextView);

    if (!d->view) {
        return;
    }
    d->view->readSessionConfig(config);
}

void KDevelop::TextView::writeSessionConfig(KConfigGroup& config)
{
    Q_D(TextView);

    if (!d->view) {
        return;
    }
    d->view->writeSessionConfig(config);
}

QString KDevelop::TextDocument::documentType() const
{
    return QStringLiteral("Text");
}

KTextEditor::View *KDevelop::TextView::textView() const
{
    Q_D(const TextView);

    return d->view;
}

QString KDevelop::TextView::viewStatus() const
{
    Q_D(const TextView);

    // only show status when KTextEditor's own status bar isn't already enabled
    const bool showStatus = !Core::self()->partControllerInternal()->showTextEditorStatusBar();
    if (!showStatus) {
        return QString();
    }

    const KTextEditor::Cursor pos = d->view ? d->view->cursorPosition() : KTextEditor::Cursor::invalid();
    return i18n(" Line: %1 Col: %2 ", pos.line() + 1, pos.column() + 1);
}

void KDevelop::TextView::sendStatusChanged()
{
    emit statusChanged(this);
}

KTextEditor::View* KDevelop::TextDocument::activeTextView() const
{
    KTextEditor::View* fallback = nullptr;
    for (auto view : views()) {
        auto textView = qobject_cast<TextView*>(view)->textView();
        if (!textView) {
            continue;
        }
        if (textView->hasFocus()) {
            return textView;
        } else if (textView->isVisible()) {
            fallback = textView;
        } else if (!fallback) {
            fallback = textView;
        }
    }
    return fallback;
}

void KDevelop::TextDocument::newDocumentStatus(KTextEditor::Document *document)
{
    Q_D(TextDocument);

    bool dirty = (d->state == IDocument::Dirty || d->state == IDocument::DirtyAndModified);

    d->setStatus(document, dirty);
}

void KDevelop::TextDocument::textChanged(KTextEditor::Document *document)
{
    Q_UNUSED(document);
    notifyContentChanged();
}

void KDevelop::TextDocument::unpopulateContextMenu()
{
    Q_D(TextDocument);

    auto* menu = qobject_cast<QMenu*>(sender());

    disconnect(menu, &QMenu::aboutToHide, this, &TextDocument::unpopulateContextMenu);

    d->cleanContextMenu();
}

void KDevelop::TextDocument::populateContextMenu( KTextEditor::View* v, QMenu* menu )
{
    Q_D(TextDocument);

    if (d->addedContextMenu) {
        qCWarning(SHELL) << "populateContextMenu() called while we still handled another menu.";
        d->cleanContextMenu();
    }

    d->currentContextMenu = menu;
    connect(menu, &QMenu::aboutToHide, this, &TextDocument::unpopulateContextMenu);

    d->addedContextMenu = new QMenu();

    EditorContext c(v, v->cursorPosition());
    auto extensions = Core::self()->pluginController()->queryPluginsForContextMenuExtensions(&c, d->addedContextMenu);

    ContextMenuExtension::populateMenu(d->addedContextMenu, extensions);

    const auto actions = d->addedContextMenu->actions();
    for (QAction* action : actions) {
        menu->addAction(action);
    }
}

void KDevelop::TextDocument::repositoryCheckFinished(bool canRecreate) {
    Q_D(TextDocument);

    if ( d->state != IDocument::Dirty && d->state != IDocument::DirtyAndModified ) {
        // document is not dirty for whatever reason, nothing to do.
        return;
    }
    if ( ! canRecreate ) {
        return;
    }
    // Ok, all safe, we can clean up the document. Close it if the file is gone,
    // and reload if it's still there.
    d->document->setModifiedOnDisk(KTextEditor::Document::OnDiskUnmodified);
    if ( QFile::exists(d->document->url().path()) ) {
        reload();
    } else {
        close(KDevelop::IDocument::Discard);
    }
}

void KDevelop::TextDocument::slotDocumentLoaded()
{
    Q_D(TextDocument);

    if (d->loaded)
        return;
    // Tell the editor integrator first
    d->loaded = true;
    notifyLoaded();
}

void KDevelop::TextDocument::documentSaved(KTextEditor::Document* document, bool saveAs)
{
    Q_UNUSED(document);
    Q_UNUSED(saveAs);
    notifySaved();
}

void KDevelop::TextDocument::documentUrlChanged(KTextEditor::Document* document)
{
    Q_D(TextDocument);

    Q_ASSERT(document == d->document); // we connect only to our document's signal

    // KDevelop prevents a document URL change to empty to relieve slots connected to the
    // IDocumentController::documentUrlChanged() signal from dealing with empty URLs. Furthermore,
    // an empty IDocument URL is not unique, which is likely to break code that assumes otherwise.

    // TODO: prevent this in KTextEditor and assert that d->document->url() is not empty
    // instead of the early return when building against a fixed KTextEditor version.

    // This is a workaround for not yet available necessary KTextEditor changes.
    if (d->document->url().isEmpty()) {
        qCDebug(SHELL) << "ignoring KTextEditor::Document URL change to empty; current URL:"
                       << url().toString(QUrl::PreferLocalFile);
        return;
    }

    if (url() != d->document->url())
        setUrl(d->document->url());
}

#include "moc_textdocument.cpp"
