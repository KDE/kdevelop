/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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

#include <QPointer>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>


#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <kstatusbar.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/codecompletioninterface.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include <language/editor/editorintegrator.h>
#include <language/interfaces/editorcontext.h>

#include "core.h"
#include "mainwindow.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "documentcontroller.h"

namespace KDevelop {

struct TextDocumentPrivate {
    TextDocumentPrivate(TextDocument *textDocument)
        : m_textDocument(textDocument)
        , m_loaded(false)
    {
        document = 0;
        state = IDocument::Clean;
    }
    QPointer<KTextEditor::Document> document;
    IDocument::DocumentState state;


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
        menu->clear();
        v->defaultContextMenu(menu);

        Context* c = new EditorContext( v, v->cursorPosition() );
        QList<ContextMenuExtension> extensions = Core::self()->pluginController()->queryPluginsForContextMenuExtensions( c );
        menu->addSeparator();

        ContextMenuExtension::populateMenu(menu, extensions);
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

        setStatus(document, dirty);
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
        EditorIntegrator::addDocument( m_textDocument->textDocument() );
        m_textDocument->notifyLoaded();
    }

private:
    TextDocument *m_textDocument;
    bool m_loaded;
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
        statusBar = 0;
        widgetLayout = 0;
        view = 0;
        statusLabel = 0;
    }
    QWidget* widget;
    QVBoxLayout* widgetLayout;
    QPointer<KTextEditor::View> view;
    KStatusBar *statusBar;
    QLabel* statusLabel;
    QString status;

};

TextDocument::TextDocument(const KUrl &url, ICore* core)
    :PartDocument(url, core), d(new TextDocumentPrivate(this))
{
    if (url.url().endsWith("kdevtmp"))
        setTitle(i18n("Untitled"));
}

TextDocument::~TextDocument()
{
    delete d;
}

bool TextDocument::isTextDocument() const
{
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

        if (!url().isEmpty())
            d->document->openUrl( url() );

        /* It appears, that by default a part will be deleted the the
           first view containing it is deleted.  Since we do want
           to have several views, disable that behaviour.  */
        d->document->setAutoDeletePart(false);

        Core::self()->partController()->addPart(d->document, false);

        connect(d->document, SIGNAL(modifiedChanged(KTextEditor::Document*)),
                 this, SLOT(newDocumentStatus(KTextEditor::Document*)));
        connect(d->document, SIGNAL(textChanged(KTextEditor::Document*)),
                 this, SLOT(textChanged(KTextEditor::Document*)));
        connect(d->document, SIGNAL(documentUrlChanged(KTextEditor::Document*)),
                 this, SLOT(documentUrlChanged(KTextEditor::Document*)));

        KTextEditor::ModificationInterface *iface = qobject_cast<KTextEditor::ModificationInterface*>(d->document);
        if (iface)
        {
            iface->setModifiedOnDiskWarning(true);
            connect(d->document, SIGNAL(modifiedOnDisk(KTextEditor::Document*, bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this, SLOT(modifiedOnDisk(KTextEditor::Document*, bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        }

        notifyTextDocumentCreated();
    }

    view = d->document->createView(parent);

    if (view) {
        view->setContextMenu( view->defaultContextMenu() );
        connect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*,QMenu*)), this, SLOT(populateContextMenu(KTextEditor::View*,QMenu*)));
    }

    if (KTextEditor::CodeCompletionInterface* cc = dynamic_cast<KTextEditor::CodeCompletionInterface*>(view)) {
        KConfigGroup group(KGlobal::config(), "Language Support");
        bool automaticInvocation = group.readEntry( "Automatic Invocation", false );

        cc->setAutomaticInvocationEnabled(automaticInvocation);
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

    if (d->document->isModified())
    {
        if (KMessageBox::warningYesNo(Core::self()->uiController()->activeMainWindow(),
                i18n( "The file \"%1\" is modified "
                      "in memory. Are you sure you "
                      "want to reload it? (Local "
                      "changes will be lost.)", url().toLocalFile() ),
                i18n( "Document is Modified" ) ) == KMessageBox::Yes )
            d->document->setModified(false);
        else
            return ;
    }

    QList<KTextEditor::Cursor> cursors;
    foreach (KTextEditor::View *view, d->document->views())
        cursors << view->cursorPosition();

    d->document->openUrl(url());

    notifyStateChanged();

    int i = 0;
    foreach (KTextEditor::View *view, d->document->views())
        view->setCursorPosition(cursors[i++]);
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
                    i18n("Close Document"));
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
                    i18n("Document Externally Modified"));
                if (code != KMessageBox::Yes)
                    return false;
            }
            break;
    }

    if (d->document->save())
    {
        notifyStateChanged();
        notifySaved();
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
    KTextEditor::View *view = d->document->activeView();

    if (view)
        return view->cursorPosition();

    return KTextEditor::Cursor();
}

void TextDocument::setCursorPosition(const KTextEditor::Cursor &cursor)
{
    if (!cursor.isValid())
        return;

    KTextEditor::View *view = d->document->activeView();

    // Rodda: Cursor must be accurate here, to the definition of accurate for KTextEditor::Cursor.
    // ie, starting from 0,0

    if (view)
        view->setCursorPosition(cursor);
}

KTextEditor::Range TextDocument::textSelection() const
{
    KTextEditor::View *view = d->document->activeView();

    if (view && view->selection()) {
        return view->selectionRange();
    }

    return PartDocument::textSelection();
}

QString TextDocument::textLine() const
{
    KTextEditor::View *view = d->document->activeView();

    if (view) {
        return d->document->line( view->cursorPosition().line() );
    }

    return PartDocument::textLine();
}

QString TextDocument::textWord() const
{
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
    if (!range.isValid())
        return;

    KTextEditor::View *view = d->document->activeView();

    if (view) {
        view->setCursorPosition(range.start());
        view->setSelection( range );
    }
}

bool TextDocument::close(DocumentSaveMode mode)
{
    if (!PartDocument::close(mode))
        return false;

    d->document->deleteLater();

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
    : View(doc), d( new TextViewPrivate )
{
}

KDevelop::TextView::~TextView()
{
    delete d;
}

QWidget * KDevelop::TextView::createWidget(QWidget * parent)
{
    TextDocument* doc = static_cast<TextDocument*>(document());
    KTextEditor::View* view = static_cast<KTextEditor::View*>(doc->createViewWidget(parent));
    if(d->initialRange.isValid()) {
        if(d->initialRange.isEmpty())
            view->setCursorPosition(d->initialRange.start());
        else
            view->setSelection(d->initialRange);
    }
    TextEditorWidget* teWidget = new TextEditorWidget(parent);
    teWidget->setEditorView(view);
    connect(teWidget, SIGNAL(statusChanged()),
            this, SLOT(sendStatusChanged()));
            
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
        KTextEditor::Cursor cursor = d->editor->editorView()->cursorPosition();
        return QString("Cursor=%1,%2").arg(cursor.line()).arg(cursor.column());
    }else
    {
        kDebug() << "TextView's internal KTE view disappeared!";
        return QString();
    }
}

void KDevelop::TextView::setInitialRange(KTextEditor::Range range) {
    d->initialRange = range;
}

void KDevelop::TextView::setState(const QString & state)
{
    static QRegExp re("Cursor=([\\d]+),([\\d]+)");
    if (d->editor && d->editor->editorView() && re.exactMatch(state))
        d->editor->editorView()->setCursorPosition(KTextEditor::Cursor(re.cap(1).toInt(), re.cap(2).toInt()));
}

QString KDevelop::TextDocument::documentType() const
{
    return "Text";
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

KDevelop::TextEditorWidget::TextEditorWidget(QWidget* parent)
: QWidget(parent), KXMLGUIClient(), d(new TextEditorWidgetPrivate)
{
    d->widgetLayout = new QVBoxLayout(this);
    d->widgetLayout->setMargin(0);
    d->widgetLayout->setSpacing(0);

    setLayout(d->widgetLayout);
    
    d->statusBar = new KStatusBar(this);
    d->statusLabel = new QLabel(d->statusBar);
    d->statusBar->addPermanentWidget(d->statusLabel);
    d->widgetLayout->addWidget(d->statusBar);
}

KDevelop::TextEditorWidget::~TextEditorWidget()
{
    delete d;
}

void KDevelop::TextEditorWidget::viewStatusChanged(KTextEditor::View*, const KTextEditor::Cursor& newPosition)
{
    d->status = i18n(" Line: %1 Col: %2 ", KGlobal::locale()->formatNumber(newPosition.line() + 1, 0), KGlobal::locale()->formatNumber(newPosition.column() + 1, 0));
    d->statusLabel->setText(d->status);
    emit statusChanged();
}

void KDevelop::TextEditorWidget::setEditorView(KTextEditor::View* view)
{
    if (d->view)
    {
        disconnect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)));
        removeChildClient(view);
    }

    d->view = view;
    insertChildClient(d->view);
    connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)), 
            this, SLOT(viewStatusChanged(KTextEditor::View*, const KTextEditor::Cursor&)));
    
    viewStatusChanged(view, view->cursorPosition());

    d->widgetLayout->insertWidget(0, d->view);
    setFocusProxy(view);
}

QString KDevelop::TextEditorWidget::status() const
{
    return d->status;
}

KTextEditor::View* KDevelop::TextEditorWidget::editorView() const
{
    return d->view;
}

#include "textdocument.moc"
