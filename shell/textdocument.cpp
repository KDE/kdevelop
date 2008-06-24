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

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kxmlguifactory.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/codecompletioninterface.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include "core.h"
#include "mainwindow.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "documentcontroller.h"

namespace KDevelop {

struct TextDocumentPrivate {
    TextDocumentPrivate(TextDocument *textDocument)
        :m_textDocument(textDocument)
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
        Context* c = new EditorContext( v );
        QList<ContextMenuExtension> extensions = Core::self()->pluginController()->queryPluginsForContextMenuExtensions( c );
        menu->addSeparator();
        foreach( const ContextMenuExtension ext, extensions )
        {
            foreach( QAction* act, ext.actions( ContextMenuExtension::FileGroup ) )
            {
                menu->addAction( act );
            }
            menu->addSeparator();
            foreach( QAction* act, ext.actions( ContextMenuExtension::EditGroup ) )
            {
                menu->addAction( act );
            }
        }

        QList<QAction*> vcsActions;
        QList<QAction*> extActions;
        QList<QAction*> refactorActions;
        foreach( const ContextMenuExtension ext, extensions )
        {

            foreach( QAction* act, ext.actions( ContextMenuExtension::VcsGroup ) )
            {
                vcsActions << act;
            }

            foreach( QAction* act, ext.actions( ContextMenuExtension::ExtensionGroup ) )
            {
                extActions << act;
            }

            foreach( QAction* act, ext.actions( ContextMenuExtension::RefactorGroup ) )
            {
                refactorActions << act;
            }

        }


        QMenu* refactormenu = menu;
        if( refactorActions.count() > 1 )
        {
            refactormenu = menu->addMenu( "Refactor");
        }
        foreach( QAction* act, refactorActions )
        {
            refactormenu->addAction( act );
        }
        menu->addSeparator();

        QMenu* vcsmenu = menu;
        if( vcsActions.count() > 1 )
        {
            vcsmenu = menu->addMenu( "Version Control");
        }
        foreach( QAction* act, vcsActions )
        {
            vcsmenu->addAction( act );
        }

        menu->addSeparator();
        foreach( QAction* act, extActions )
        {
            menu->addAction( act );
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


private:
    TextDocument *m_textDocument;
};

class TextViewPrivate
{
public:
    TextViewPrivate()
    {
        m_view = 0;
    }
    QPointer<KTextEditor::View> m_view;
    QString m_status;
};

TextDocument::TextDocument(const KUrl &url, ICore* core)
    :PartDocument(url, core), d(new TextDocumentPrivate(this))
{
    if (url.url().endsWith("kdevtmp"))
        setObjectName(i18n("Untitled"));
}

TextDocument::~TextDocument()
{
    delete d;
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
        d->document = Core::self()->partManagerInternal()->createTextPart(url(),
            Core::self()->documentController()->encoding(), !url().isEmpty());
        /* It appears, that by default a part will be deleted the the
           first view containing it is deleted.  Since we do want
           to have several views, disable that behaviour.  */
        d->document->setAutoDeletePart(false);

        Core::self()->partManager()->addPart(d->document, false);

        connect(d->document, SIGNAL(modifiedChanged(KTextEditor::Document*)),
                 this, SLOT(newDocumentStatus(KTextEditor::Document*)));
        connect(d->document, SIGNAL(textChanged(KTextEditor::Document*)),
                 this, SLOT(textChanged(KTextEditor::Document*)));

        KTextEditor::ModificationInterface *iface = qobject_cast<KTextEditor::ModificationInterface*>(d->document);
        if (iface)
        {
            iface->setModifiedOnDiskWarning(true);
            connect(d->document, SIGNAL(modifiedOnDisk(KTextEditor::Document*, bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this, SLOT(modifiedOnDisk(KTextEditor::Document*, bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        }
    }

    view = d->document->createView(parent);

    if (view) {
        view->setContextMenu( view->defaultContextMenu() );
        connect(view, SIGNAL(contextMenuAboutToShow(KTextEditor::View*,QMenu*)), this, SLOT(populateContextMenu(KTextEditor::View*,QMenu*)));
    }

    if (KTextEditor::CodeCompletionInterface* cc = dynamic_cast<KTextEditor::CodeCompletionInterface*>(view)) {
        KConfigGroup group(KGlobal::config(), "Code Completion");
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
    if (cursor.line() < 0)
        return ;

    KTextEditor::View *view = d->document->activeView();

    KTextEditor::Cursor c = cursor;
    if (c.column() == 1)
        c.setColumn(0);

    if (view)
        view->setCursorPosition(c);
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
    d->m_view = static_cast<KTextEditor::View*>(
        static_cast<TextDocument*>(document())->createViewWidget(parent));

    connect(d->m_view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)), this, SLOT(viewStatusChanged(KTextEditor::View*, const KTextEditor::Cursor&)));
    viewStatusChanged(d->m_view, d->m_view->cursorPosition());

    return d->m_view;
}

QString KDevelop::TextView::viewState() const
{
    if( !d->m_view.isNull() )
    {
        KTextEditor::Cursor cursor = d->m_view->cursorPosition();
        return QString("Cursor=%1,%2").arg(cursor.line()).arg(cursor.column());
    }else
    {
        kDebug() << "TextView's internal KTE view disappeared!";
        return QString();
    }
}

void KDevelop::TextView::setState(const QString & state)
{
    static QRegExp re("Cursor=([\\d]+),([\\d]+)");
    if (d->m_view && re.exactMatch(state))
        d->m_view->setCursorPosition(KTextEditor::Cursor(re.cap(1).toInt(), re.cap(2).toInt()));
}

QString KDevelop::TextDocument::documentType() const
{
    return "Text";
}

KTextEditor::View *KDevelop::TextView::textView() const
{
    return d->m_view;
}

QString KDevelop::TextView::viewStatus() const
{
    return d->m_status;
}

void KDevelop::TextView::viewStatusChanged(KTextEditor::View*, const KTextEditor::Cursor& newPosition)
{
    d->m_status = i18n(" Line: %1 Col: %2 ", KGlobal::locale()->formatNumber(newPosition.line() + 1, 0), KGlobal::locale()->formatNumber(newPosition.column() + 1, 0));
    emit statusChanged(this);
}

#include "textdocument.moc"
