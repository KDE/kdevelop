/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "textdocument.h"

#include <QAction>
#include <QFile>
#include <QMenu>
#include <QMimeDatabase>
#include <QPointer>
#include <QWidget>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/ModificationInterface>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/MarkInterface>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <vcs/interfaces/icontentawareversioncontrol.h>

#include <language/interfaces/editorcontext.h>
#include <language/backgroundparser/backgroundparser.h>

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
        if (!range.isEmpty()) {
            view->setSelection(range);
        }
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
        delete addedContextMenu;
        addedContextMenu = nullptr;

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

        q->notifyStateChanged();
        Core::self()->uiControllerInternal()->setStatusIcon(q, statusIcon);
    }

    inline KConfigGroup katePartSettingsGroup() const
    {
        return KSharedConfig::openConfig()->group("KatePart Settings");
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
        if ( !qobject_cast<KTextEditor::ModificationInterface*>( document ) ) {
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

    void modifiedOnDisk(KTextEditor::Document *document, bool /*isModified*/,
        KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
    {
        bool dirty = false;
        switch (reason)
        {
            case KTextEditor::ModificationInterface::OnDiskUnmodified:
                break;
            case KTextEditor::ModificationInterface::OnDiskModified:
            case KTextEditor::ModificationInterface::OnDiskCreated:
            case KTextEditor::ModificationInterface::OnDiskDeleted:
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

    TextDocument * const q;

    QPointer<KTextEditor::Document> document;
    IDocument::DocumentState state = IDocument::Clean;
    QString encoding;
    bool loaded = false;
    // we want to remove the added stuff when the menu hides
    QMenu* addedContextMenu = nullptr;
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

bool TextDocument::isTextDocument() const
{
    Q_D(const TextDocument);

    if( !d->document )
    {
        /// @todo Somehow it can happen that d->document is zero, which makes
        /// code relying on "isTextDocument() == (bool)textDocument()" crash
        qCWarning(SHELL) << "Broken text-document: " << url();
        return false;
    }

    return true;
}

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
                    (TopDUContext::Features) ( TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate ),
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

        if (qobject_cast<KTextEditor::MarkInterface*>(d->document)) {
            // can't use new signal/slot syntax here, MarkInterface is not a QObject
            connect(d->document.data(), SIGNAL(marksChanged(KTextEditor::Document*)),
                    this, SLOT(saveSessionConfig()));
        }

        if (auto iface = qobject_cast<KTextEditor::ModificationInterface*>(d->document)) {
            iface->setModifiedOnDiskWarning(true);
            // can't use new signal/slot syntax here, ModificationInterface is not a QObject
            connect(d->document.data(), SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this, SLOT(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        }

        notifyTextDocumentCreated();
    }

    view = d->document->createView(parent, Core::self()->uiControllerInternal()->defaultMainWindow()->kateWrapper()->interface());

    // get rid of some actions regarding the config dialog, we merge that one into the kdevelop menu already
    delete view->actionCollection()->action(QStringLiteral("set_confdlg"));
    delete view->actionCollection()->action(QStringLiteral("editor_options"));

    view->setStatusBarEnabled(Core::self()->partControllerInternal()->showTextEditorStatusBar());

    connect(view, &KTextEditor::View::contextMenuAboutToShow, this, &TextDocument::populateContextMenu);

    if (auto* cc = qobject_cast<KTextEditor::CodeCompletionInterface*>(view))
        cc->setAutomaticInvocationEnabled(core()->languageController()->completionSettings()->automaticCompletionEnabled());

    return view;
}

KParts::Part *TextDocument::partForView(QWidget *view) const
{
    Q_D(const TextDocument);

    if (d->document && d->document->views().contains((KTextEditor::View*)view))
        return d->document;
    return nullptr;
}



// KDevelop::IDocument implementation

void TextDocument::reload()
{
    Q_D(TextDocument);

    if (!d->document)
        return;

    KTextEditor::ModificationInterface* modif=nullptr;
    if(d->state ==Dirty) {
        modif = qobject_cast<KTextEditor::ModificationInterface*>(d->document);
        modif->setModifiedOnDiskWarning(false);
    }
    d->document->documentReload();
    if(modif)
        modif->setModifiedOnDiskWarning(true);
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
                int code = KMessageBox::warningYesNoCancel(
                    Core::self()->uiController()->activeMainWindow(),
                    i18n("The file \"%1\" is modified on disk.\n\nAre "
                        "you sure you want to overwrite it? (External "
                        "changes will be lost.)", d->document->url().toLocalFile()),
                    i18nc("@title:window", "Document Externally Modified"));
                if (code != KMessageBox::Yes)
                    return false;
            }
            break;
    }

    if (!KDevelop::ensureWritable(QList<QUrl>() << url())) {
        return false;
    }

    QUrl urlBeforeSave = d->document->url();
    if (d->document->documentSave())
    {
        if (d->document->url() != urlBeforeSave)
            notifyUrlChanged();
        return true;
    }
    return false;
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
        view->setCursorPosition(cursor);
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
    Q_D(const TextDocument);

    if (!d->document) {
        return QString();
    }

    return d->document->text( range );
}

QString TextDocument::textLine() const
{
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

QIcon KDevelop::TextDocument::defaultIcon() const
{
    Q_D(const TextDocument);

    if (d->document) {
        QMimeType mime = QMimeDatabase().mimeTypeForName(d->document->mimeType());
        QIcon icon = QIcon::fromTheme(mime.iconName());
        if (!icon.isNull()) {
            return icon;
        }
    }
    return PartDocument::defaultIcon();
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

void KDevelop::TextDocument::populateContextMenu( KTextEditor::View* v, QMenu* menu )
{
    Q_D(TextDocument);

    if (d->addedContextMenu) {
        const auto actions = d->addedContextMenu->actions();
        for (QAction* action : actions) {
            menu->removeAction(action);
        }
        delete d->addedContextMenu;
    }

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
    KTextEditor::ModificationInterface* modIface = qobject_cast<KTextEditor::ModificationInterface*>( d->document );
    Q_ASSERT(modIface);
    // Ok, all safe, we can clean up the document. Close it if the file is gone,
    // and reload if it's still there.
    d->setStatus(d->document, false);
    modIface->setModifiedOnDisk(KTextEditor::ModificationInterface::OnDiskUnmodified);
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
    notifyStateChanged();
}

void KDevelop::TextDocument::documentUrlChanged(KTextEditor::Document* document)
{
    Q_D(TextDocument);

    Q_UNUSED(document);
    if (url() != d->document->url())
        setUrl(d->document->url());
}

#include "moc_textdocument.cpp"
