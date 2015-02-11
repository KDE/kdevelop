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

#include <QFile>
#include <QPointer>
#include <QTextCodec>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QMimeDatabase>
#include <QDebug>

#include <KLocalizedString>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kxmlguifactory.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <vcs/interfaces/icontentawareversioncontrol.h>

#include <language/interfaces/editorcontext.h>
#include <language/backgroundparser/backgroundparser.h>

#include <project/projectutils.h>
#include <project/projectmodel.h>

#include "core.h"
#include "mainwindow.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "documentcontroller.h"
#include "debug.h"
#include <path.h>

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

struct TextDocumentPrivate {
    TextDocumentPrivate(TextDocument *textDocument)
        : encoding(""), m_textDocument(textDocument)
        , m_loaded(false), m_addedContextMenu(0)
    {
        document = 0;
        state = IDocument::Clean;
    }

    ~TextDocumentPrivate()
    {
        if (m_addedContextMenu) {
            delete m_addedContextMenu;
            m_addedContextMenu = 0;
        }
        if (document) {
            saveSessionConfig();
            delete document;
        }
    }

    QPointer<KTextEditor::Document> document;
    IDocument::DocumentState state;
    QString encoding;

    void newDocumentStatus(KTextEditor::Document *document)
    {
        bool dirty = (state == IDocument::Dirty || state == IDocument::DirtyAndModified);

        setStatus(document, dirty);
    }

    void textChanged(KTextEditor::Document *document)
    {
        Q_UNUSED(document);
        m_textDocument->notifyContentChanged();
    }

    void populateContextMenu( KTextEditor::View* v, QMenu* menu )
    {
        if (m_addedContextMenu) {
            foreach ( QAction* action, m_addedContextMenu->actions() ) {
                menu->removeAction(action);
            }
            delete m_addedContextMenu;
        }

        m_addedContextMenu = new QMenu();

        Context* c = new EditorContext( v, v->cursorPosition() );
        QList<ContextMenuExtension> extensions = Core::self()->pluginController()->queryPluginsForContextMenuExtensions( c );

        ContextMenuExtension::populateMenu(m_addedContextMenu, extensions);

        {
            QUrl url = v->document()->url();
            QList< ProjectBaseItem* > items = Core::self()->projectController()->projectModel()->itemsForPath( IndexedString(url) );
            if (!items.isEmpty()) {
                populateParentItemsMenu( items.front(), m_addedContextMenu );
            }
        }

        foreach ( QAction* action, m_addedContextMenu->actions() ) {
            menu->addAction(action);
        }
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

    // Determines whether the current contents of this document in the editor
    // could be retrieved from the VCS if they were dismissed.
    void queryCanRecreateFromVcs(KTextEditor::Document* document) const {
        IProject* project = 0;
        // Find projects by checking which one contains the file's parent directory,
        // to avoid issues with the cmake manager temporarily removing files from a project
        // during reloading.
        KDevelop::Path path(document->url());
        foreach ( KDevelop::IProject* current, Core::self()->projectController()->projects() ) {
            if ( current->path().isParentOf(path) ) {
                project = current;
                break;
            }
        }
        if (!project) {
            return;
        }
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
                         m_textDocument, [&] (bool canRecreate) { m_textDocument->d->repositoryCheckFinished(canRecreate); });
        // Abort the request when the user edits the document
        QObject::connect(m_textDocument->textDocument(), &KTextEditor::Document::textChanged,
                         req, &CheckInRepositoryJob::abort);
    }

    void repositoryCheckFinished(bool canRecreate) {
        if ( state != IDocument::Dirty && state != IDocument::DirtyAndModified ) {
            // document is not dirty for whatever reason, nothing to do.
            return;
        }
        if ( ! canRecreate ) {
            return;
        }
        KTextEditor::ModificationInterface* modIface = qobject_cast<KTextEditor::ModificationInterface*>( document );
        Q_ASSERT(modIface);
        // Ok, all safe, we can clean up the document. Close it if the file is gone,
        // and reload if it's still there.
        setStatus(document, false);
        modIface->setModifiedOnDisk(KTextEditor::ModificationInterface::OnDiskUnmodified);
        if ( QFile::exists(document->url().path()) ) {
            m_textDocument->reload();
        } else {
            m_textDocument->close(KDevelop::IDocument::Discard);
        }
    }

    void setStatus(KTextEditor::Document* document, bool dirty)
    {
        QIcon statusIcon;

        if (document->isModified())
            if (dirty) {
                state = IDocument::DirtyAndModified;
                statusIcon = QIcon::fromTheme("edit-delete");
            } else {
                state = IDocument::Modified;
                statusIcon = QIcon::fromTheme("document-save");
            }
        else
            if (dirty) {
                state = IDocument::Dirty;
                statusIcon = QIcon::fromTheme("document-revert");
            } else {
                state = IDocument::Clean;
            }

        m_textDocument->notifyStateChanged();
        Core::self()->uiControllerInternal()->setStatusIcon(m_textDocument, statusIcon);
    }

    void documentUrlChanged(KTextEditor::Document* document)
    {
        if (m_textDocument->url() != document->url())
            m_textDocument->setUrl(document->url());
    }

    void slotDocumentLoaded()
    {
        if (m_loaded)
            return;
        // Tell the editor integrator first
        m_loaded = true;
        m_textDocument->notifyLoaded();
    }

    void documentSaved(KTextEditor::Document* document, bool saveAs)
    {
        Q_UNUSED(document);
        Q_UNUSED(saveAs);
        m_textDocument->notifySaved();
        m_textDocument->notifyStateChanged();
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
        if(document->url().isValid()) {
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

        document->readSessionConfig(docConfigGroup(), {"SkipUrl"});
    }


private:
    TextDocument *m_textDocument;
    bool m_loaded;
    // we want to remove the added stuff when the menu hides
    QMenu* m_addedContextMenu;
};

struct TextViewPrivate
{
    TextViewPrivate(TextView* q) : q(q) {}

    void sendStatusChanged();

    TextView* const q;
    QPointer<KTextEditor::View> view;
    KTextEditor::Range initialRange;
};

TextDocument::TextDocument(const QUrl &url, ICore* core, const QString& encoding)
    :PartDocument(url, core), d(new TextDocumentPrivate(this))
{
    d->encoding = encoding;
}

TextDocument::~TextDocument()
{
    delete d;
}

bool TextDocument::isTextDocument() const
{
    if( !d->document )
    {
        /// @todo Somehow it can happen that d->document is zero, which makes
        /// code relying on "isTextDocument() == (bool)textDocument()" crash
        qWarning() << "Broken text-document: " << url();
        return false;
    }

    return true;
}

KTextEditor::Document *TextDocument::textDocument() const
{
    return d->document;
}

QWidget *TextDocument::createViewWidget(QWidget *parent)
{
    KTextEditor::View* view = 0L;

    if (!d->document)
    {
        d->document = Core::self()->partControllerInternal()->createTextPart(Core::self()->documentController()->encoding());

        // Connect to the first text changed signal, it occurs before the completed() signal
        connect(d->document.data(), &KTextEditor::Document::textChanged, this, [&] { d->slotDocumentLoaded(); });
        // Also connect to the completed signal, sometimes the first text changed signal is missed because the part loads too quickly (? TODO - confirm this is necessary)
        connect(d->document.data(), static_cast<void(KTextEditor::Document::*)()>(&KTextEditor::Document::completed), this, [&] { d->slotDocumentLoaded(); });

        // force a reparse when a document gets reloaded
        connect(d->document.data(), &KTextEditor::Document::reloaded,
                this, [] (KTextEditor::Document* document) {
            ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString(document->url()),
                    (TopDUContext::Features) ( TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate ),
                    BackgroundParser::BestPriority, 0);
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
                 this, [&] (KTextEditor::Document* document) { d->newDocumentStatus(document); });
        connect(d->document.data(), &KTextEditor::Document::textChanged,
                 this, [&] (KTextEditor::Document* document) { d->textChanged(document); });
        connect(d->document.data(), &KTextEditor::Document::documentUrlChanged,
                 this, [&] (KTextEditor::Document* document) { d->documentUrlChanged(document); });
        connect(d->document.data(), &KTextEditor::Document::documentSavedOrUploaded,
                 this, [&] (KTextEditor::Document* document, bool saveAs) { d->documentSaved(document, saveAs); });

        if (qobject_cast<KTextEditor::MarkInterface*>(d->document)) {
            // can't use new signal/slot syntax here, MarkInterface is not a QObject
            connect(d->document, SIGNAL(marksChanged(KTextEditor::Document*)),
                    this, SLOT(saveSessionConfig()));
        }

        if (auto iface = qobject_cast<KTextEditor::ModificationInterface*>(d->document)) {
            iface->setModifiedOnDiskWarning(true);
            // can't use new signal/slot syntax here, ModificationInterface is not a QObject
            connect(d->document, SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this, SLOT(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        }

        notifyTextDocumentCreated();
    }

    view = d->document->createView(parent);
    view->setStatusBarEnabled(Core::self()->partControllerInternal()->showTextEditorStatusBar());

    if (view) {
        connect(view, &KTextEditor::View::contextMenuAboutToShow, this, [&] (KTextEditor::View* v, QMenu* menu) { d->populateContextMenu(v, menu); });

        //in KDE >= 4.4 we can use KXMLGuiClient::replaceXMLFile to provide
        //katepart with our own restructured UI configuration
        const QString uiFile = QCoreApplication::applicationName() + "/katepartui.rc";
        QStringList katePartUIs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, uiFile);
        if (!katePartUIs.isEmpty()) {
            const QString katePartUI = katePartUIs.last();
            const QString katePartLocalUI = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+ '/' + uiFile;
            if (!QFile::exists(katePartLocalUI)) {
                // prevent warning:
                // kdevelop/kdeui (kdelibs): No such XML file ".../.kde/share/apps/kdevelop/katepartui.rc"
                QFile::copy(katePartUI, katePartLocalUI);
            }
            view->replaceXMLFile(katePartUI, katePartLocalUI);
        }
    }

    if (KTextEditor::CodeCompletionInterface* cc = dynamic_cast<KTextEditor::CodeCompletionInterface*>(view))
        cc->setAutomaticInvocationEnabled(core()->languageController()->completionSettings()->automaticCompletionEnabled());

    if (KTextEditor::ConfigInterface *config = qobject_cast<KTextEditor::ConfigInterface*>(view)) {
        config->setConfigValue("allow-mark-menu", false);
        config->setConfigValue("default-mark-type", KTextEditor::MarkInterface::BreakpointActive);
    }

    return view;
}

KParts::Part *TextDocument::partForView(QWidget *view) const
{
    if (d->document && d->document->views().contains((KTextEditor::View*)view))
        return d->document;
    return 0;
}



// KDevelop::IDocument implementation

void TextDocument::reload()
{
    if (!d->document)
        return;

    KTextEditor::ModificationInterface* modif=0;
    if(d->state==Dirty) {
        modif = qobject_cast<KTextEditor::ModificationInterface*>(d->document);
        modif->setModifiedOnDiskWarning(false);
    }
    d->document->documentReload();
    if(modif)
        modif->setModifiedOnDiskWarning(true);
}

bool TextDocument::save(DocumentSaveMode mode)
{
    if (!d->document)
        return true;

    if (mode & Discard)
        return true;

    switch (d->state)
    {
        case IDocument::Clean:
            return true;

        case IDocument::Modified: break;
            if (!(mode & Silent))
            {
                int code = KMessageBox::warningYesNoCancel(
                    Core::self()->uiController()->activeMainWindow(),
                    i18n("The document \"%1\" has unsaved changes. Would you like to save them?", d->document->url().toLocalFile()),
                    i18nc("@title:window", "Close Document"));
                if (code != KMessageBox::Yes)
                    return false;
            }
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
    return d->state;
}

KTextEditor::Cursor KDevelop::TextDocument::cursorPosition() const
{
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
    if (!d->document) {
        return KTextEditor::Range::invalid();
    }

    KTextEditor::View *view = activeTextView();

    if (view && view->selection()) {
        return view->selectionRange();
    }

    return PartDocument::textSelection();
}

QString TextDocument::textLine() const
{
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
        while( startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' || linestr[startPos] == '~' ) )
        {
            --startPos;
        }

        while( endPos < linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' || linestr[endPos] == '~' ) )
        {
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
    if (!range.isValid() || !d->document)
        return;

    KTextEditor::View *view = activeTextView();

    if (view) {
        selectAndReveal(view, range);
    }
}

bool TextDocument::close(DocumentSaveMode mode)
{
    if (!PartDocument::close(mode))
        return false;

    if ( d->document ) {
        d->saveSessionConfig();
        delete d->document; //We have to delete the document right now, to prevent random crashes in the event handler
    }

    return true;
}

Sublime::View* TextDocument::newView(Sublime::Document* doc)
{
    Q_UNUSED(doc);

    emit viewNumberChanged(this);
    return new TextView(this);
}

}

KDevelop::TextView::TextView(TextDocument * doc)
    : View(doc, View::TakeOwnership), d(new TextViewPrivate(this))
{
}

KDevelop::TextView::~TextView()
{
    delete d;
}

QWidget * KDevelop::TextView::createWidget(QWidget * parent)
{
    auto textDocument = qobject_cast<TextDocument*>(document());
    Q_ASSERT(textDocument);
    QWidget* widget = textDocument->createViewWidget(parent);
    d->view = qobject_cast<KTextEditor::View*>(widget);
    Q_ASSERT(d->view);
    if (d->view) {
        connect(d->view.data(), &KTextEditor::View::cursorPositionChanged,
                this, [&] { d->sendStatusChanged(); });
    }
    return widget;
}

QString KDevelop::TextView::viewState() const
{
    if (d->view) {
        if (d->view->selection()) {
            KTextEditor::Range selection = d->view->selectionRange();
            return QStringLiteral("Selection=%1,%2,%3,%4").arg(selection.start().line())
                                                   .arg(selection.start().column())
                                                   .arg(selection.end().line())
                                                   .arg(selection.end().column());
        } else {
            KTextEditor::Cursor cursor = d->view->cursorPosition();
            return QStringLiteral("Cursor=%1,%2").arg(cursor.line()).arg(cursor.column());
        }
    }
    else {
        qCDebug(SHELL) << "TextView's internal KTE view disappeared!";
        return QString();
    }
}

void KDevelop::TextView::setInitialRange(const KTextEditor::Range& range)
{
    if (d->view) {
        selectAndReveal(d->view, range);
    } else {
        d->initialRange = range;
    }
}

KTextEditor::Range KDevelop::TextView::initialRange() const
{
    return d->initialRange;
}

void KDevelop::TextView::setState(const QString & state)
{
    static QRegExp reCursor("Cursor=([\\d]+),([\\d]+)");
    static QRegExp reSelection("Selection=([\\d]+),([\\d]+),([\\d]+),([\\d]+)");
    if (reCursor.exactMatch(state)) {
        setInitialRange(KTextEditor::Range(KTextEditor::Cursor(reCursor.cap(1).toInt(), reCursor.cap(2).toInt()), 0));
    } else if (reSelection.exactMatch(state)) {
        KTextEditor::Range range(reSelection.cap(1).toInt(), reSelection.cap(2).toInt(), reSelection.cap(3).toInt(), reSelection.cap(4).toInt());
        setInitialRange(range);
    }
}

QString KDevelop::TextDocument::documentType() const
{
    return "Text";
}

QIcon KDevelop::TextDocument::defaultIcon() const
{
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
    return d->view;
}

QString KDevelop::TextView::viewStatus() const
{
    const KTextEditor::Cursor pos = d->view ? d->view->cursorPosition() : KTextEditor::Cursor::invalid();
    return i18n(" Line: %1 Col: %2 ", pos.line() + 1, pos.column() + 1);
}

void KDevelop::TextViewPrivate::sendStatusChanged()
{
    emit q->statusChanged(q);
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

#include "moc_textdocument.cpp"
