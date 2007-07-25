/***************************************************************************
 *   Copyright 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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

#include <klocale.h>
#include <kmessagebox.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>

#include <sublime/mainwindow.h>

#include "core.h"
#include "uicontroller.h"
#include "partcontroller.h"
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
        if (document->isModified())
            state = IDocument::Clean;
        else
            state = IDocument::Dirty;
        m_textDocument->notifyStateChanged();
    }

    void textChanged(KTextEditor::Document *document)
    {
        m_textDocument->notifyContentChanged();
    }
    
    void modifiedOnDisk(KTextEditor::Document *document, bool /*isModified*/,
        KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
    {
        switch (reason)
        {
            case KTextEditor::ModificationInterface::OnDiskUnmodified:
                if (!document->isModified())
                    state = IDocument::Clean;
                else
                    state = IDocument::Dirty;
                break;
            case KTextEditor::ModificationInterface::OnDiskModified:
            case KTextEditor::ModificationInterface::OnDiskCreated:
            case KTextEditor::ModificationInterface::OnDiskDeleted:
                state = IDocument::DirtyAndModified;
        }
        m_textDocument->notifyStateChanged();
    }

private:
    TextDocument *m_textDocument;
};


TextDocument::TextDocument(const KUrl &url, ICore* core)
    :PartDocument(url, core), d(new TextDocumentPrivate(this))
{
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
    if (!d->document)
    {
        d->document = Core::self()->partManagerInternal()->createTextPart(url(),
            Core::self()->documentController()->encoding(), !url().isEmpty());
        Core::self()->partManager()->addPart(d->document);

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

        return d->document->widget();
    }
    return d->document->createView(parent);
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

    switch (d->state)
    {
        case IDocument::Clean: return true;
        case IDocument::Modified: break;
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
    return Clean;
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

}

#include "textdocument.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
