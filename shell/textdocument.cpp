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

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kxmlguifactory.h>
#include <kdeversion.h>

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

#include <project/projectutils.h>

#include "core.h"
#include "mainwindow.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "documentcontroller.h"

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
            KUrl url = v->document()->url();
            IProject* project = Core::self()->projectController()->findProjectForUrl( url );
            if(project)
            {
                QList< ProjectBaseItem* > items = project->itemsForUrl( url );
                if(!items.isEmpty())
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
        IProject* project = Core::self()->projectController()->findProjectForUrl( document->url() );
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
        QObject::connect(req, SIGNAL(finished(bool)),
                         m_textDocument, SLOT(repositoryCheckFinished(bool)));
        // Abort the request when the user edits the document
        QObject::connect(m_textDocument->textDocument(), SIGNAL(textChanged(KTextEditor::Document*)),
                         req, SLOT(abort()));
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
                statusIcon = KIcon("edit-delete");
            } else {
                state = IDocument::Modified;
                statusIcon = KIcon("document-save");
            }
        else
            if (dirty) {
                state = IDocument::Dirty;
                statusIcon = KIcon("document-revert");
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
        return KGlobal::config()->group("KatePart Settings");
    }

    inline QString docConfigGroupName() const
    {
        return document->url().pathOrUrl();
    }

    inline KConfigGroup docConfigGroup() const
    {
        return katePartSettingsGroup().group(docConfigGroupName());
    }

    void saveSessionConfig()
    {
        if(!document->url().isValid()) {
            return;
        }
        if (KTextEditor::ParameterizedSessionConfigInterface *sessionConfigIface =
            qobject_cast<KTextEditor::ParameterizedSessionConfigInterface*>(document))
        {
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
            sessionConfigIface->writeParameterizedSessionConfig(group,
                KTextEditor::ParameterizedSessionConfigInterface::SkipUrl);
        }
    }

    void loadSessionConfig()
    {
        if (!document || !katePartSettingsGroup().hasGroup(docConfigGroupName())) {
            return;
        }
        if (KTextEditor::ParameterizedSessionConfigInterface *sessionConfigIface =
            qobject_cast<KTextEditor::ParameterizedSessionConfigInterface*>(document))
        {
            sessionConfigIface->readParameterizedSessionConfig(docConfigGroup(),
                KTextEditor::ParameterizedSessionConfigInterface::SkipUrl);
        }
    }


private:
    TextDocument *m_textDocument;
    bool m_loaded;
    // we want to remove the added stuff when the menu hides
    QMenu* m_addedContextMenu;
};

class TextViewPrivate
{
public:
    TextViewPrivate() : editor(0) {}
    TextEditorWidget* editor;
    KTextEditor::Range initialRange;
};

class TextEditorWidgetPrivate
{
public:
    TextEditorWidgetPrivate()
    {
        widget = 0;
        widgetLayout = 0;
        view = 0;
        statusLabel = 0;
    }

    void viewEditModeChanged(KTextEditor::View* view, KTextEditor::View::EditMode mode)
    {
#ifdef KTEXTEDITOR_HAS_VIMODE
        if ( ! statusLabel ) {
            statusLabel = new QLabel();
            view->layout()->addWidget(statusLabel);
        }
        if ( mode == KTextEditor::View::EditViMode ) {
            statusLabel->setText(view->viewMode() + "    <i>" + view->document()->url().fileName() + "</i>");
            statusLabel->setHidden(false);
        }
        else {
            statusLabel->setHidden(true);
        }
#else
        Q_UNUSED(view);
        Q_UNUSED(mode);
#endif
    }
    QWidget* widget;
    QVBoxLayout* widgetLayout;
    QPointer<KTextEditor::View> view;
    QLabel* statusLabel;
    QString status;
    const TextView* textView;

};

TextDocument::TextDocument(const KUrl &url, ICore* core, const QString& encoding)
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
        connect(d->document, SIGNAL(textChanged(KTextEditor::Document*)), this, SLOT(slotDocumentLoaded()));
        // Also connect to the completed signal, sometimes the first text changed signal is missed because the part loads too quickly (? TODO - confirm this is necessary)
        connect(d->document, SIGNAL(completed()), this, SLOT(slotDocumentLoaded()));

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

        connect(d->document, SIGNAL(modifiedChanged(KTextEditor::Document*)),
                 this, SLOT(newDocumentStatus(KTextEditor::Document*)));
        connect(d->document, SIGNAL(textChanged(KTextEditor::Document*)),
                 this, SLOT(textChanged(KTextEditor::Document*)));
        connect(d->document, SIGNAL(documentUrlChanged(KTextEditor::Document*)),
                 this, SLOT(documentUrlChanged(KTextEditor::Document*)));
        connect(d->document, SIGNAL(documentSavedOrUploaded(KTextEditor::Document*,bool)),
                 this, SLOT(documentSaved(KTextEditor::Document*,bool)));
        connect(d->document, SIGNAL(marksChanged(KTextEditor::Document*)),
                 this, SLOT(saveSessionConfig()));

        KTextEditor::ModificationInterface *iface = qobject_cast<KTextEditor::ModificationInterface*>(d->document);
        if (iface)
        {
            iface->setModifiedOnDiskWarning(true);
            connect(d->document, SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this, SLOT(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        }

        notifyTextDocumentCreated();
    }

    view = d->document->createView(parent);

    if (view) {
        connect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*,QMenu*)), this, SLOT(populateContextMenu(KTextEditor::View*,QMenu*)));

        //in KDE >= 4.4 we can use KXMLGuiClient::replaceXMLFile to provide
        //katepart with our own restructured UI configuration
        const QString uiFile = KGlobal::mainComponent().componentName() + "/katepartui.rc";
        QStringList katePartUIs = KGlobal::mainComponent().dirs()->findAllResources("data", uiFile);
        if (!katePartUIs.isEmpty()) {
            const QString katePartUI = katePartUIs.last();
            const QString katePartLocalUI = KStandardDirs::locateLocal("data", uiFile);
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

    KUrl urlBeforeSave = d->document->url();
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

    KTextEditor::View *view = d->document->activeView();

    if (view)
        return view->cursorPosition();

    return KTextEditor::Cursor::invalid();
}

void TextDocument::setCursorPosition(const KTextEditor::Cursor &cursor)
{
    if (!cursor.isValid() || !d->document)
        return;

    KTextEditor::View *view = d->document->activeView();

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

    KTextEditor::View *view = d->document->activeView();

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

    KTextEditor::View *view = d->document->activeView();

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

    KTextEditor::View *view = d->document->activeView();

    if (view) {
        KTextEditor::Cursor start = view->cursorPosition();
        kDebug() << "got start position from view:" << start.line() << start.column();
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
            kDebug() << "found word" << startPos << endPos << linestr.mid( startPos+1, endPos - startPos - 1 );
            return linestr.mid( startPos + 1, endPos - startPos - 1 );
        }
    }

    return PartDocument::textWord();
}

void TextDocument::setTextSelection(const KTextEditor::Range &range)
{
    if (!range.isValid() || !d->document)
        return;

    KTextEditor::View *view = d->document->activeView();

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
    : View(doc, View::TakeOwnership), d( new TextViewPrivate )
{
}

KDevelop::TextView::~TextView()
{
    delete d->editor; //We have to delete the view right now, to prevent random crashes in the event handler
    d->editor = 0;
    delete d;
}

QWidget * KDevelop::TextView::createWidget(QWidget * parent)
{
    TextEditorWidget* teWidget = new TextEditorWidget(this, parent);
    connect(teWidget, SIGNAL(statusChanged()), this, SLOT(sendStatusChanged()));

    d->editor = teWidget;
    connect(d->editor, SIGNAL(destroyed(QObject*)), this, SLOT(editorDestroyed(QObject*)));
    
    return teWidget;
}

void KDevelop::TextView::editorDestroyed(QObject* obj) {
    if(obj == d->editor)
        d->editor = 0;
}

QString KDevelop::TextView::viewState() const
{
    if( d->editor && d->editor->editorView() )
    {
        if (d->editor->editorView()->selection()) {
            KTextEditor::Range selection = d->editor->editorView()->selectionRange();
            return QString("Selection=%1,%2,%3,%4").arg(selection.start().line())
                                                   .arg(selection.start().column())
                                                   .arg(selection.end().line())
                                                   .arg(selection.end().column());
        } else {
            KTextEditor::Cursor cursor = d->editor->editorView()->cursorPosition();
            return QString("Cursor=%1,%2").arg(cursor.line()).arg(cursor.column());
        }
    }
    else
    {
        kDebug() << "TextView's internal KTE view disappeared!";
        return QString();
    }
}

void KDevelop::TextView::setInitialRange(const KTextEditor::Range& range)
{
    if(d->editor && d->editor->isInitialized()) {
        selectAndReveal(d->editor->editorView(), range);
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
        KMimeType::Ptr mime = KMimeType::mimeType(d->document->mimeType());
        KIcon icon(mime->iconName());
        if (!icon.isNull()) {
            return icon;
        }
    }
    return PartDocument::defaultIcon();
}

KTextEditor::View *KDevelop::TextView::textView() const
{  
    if (d->editor)
        return d->editor->editorView();

    return 0;
}

QString KDevelop::TextView::viewStatus() const
{
    if (d->editor)
        return d->editor->status();

    return QString();
}

void KDevelop::TextView::sendStatusChanged()
{
    emit statusChanged(this);
}

KDevelop::TextEditorWidget::TextEditorWidget(const TextView* view, QWidget* parent)
: QWidget(parent), KXMLGUIClient(), d(new TextEditorWidgetPrivate)
{
    d->widgetLayout = new QVBoxLayout(this);
    d->widgetLayout->setMargin(0);
    d->widgetLayout->setSpacing(0);
    d->textView = view;
    d->view = 0;

    setLayout(d->widgetLayout);
    initialize();
}

KDevelop::TextEditorWidget::~TextEditorWidget()
{
    delete d;
}

void KDevelop::TextEditorWidget::initialize()
{
    if(d->view)
        return;
    TextDocument* doc = static_cast<TextDocument*>(d->textView->document());
    KTextEditor::View* view = qobject_cast<KTextEditor::View*>(doc->createViewWidget(this));
    KTextEditor::Range ir = d->textView->initialRange();
    selectAndReveal(view, ir);
    setEditorView(view);
}

void KDevelop::TextEditorWidget::viewStatusChanged(KTextEditor::View* view, const KTextEditor::Cursor& )
{
    // This fetches the virtual cursor, which expands tab characters properly, i.e. instead of col == 1
    // you'll get col == 9 with this when a tab is at the start of the line.
    KTextEditor::Cursor pos = view->cursorPositionVirtual();
    d->status = i18n(" Line: %1 Col: %2 ", KGlobal::locale()->formatNumber(pos.line() + 1, 0), KGlobal::locale()->formatNumber(pos.column() + 1, 0));
    emit statusChanged();
}

void KDevelop::TextEditorWidget::setEditorView(KTextEditor::View* view)
{
    if (d->view)
    {
        disconnect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
        removeChildClient(view);
    }

    d->view = view;
    insertChildClient(d->view);
    connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), 
            this, SLOT(viewStatusChanged(KTextEditor::View*,KTextEditor::Cursor)));
    
    viewStatusChanged(view, view->cursorPosition());

    connect(view, SIGNAL(viewEditModeChanged(KTextEditor::View*,KTextEditor::View::EditMode)),
            this, SLOT(viewEditModeChanged(KTextEditor::View*,KTextEditor::View::EditMode)));

    d->widgetLayout->insertWidget(0, d->view);
    setFocusProxy(view);
}

QString KDevelop::TextEditorWidget::status() const
{
    return d->status;
}

KTextEditor::View* KDevelop::TextEditorWidget::editorView()
{
    if(!d->view)
        initialize();
    return d->view;
}

bool KDevelop::TextEditorWidget::isInitialized() const
{
    return d->view!=0;
}

void KDevelop::TextEditorWidget::showEvent(QShowEvent* event)
{
    if(!d->view)
        initialize();
    QWidget::showEvent(event);
}

#include "textdocument.moc"
