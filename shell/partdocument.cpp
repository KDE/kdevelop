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
#include "partdocument.h"

#include <QMimeDatabase>

#include <KMessageBox>
#include <KLocalizedString>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/mainwindow.h>

#include "core.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "documentcontroller.h"

namespace KDevelop {

class PartDocumentPrivate {
public:

    QMap<QWidget*, KParts::Part*> partForView;
    QString preferredPart;
};

PartDocument::PartDocument(const KUrl& url, KDevelop::ICore* core, const QString& preferredPart)
    : Sublime::UrlDocument(core->uiController()->controller(), url), KDevelop::IDocument(core), d(new PartDocumentPrivate)
{
    d->preferredPart = preferredPart;
}

PartDocument::~PartDocument()
{
    delete d;
}

QWidget *PartDocument::createViewWidget(QWidget* /*parent*/)
{
    KParts::Part *part = Core::self()->partControllerInternal()->createPart(url(), d->preferredPart);
    if( part )
    {
        Core::self()->partController()->addPart(part);
        QWidget *w = part->widget();
        d->partForView[w] = part;
        return w;
    }
    return 0;
}

KParts::Part *PartDocument::partForView(QWidget *view) const
{
    return d->partForView[view];
}



//KDevelop::IDocument implementation


QMimeType PartDocument::mimeType() const
{
    return QMimeDatabase().mimeTypeForUrl(url());
}

KTextEditor::Document *PartDocument::textDocument() const
{
    return 0;
}

bool PartDocument::isActive() const
{
    return Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView()->document() == this;
}

bool PartDocument::save(DocumentSaveMode /*mode*/)
{
    //part document is read-only so do nothing here
    return true;
}

bool PartDocument::askForCloseFeedback()
{
    if (state() == IDocument::Modified) {
        int code = KMessageBox::warningYesNoCancel(
            Core::self()->uiController()->activeMainWindow(),
            i18n("The document \"%1\" has unsaved changes. Would you like to save them?", url().toLocalFile()),
            i18n("Close Document"));

        if (code == KMessageBox::Yes) {
            if (!save(Default))
                return false;

        } else if (code == KMessageBox::Cancel) {
            return false;
        }

    /// @todo Is this behavior right?
    } else if (state() == IDocument::DirtyAndModified) {
        int code = KMessageBox::warningYesNoCancel(
            Core::self()->uiController()->activeMainWindow(),
            i18n("The document \"%1\" has unsaved changes and was modified by an external process.\n"
                 "Do you want to override the external changes?", url().toLocalFile()),
            i18n("Close Document"));

        if (code == KMessageBox::Yes) {
            if (!save(Default))
                return false;

        } else if (code == KMessageBox::Cancel) {
            return false;
        }

    }
    return true;
}

bool PartDocument::close(DocumentSaveMode mode)
{
    if (!(mode & Discard)) {
        if (mode & Silent) {
            if (!save(mode))
                return false;
        } else {
            if( !askForCloseFeedback() )
                return false;
        }
    }

    //close all views and then delete ourself
    closeViews();

    foreach (KParts::Part* part, d->partForView)
        part->deleteLater();

    // The document will be deleted automatically if there are no views left

    return true;
}

bool PartDocument::closeDocument(bool silent) {
    return close(silent ? Silent : Default);
}

void PartDocument::reload()
{
    //part document is read-only so do nothing here
}

IDocument::DocumentState PartDocument::state() const
{
    return Clean;
}

void PartDocument::activate(Sublime::View *activeView, KParts::MainWindow *mainWindow)
{
    Q_UNUSED(mainWindow);
    KParts::Part *part = partForView(activeView->widget());
    if (Core::self()->partController()->activePart() != part)
        Core::self()->partController()->setActivePart(part);
    notifyActivated();
}

KTextEditor::Cursor KDevelop::PartDocument::cursorPosition() const
{
    return KTextEditor::Cursor::invalid();
}

void PartDocument::setCursorPosition(const KTextEditor::Cursor &cursor)
{
    //do nothing here
    Q_UNUSED(cursor);
}

void PartDocument::setTextSelection(const KTextEditor::Range &range)
{
    Q_UNUSED(range);
}

KUrl PartDocument::url() const
{
    return Sublime::UrlDocument::url();
}

void PartDocument::setUrl(const KUrl& newUrl)
{
    Sublime::UrlDocument::setUrl(newUrl);
    if(!prettyName().isEmpty())
        setTitle(prettyName());
    notifyUrlChanged();
}

void PartDocument::setPrettyName(QString name)
{
    KDevelop::IDocument::setPrettyName(name);
    // Re-set the url, to trigger the whole chain
    if(!name.isEmpty())
        setTitle(name);
    else
        setTitle(url().fileName());
}

QMap<QWidget*, KParts::Part*> PartDocument::partForView() const
{
    return d->partForView;
}

void PartDocument::addPartForView(QWidget* w, KParts::Part* p)
{
    d->partForView[w]=p;
}

}

