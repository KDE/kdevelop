/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "partdocument.h"

#include <QMimeDatabase>

#include <KMessageBox>
#include <KMessageBox_KDevCompat>
#include <KLocalizedString>
#include <KTextEditor/Cursor>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/mainwindow.h>

#include "core.h"
#include "uicontroller.h"
#include "partcontroller.h"

namespace KDevelop {

class PartDocumentPrivate {
public:
    explicit PartDocumentPrivate(const QString& preferredPart)
        : preferredPart(preferredPart)
    {}

    QMap<QWidget*, KParts::Part*> partForView;
    const QString preferredPart;
};

PartDocument::PartDocument(const QUrl& url, KDevelop::ICore* core, const QString& preferredPart)
    : Sublime::UrlDocument(core->uiController()->controller(), url)
    , KDevelop::IDocument(core)
    , d_ptr(new PartDocumentPrivate(preferredPart))
{
}

PartDocument::~PartDocument() = default;

QWidget *PartDocument::createViewWidget(QWidget* /*parent*/)
{
    Q_D(PartDocument);

    KParts::Part *part = Core::self()->partControllerInternal()->createPart(url(), d->preferredPart);
    if( part )
    {
        Core::self()->partController()->addPart(part);
        QWidget *w = part->widget();
        d->partForView[w] = part;
        return w;
    }
    return nullptr;
}

KParts::Part *PartDocument::partForView(QWidget *view) const
{
    Q_D(const PartDocument);

    return d->partForView[view];
}



//KDevelop::IDocument implementation


QMimeType PartDocument::mimeType() const
{
    return QMimeDatabase().mimeTypeForUrl(url());
}

QIcon PartDocument::icon() const
{
    return Sublime::UrlDocument::icon();
}

KTextEditor::Document *PartDocument::textDocument() const
{
    return nullptr;
}

bool PartDocument::isActive() const
{
    const auto activeView = Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView();
    if (!activeView) {
        return false;
    }

    return activeView->document() == this;
}

bool PartDocument::save(DocumentSaveMode /*mode*/)
{
    //part document is read-only so do nothing here
    return true;
}

bool PartDocument::askForCloseFeedback()
{
    int code = -1;
    if (state() == IDocument::Modified) {
        code = KMessageBox::warningTwoActionsCancel(
            Core::self()->uiController()->activeMainWindow(),
            i18n("The document \"%1\" has unsaved changes. Would you like to save them?", url().toLocalFile()),
            i18nc("@title:window", "Close Document"), KStandardGuiItem::save(), KStandardGuiItem::discard());

    /// @todo Is this behavior right?
    } else if (state() == IDocument::DirtyAndModified) {
        code = KMessageBox::warningTwoActionsCancel(
            Core::self()->uiController()->activeMainWindow(),
            i18n("The document \"%1\" has unsaved changes and was modified by an external process.\n"
                 "Do you want to overwrite the external changes?",
                 url().toLocalFile()),
            i18nc("@title:window", "Close Document"),
            KGuiItem(i18nc("@action:button", "Overwrite External Changes"), QStringLiteral("document-save")),
            KStandardGuiItem::discard());
    }

    if (code >= 0) {
        if (code == KMessageBox::PrimaryAction) {
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
    Q_D(PartDocument);

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

    for (KParts::Part* part : std::as_const(d->partForView)) {
        part->deleteLater();
    }

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

QUrl PartDocument::url() const
{
    return Sublime::UrlDocument::url();
}

void PartDocument::setUrl(const QUrl& newUrl)
{
    const auto previousUrl = Sublime::UrlDocument::url();
    Sublime::UrlDocument::setUrl(newUrl);
    if(!prettyName().isEmpty())
        setTitle(prettyName());
    notifyUrlChanged(previousUrl);
}

void PartDocument::setPrettyName(const QString& name)
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
    Q_D(const PartDocument);

    return d->partForView;
}

void PartDocument::addPartForView(QWidget* w, KParts::Part* p)
{
    Q_D(PartDocument);

    d->partForView[w]=p;
}

}

#include "moc_partdocument.cpp"
